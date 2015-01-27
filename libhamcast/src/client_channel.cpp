/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/

#include <map>
#include <set>
#include <list>
#include <memory>
#include <algorithm>

#include <boost/thread.hpp>

#include "hamcast/config.hpp"
#include "hamcast/membership_event.hpp"
#include "hamcast/util/deserializer.hpp"
#include "hamcast/async_multicast_socket.hpp"

#include "hamcast/util/sink.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/util/future.hpp"
#include "hamcast/util/socket_io.hpp"
#include "hamcast/util/read_buffer.hpp"
#include "hamcast/util/id_generator.hpp"
#include "hamcast/util/buffered_sink.hpp"
#include "hamcast/util/serialization.hpp"
#include "hamcast/util/buffered_source.hpp"

#include "hamcast/ipc/client_channel.hpp"
#include "hamcast/ipc/fake_client_channel.hpp"
#include "hamcast/ipc/middleware_configuration.hpp"

#ifdef HC_USE_UNIX_SOCKETS
#include <cstdio>
#include <cstdlib>
#include <sys/un.h>
#include <sys/socket.h>
#endif

#include <iostream>
using std::cerr;
using std::endl;

namespace {

using namespace hamcast;
using namespace hamcast::ipc;
using namespace hamcast::detail;

boost::mutex m_instance_mtx;
client_channel::ptr m_instance;

std::pair<util::source::ptr, util::sink::ptr> new_client_socket_io()
{
    HC_LOG_TRACE("");
#   ifndef HC_USE_UNIX_SOCKETS
    middleware_configuration mc;
    mc.read();
    util::socket_io::ptr sio = util::socket_io::create("127.0.0.1", mc.port());
#   else
    int fd;
    sockaddr_un addr;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        throw connection_to_middleware_failed(socket_creation_failed);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    if (connect(fd, (sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("connect error");
        throw connection_to_middleware_failed(no_running_middleware_found);
    }
    HC_LOG_DEBUG("fd = " << fd);
    util::socket_io::ptr sio = util::socket_io::create(fd);
#   endif
    HC_LOG_DEBUG(   "sio->read_handle() = " << sio->read_handle()
                 << ", sio->write_handle() = " << sio->write_handle());
    std::pair<util::source::ptr, util::sink::ptr> result;
    result.first.reset(sio.get());
    result.second.reset(new util::buffered_sink<>(sio.get()));
    // verify this client; lifetime scope of s
    {
        util::serializer s(result.second);
        s << magic_number
          << major_version
          << minor_version;
    }
    // accepted?
    {
        boost::uint8_t answer;
        util::deserializer d(result.first);
        d >> answer;
        if (answer != 1)
        {
            throw connection_to_middleware_failed(incompatible_middleware_found);
        }
    }
    return result;
}

struct omp_ptr_less
{
    inline bool operator()(const omp_ptr& lhs, const omp_ptr& rhs)
    {
        // nullptr < {everything}
        return (lhs && rhs) ? lhs->seq_nr < rhs->seq_nr : rhs != 0;
    }
};

typedef std::map<sequence_number, omp_ptr> omp_ptr_map;

typedef std::map<socket_id, omp_ptr_map> socket_mcast_buf_map;


template<typename T>
struct scope_cleaner { };

template<>
struct scope_cleaner<omp_ptr_map>
{
    omp_ptr_map& mbuf;
    scope_cleaner(omp_ptr_map& buf) : mbuf(buf) { }
    ~scope_cleaner()
    {
        for (omp_ptr_map::iterator i = mbuf.begin(); i != mbuf.end(); ++i)
        {
            delete (i->second);
        }
        mbuf.clear();
    }
};

template<>
struct scope_cleaner<socket_mcast_buf_map>
{
    socket_mcast_buf_map& mb_map;
    scope_cleaner(socket_mcast_buf_map& scoped_map) : mb_map(scoped_map) { }
    ~scope_cleaner()
    {
        for (socket_mcast_buf_map::iterator k = mb_map.begin();
             k != mb_map.end();
             ++k)
        {
            scope_cleaner<omp_ptr_map> tmp(k->second);
        }
        mb_map.clear();
    }
};

// returns: the number of released bytes of all content fields
// with respect to max_buffered_sends (each content_size is counted as
// max(content_size, default_max_buffer_size / max_buffered_sends)
boost::uint32_t drop_smaller_or_equal_seq_nrs(socket_data& ostm,
                                              boost::uint32_t seq_nr)
{
    HC_LOG_TRACE("seq_nr = " << seq_nr);
    omp_ptr_map& mb = ostm.buffer;
    boost::uint32_t result = 0;
    omp_ptr_map::iterator begin = mb.begin();
    // end = first element whose key is > seq_nr
    omp_ptr_map::iterator end = mb.upper_bound(seq_nr);
    HC_LOG_DEBUG("head: " << begin->first << ", upper_bound = " << (end == mb.end() ? 0 : end->first));
    omp_ptr_map tmp;
    long removed_items = 0;
    if (begin != end)
    {
        tmp.insert(begin, end);
        scope_cleaner<omp_ptr_map> cleaner(tmp);
        for (omp_ptr_map::iterator i = begin; i != end; ++i)
        {
//			result += i->second->content_size;
            result += std::max(i->second->content_size, static_cast<boost::uint32_t>(min_buffer_chunk));
            ++removed_items;
        }
        mb.erase(begin, end);
        // cleaner deletes all dropped pointers
    }
    HC_LOG_DEBUG(removed_items << " elements removed");
    begin = mb.begin();
    // end = first element whose key is > seq_nr
    if (begin == mb.end())
    {
        HC_LOG_DEBUG("new head: -empty-");
    }
    else
    {
        HC_LOG_DEBUG("new head: " << begin->first);
    }
    ostm.pending_sends -= removed_items;
    return result;
}

void do_send(util::serializer& s, socket_data& ostm,
             ipc::sequence_number from, ipc::sequence_number to)
{
    HC_LOG_TRACE("from = " << from << ", to = " << to);
    long sent = 0;
    omp_ptr_map::iterator begin = ostm.buffer.find(from);
    if (begin == ostm.buffer.end())
    {
        HC_LOG_DEBUG("nothing to send");
        return;
    }
    omp_ptr_map::iterator end = ostm.buffer.upper_bound(to);
    for (omp_ptr_map::iterator i = begin; i != end; ++i)
    {
        omp_ptr o = i->second;
        HC_LOG_DEBUG("send seq_nr " << o->seq_nr);
        message::serialize_as_message(s,
                                      async_send,
                                      o->stm_id,
                                      o->sck_id,
                                      o->seq_nr,
                                      o->content_size,
                                      o->content);
        ++sent;
    }
    HC_LOG_DEBUG(sent << " messages sent");
    s.flush();
    ostm.pending_sends += sent;
    ostm.last_sent += sent;
}

typedef std::map<request_id, outgoing_synchronous_message*> requests_map;

} // namespace <anonymous>

namespace hamcast { namespace detail {

client_channel_msg::~client_channel_msg() { }

} } // namespace hamcast::detail

namespace hamcast { namespace ipc {

bool socket_data::has_pending_sends()
{
    return !buffer.empty();
}

client_channel::ptr client_channel::get()
{
    boost::mutex::scoped_lock guard(m_instance_mtx);
    if (!m_instance)
    {
#       ifdef HC_USE_FAKE_MIDDLEWARE
        m_instance.reset(create_fake_client_channel());
#       else
        m_instance.reset(new client_channel());
#       endif
        m_instance->run();
    }
    return m_instance;
}

client_channel::client_channel()
    : channel(new_client_socket_io())
    , rd_state(rd_msg_type)
{
    util::deserializer d(m_source);
    d >> m_max_msg_size;
    wb.reserve(default_max_write_buffer_size);
}

client_channel::client_channel(const util::source::ptr& in,
                               const util::sink::ptr& out)
    : channel(in, out), m_max_msg_size(default_max_msg_size)
    , rd_state(rd_msg_type)
{
    wb.reserve(default_max_write_buffer_size);
}

void client_channel::register_d_ptr(socket_id sid,
                                    const multicast_socket::d_ptr &ptr)
{
    push(new reg_d_ptr(sid, ptr));
}

void client_channel::register_callback(const membership_event_callback& cb)
{
    push(new register_callback_msg(cb));
}

void client_channel::register_receive_callback(socket_id sock,
                                               const receive_callback& cb) {
    push(new register_recv_cb_msg(sock, cb));
}



// wiretap create and delete operations
void wiretap(socket_data_map& smap,
             outgoing_synchronous_message* request,
             const sync_response_view& resp_view)
{
    if (resp_view.exc_id() != eid_none) return;
    switch (request->fid)
    {

     case fid_create_socket:
        {
            HC_REQUIRE(resp_view.content());
            socket_id sid;
            // lifetime scope of d
            {
                util::const_buffer cb(resp_view.content_size(),
                                      resp_view.content());
                util::deserializer d(new util::read_buffer(cb));
                d >> sid;
            }
            HC_REQUIRE(smap.insert(std::make_pair(sid, socket_data())).second);
        }
        break;

     case fid_create_send_stream:
        {
            HC_REQUIRE(resp_view.content() && request->buf);
            // fetch returned stream id
            stream_id stm_id;
            // lifetime scope of d
            {
                util::const_buffer cb(resp_view.content_size(),
                                      resp_view.content());
                util::deserializer d(new util::read_buffer(cb));
                d >> stm_id;
            }
            // fetch socket id from request
            socket_id sck_id;
            {
                util::const_buffer cb2(request->buf->size(),
                                       request->buf->data());
                util::deserializer d2(new util::read_buffer(cb2));
                d2 >> sck_id;
            }
            socket_data_map::iterator i = smap.find(sck_id);
            HC_REQUIRE(i != smap.end());
        }
        break;

     case fid_delete_socket:
        {
            HC_REQUIRE(request->buf);
            // fetch socket id from request
            socket_id sck_id;
            {
                util::const_buffer cb2(request->buf->size(),
                                       request->buf->data());
                util::deserializer d2(new util::read_buffer(cb2));
                d2 >> sck_id;
            }
            socket_data_map::iterator i = smap.find(sck_id);
            HC_REQUIRE(i != smap.end());
            i->second.state = socket_closed_state;
        }
        break;

     default: break;

    }
}

void client_channel::ipc_read()
{
    HC_LOG_TRACE("");
    wb.append_some_from(*m_source, wb.remaining());
    rd_buf.reset(wb.size(), wb.data());
    bool finished = false;
    while (!finished)
    {
        switch (rd_state)
        {

         case rd_msg_type:
            try_read(m_msg_type_field, finished, rd_field1);
            if (finished) break;

         case rd_field1:
            try_read(m_field1, finished, rd_field2);
            if (finished) break;

         case rd_field2:
            try_read(m_field2, finished, rd_field3);
            if (finished) break;

         case rd_field3:
            try_read(m_field3, finished, rd_content_size);
            if (finished) break;

         case rd_content_size:
            try_read(m_content_size, finished, rd_content);
            if (finished) break;

         case rd_content:
            if (m_content_size > 0 && rd_buf.available() < m_content_size)
            {
                // wait for new data
                finished = true;
            }
            else
            {
                message_type mtype = static_cast<message_type>(m_msg_type_field);
                switch (mtype)
                {

                 case async_recv:
                 {
                    HC_LOG_SCOPE("async_recv", "");
                    HC_REQUIRE(m_content_size > 0);
                    util::read_buffer rdbuf(m_content_size, rd_buf.data());
                    ref_counted::add_ref(&rdbuf); // deserializer modifies refcount
                    { // lifetime scope of deserializer d
                        util::deserializer d(&rdbuf);
                        socket_id sck_id = static_cast<socket_id>(m_field2);
                        socket_data_map::iterator i = m_sockets.find(sck_id);
                        if (i == m_sockets.end())
                        {
                            HC_LOG_ERROR("socket " << sck_id << " not found");
                        }
                        else
                        {
                            socket_data& sdata = i->second;
                            if (sdata.mcast_obj)
                            {
                                // this socket uses blocking receive
                                multicast_packet mp;
                                d >> mp;
                                multicast_socket::async_recv(sdata.mcast_obj, mp);
                                rd_buf.skip(m_content_size);
                            }
                            else if (sdata.recv_cb)
                            {
                                //size_t before = rdbuf.available();
                                // this socket uses asynchronous callbacks
                                uri from;
                                d >> from;
                                boost::uint32_t data_size;
                                d >> data_size;
                                //size_t consumed = before - rdbuf.available();
                                //size_t data_size = m_content_size - consumed;
                                sdata.recv_cb(from, data_size, rdbuf.data());
                                rd_buf.skip(m_content_size);
                            }
                            else
                            {
                                HC_LOG_ERROR("received multicast packet for a "
                                             "socket with neither a d_ptr nor "
                                             "a callback assigned");
                            }
                        }
                    }
                 }
                 break;

                 case sync_response:
                 {
                    HC_LOG_SCOPE("sync_response", "");
                    char* content = 0;
                    if (m_content_size > 0)
                    {
                        content = new char[m_content_size];
                        rd_buf.read(m_content_size, content);
                    }
                    // message takes ownership of content
                    message::ptr msg =  message::create(mtype, m_field1, m_field2,
                                                        m_field3, m_content_size,
                                                        content);
                    sync_response_view srv(*msg);
                    requests_map::iterator i = m_requests.find(srv.req_id());
                    if (i == m_requests.end())
                    {
                        HC_LOG_ERROR("request " << srv.req_id() << " not found");
                    }
                    else
                    {
                        outgoing_synchronous_message* osm = i->second;
                        m_requests.erase(i);
                        wiretap(m_sockets, osm, srv);
                        osm->fmsg->set(msg);
                        delete osm;
                    }
                 }
                 break;

                 case retransmit:
                 {
                    HC_REQUIRE(m_content_size == 0);
                    stream_id stm_id = m_field1;
                    socket_id sck_id = m_field2;
                    sequence_number seq_nr = m_field3;
                    HC_LOG_SCOPE("retransmit", "seq_nr = " << seq_nr);
                    socket_data_map::iterator i = m_sockets.find(sck_id);
                    if (i != m_sockets.end())
                    {
                        util::serializer s(m_sink);
                        socket_data& sdata = i->second;
                        sequence_number from = seq_nr;
                        sequence_number to = sdata.last_sent;
                        do_send(s, sdata, from, to);
                    }
                 }
                 break;

                 case cumulative_ack:
                 {
                    HC_REQUIRE(m_content_size == 0);
                    stream_id stm_id = m_field1;
                    socket_id sck_id = m_field2;
                    sequence_number seq_nr = m_field3;
                    HC_LOG_SCOPE("cumulative_ack", "socket = " << sck_id
                                                   << ", seq_nr = " << seq_nr);
                    socket_data_map::iterator i = m_sockets.find(sck_id);
                    if (i != m_sockets.end())
                    {
                        util::serializer s(m_sink);
                        socket_data& sdata = i->second;
                        boost::uint32_t released = drop_smaller_or_equal_seq_nrs(sdata, seq_nr);
                        multicast_socket::release_storage(sdata.mcast_obj, released);
                        // make sure there are still pending sends
                        if (sdata.pending_sends < static_cast<long>(max_pending_sends))
                        {
                            sequence_number from = sdata.last_sent + 1;
                            sequence_number to = from + (max_pending_sends - sdata.pending_sends);
                            do_send(s, sdata, from, to);
                        }
                        if (sdata.state == socket_about_to_close_state)
                        {
                            HC_LOG_DEBUG("socket marked as closed");
                            if (!sdata.has_pending_sends()) {
                                HC_LOG_DEBUG("no more pending sends");
                                if (sdata.delayed_close_message)
                                {
                                    HC_LOG_DEBUG("close message re-enqueued");
                                    push(sdata.delayed_close_message);
                                    sdata.delayed_close_message = 0;
                                }
                                else
                                {
                                    HC_LOG_DEBUG("delayed_close_message == NULL");
                                }
                            }
                        }
                    }
                    else
                    {
                        HC_LOG_ERROR("ACK on an unknown socket");
                    }
                }
                break;

                 case async_event:
                    {
                        HC_LOG_SCOPE("async_event", "");
                        util::deserializer d(new util::read_buffer(m_content_size,
                                                                   rd_buf.data()));
                        uri grp;
                        d >> grp;
                        if (grp.empty())
                        {
                            throw std::logic_error("empty group name");
                        }
                        // verify event type
                        switch (static_cast<membership_event_type>(m_field1))
                        {
                         case join_event:
                         case leave_event:
                         case new_source_event:
                            break;
                         default: throw std::logic_error("invalid event type");
                        }
                        membership_event_type met
                                = static_cast<membership_event_type>(m_field1);
                        interface_id iid = static_cast<interface_id>(m_field2);
                        membership_event event(grp, iid, met);
                        for (std::list<membership_event_callback>::iterator i = m_cbs.begin();
                             i != m_cbs.end(); ++i)
                        {
                            (*i)(event);
                        }
                    }
                    break;

                 default:
                    HC_LOG_FATAL("ipc_read: invalid message type: " << mtype);
                    throw std::logic_error("invalid message type");

                }
                if (!finished)
                {
                    rd_state = rd_msg_type;
                }
            }
            break;

         default: throw std::logic_error("invalid state");

        }
    }
    wb.erase(0, rd_buf.read_position());
}

void client_channel::poll_messages(size_t num)
{
    util::serializer s(m_sink);
    client_channel_msg* e = 0;
    for (size_t i = 0; i < num; ++i)
    {
        e = m_queue.try_pop();
        HC_REQUIRE(e != 0);
        switch (e->type)
        {

         case reg_d_ptr_t:
            {
                HC_LOG_SCOPE("reg_d_ptr_t", "");
                reg_d_ptr* rdp = e->downcast<reg_d_ptr>();
                m_sockets[rdp->sck_id].mcast_obj = rdp->ptr;
                delete rdp;
            }
            break;

        case reg_recv_cb_t:
            {
                HC_LOG_SCOPE("reg_recv_cb_t", "");
                register_recv_cb_msg* rrcb = e->downcast<register_recv_cb_msg>();
                m_sockets[rrcb->sck_id].recv_cb = rrcb->fun;
                delete rrcb;
            }
            break;

         case omp_t:
            {
                HC_LOG_SCOPE("omp_t", "");
                outgoing_mcast_packet* omp = e->downcast<outgoing_mcast_packet>();
                socket_data_map::iterator i = m_sockets.find(omp->sck_id);
                if (i == m_sockets.end())
                {
                    HC_LOG_ERROR("socket " << omp->sck_id << " not found");
                    delete omp;
                }
                else
                {
                    socket_data& sdata = i->second;
                    omp->seq_nr = sdata.seq_nrs.next();
                    HC_LOG_DEBUG("seq_nr = " << omp->seq_nr);
                    if (omp->seq_nr == 0 || omp->seq_nr == (sdata.last_sent + 1))
                    {
                        if (sdata.pending_sends < static_cast<long>(max_pending_sends))
                        {
                            HC_LOG_DEBUG("send seq_nr " << omp->seq_nr);
                            sdata.pending_sends += 1;
                            message::serialize_as_message(s,
                                                          async_send,
                                                          omp->stm_id,
                                                          omp->sck_id,
                                                          omp->seq_nr,
                                                          omp->content_size,
                                                          omp->content);
                            s.flush();
                            sdata.last_sent = omp->seq_nr;
                        }
                    }
                    sdata.buffer.insert(std::make_pair(omp->seq_nr, omp));
                }
            }
            break;

         case osm_t:
            {
                outgoing_synchronous_message* osm = e->downcast<outgoing_synchronous_message>();
                HC_LOG_SCOPE("osm_t", "fid = " << osm->fid);
                bool do_not_send = false;
                if (osm->fid == fid_delete_socket)
                {
                    socket_id sck_id;
                    util::const_buffer mcb(osm->buf->size(),
                                           osm->buf->data());
                    util::deserializer md(new util::read_buffer(mcb));
                    md >> sck_id;
                    socket_data_map::iterator i = m_sockets.find(sck_id);
                    HC_REQUIRE(i != m_sockets.end());
                    socket_data& sdata = i->second;
                    if (sdata.has_pending_sends())
                    {
                        HC_LOG_DEBUG("IPC close message delayed, because "
                                     "socket has still pendings sends");
                        sdata.delayed_close_message = osm;
                        sdata.state = socket_about_to_close_state;
                        // do not send message now
                        do_not_send = true;
                    }
                }
                if (!do_not_send)
                {
                    request_id rid = req_ids.next();
                    message::serialize_as_message(s,
                                                  sync_request,
                                                  osm->fid,
                                                  rid,
                                                  0,
                                                  static_cast<boost::uint32_t>(osm->buf->size()),
                                                  osm->buf->data());
                    s.flush();
                    HC_LOG_DEBUG("request id = " << rid);
                    m_requests[rid] = osm;
                }
            }
            break;

         case reg_cb_t:
            {
                m_cbs.push_back(e->downcast<register_callback_msg>()->fun);
                delete e;
            }
            break;

         default:
            HC_LOG_FATAL("invalid message type: " << e->type);
            throw std::logic_error("invalid message type");

        }
    }
}

void client_channel::on_exit(const std::string&)
{
    HC_LOG_FATAL("connection to middleware lost");
    abort();
}

void client_channel::send_async_data(socket_id sock, boost::uint16_t stream,
                                     boost::uint32_t ct_size, char* ct)
{
    push(new outgoing_mcast_packet(sock, stream, ct_size, ct));
}

message::ptr client_channel::send_sync_request(function_id fid,
                                               const util::const_buffer& buf)
{
    util::future<message::ptr> fmsg;
    push(new outgoing_synchronous_message(fid, &buf, &fmsg));
    return fmsg.get();
}

void client_channel::push(detail::client_channel_msg* msg)
{
    HC_MEMORY_BARRIER();
    m_queue.unsafe_push(msg);
    HC_MEMORY_BARRIER();
    notify_message();
}

} } // namespace hamcast::ipc
