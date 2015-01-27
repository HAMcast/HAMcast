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

#include <set>
#include <iostream>
#include <algorithm>
#include <boost/thread.hpp>

#include "session.hpp"
#include "middleware.hpp"
#include "tech_module.hpp"

namespace // static data
{
    boost::mutex s_is_img_mtx;
    volatile bool s_is_img = false;
}

using boost::bind;
using namespace hamcast::ipc;

namespace hamcast { namespace middleware {

void session_add_ref::operator()(session* rc)
{
    ref_counted::add_ref(rc);
}

void session_release::operator()(session* rc)
{
    ref_counted::release(rc);
}

namespace {

void cb_set_is_img(session*, util::unit&, bool value)
{
    boost::mutex::scoped_lock guard(s_is_img_mtx);
    s_is_img = value;
}

void cb_get_is_img(session*, bool& result)
{
    boost::mutex::scoped_lock guard(s_is_img_mtx);
    result = s_is_img;
}

void cb_cr_sock_stream(session* self,
                       ipc::stream_id& result,
                       socket_id sid,
                       const uri& group)
{
    HC_LOG_TRACE("sid = " << sid << ", group = " << group.str());
    HC_REQUIRE(!group.empty());
    result = self->socket(sid).add_stream(group);
    HC_LOG_DEBUG("stream_id (result) = " << result);
}

void cb_get_sock_ifs(session* self,
                     std::vector<interface_id>& result,
                     socket_id sid)
{
    HC_LOG_TRACE("sid = " << sid);
    const tech_module_vec& iv = self->socket(sid).interfaces();
    HC_REQUIRE(!iv.empty());
    result.clear();
    result.reserve(iv.size());
    for (tech_module_vec::const_iterator i(iv.begin()); i != iv.end(); ++i)
    {
        const tech_module::ptr& tm = *i;
        HC_REQUIRE(tm.get() != 0);
        result.push_back((*i)->iface_id());
    }
}

void cb_designated_host(session*,
                        bool& result,
                        interface_id iid,
                        const uri& group)
{
    HC_LOG_TRACE("iid = " << iid << ", group = " << group.str());
    tech_module::ptr tm = tech_module::instance_by_iid(iid);
    HC_REQUIRE_VERBOSE(tm.get() != 0,
                            "No such interface (id = " << iid << ")");
    result = tm->designated_host(group);
}

void cb_set_sock_ifs(session* self,
                     hamcast::util::unit&,
                     socket_id sid,
                     const std::vector<interface_id>& ifs)
{
    HC_REQUIRE(!ifs.empty());
    HC_LOG_TRACE("sid = " << sid);
    // get pointers
    std::vector<tech_module::ptr> tms;
    for (std::vector<interface_id>::const_iterator i = ifs.begin();
         i != ifs.end();
         ++i)
    {
        tech_module::ptr tm = tech_module::instance_by_iid(*i);
        HC_REQUIRE(tm.get() != 0);
        tms.push_back(tm);
    }
    // delegate call
    self->socket(sid).set_interfaces(tms);
}

void cb_add_sock_if(session* self,
                    hamcast::util::unit& result,
                    socket_id sid,
                    interface_id iid)
{
    HC_LOG_TRACE("sid = " << sid << ", iid = " << iid);
    std::vector<interface_id> ifs;
    cb_get_sock_ifs(self, ifs, sid);
    std::vector<interface_id>::iterator i = std::find(ifs.begin(),
                                                      ifs.end(),
                                                      iid);
    if (i == ifs.end())
    {
        ifs.push_back(iid);
        cb_set_sock_ifs(self, result, sid, ifs);
    }
    // else: don't add interface; already set
}

void cb_rm_sock_if(session* self,
                   hamcast::util::unit& result,
                   socket_id sid,
                   interface_id iid)
{
    HC_LOG_TRACE("sid = " << sid << ", iid = " << iid);
    std::vector<interface_id> ifs;
    cb_get_sock_ifs(self, ifs, sid);
    std::vector<interface_id>::iterator i = std::find(ifs.begin(),
                                                      ifs.end(),
                                                      iid);
    if (i != ifs.end())
    {
        ifs.erase(i);
        cb_set_sock_ifs(self, result, sid, ifs);
    }
    // else: couldn't remove something that's not there
}

void cb_neighbor_set(session*,
                     std::vector<uri>& result,
                     interface_id iid)
{
    HC_LOG_TRACE("iid = " << iid);
    tech_module::ptr tm = tech_module::instance_by_iid(iid);
    HC_REQUIRE_VERBOSE(tm.get() != 0,
                            "No such interface (id = " << iid << ")");
    tm->neighbor_set(result);
}

void cb_children_set(session*,
                     std::vector<uri>& result,
                     interface_id iid,
                     const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("iid = " << iid << ", group = " << group.str());
    tech_module::ptr tm = tech_module::instance_by_iid(iid);
    HC_REQUIRE_VERBOSE(tm.get() != 0,
                            "No such interface (id = " << iid << ")");
    tm->children_set(result, group);
}

template<typename K, typename V>
void set_from_map(const std::map<K,V>& container, const K& key, V& storage)
{
    typename std::map<K,V>::const_iterator i = container.find(key);
    if (i != container.end()) storage = i->second;
}

void cb_get_ifs(session*, std::vector<hamcast::interface_property>& result)
{
    static std::string s_if_name = "if_name";
    static std::string s_if_addr = "if_addr";
    static std::string s_if_tech = "if_tech";
    HC_LOG_TRACE("");
    std::vector<tech_module::ptr> mods = tech_module::instances();
    for (size_t i = 0; i < mods.size(); ++i)
    {
        tech_module& tm = *(mods[i]);
        hamcast::interface_property p;
        p.id = tm.iface_id();
        // default values
        p.name = "";
        p.address = "";
        p.technology = "";
        const string_map& sd = tm.service_discovery_results();
        set_from_map(sd, s_if_name, p.name);
        set_from_map(sd, s_if_addr, p.address);
        set_from_map(sd, s_if_tech, p.technology);
        result.push_back(p);
    }
}

void cb_cr_sock(session* self, socket_id& result)
{
    HC_LOG_TRACE("");
    result = self->add_new_socket();
    HC_LOG_DEBUG("result = " << result);
}

void cb_rm_sock(session* self, hamcast::util::unit&, socket_id sid)
{
    HC_LOG_TRACE("sid = " << sid);
    self->remove_socket(sid);
}

void cb_parent_set(session*,
                   std::vector<uri>& result,
                   interface_id iid,
                   const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("iid = " << iid << ", group = " << group.str());
    tech_module::ptr tm = tech_module::instance_by_iid(iid);
    HC_REQUIRE_VERBOSE(tm.get() != 0,
                            "No such interface (id = " << iid << ")");
    tm->parent_set(result, group);
}

void cb_group_set(session*,
                  std::vector<std::pair<uri, boost::uint32_t> >& result,
                  interface_id iid)
{
    HC_LOG_TRACE("iid = " << iid);
    tech_module::ptr tm = tech_module::instance_by_iid(iid);
    HC_REQUIRE_VERBOSE(tm.get() != 0,
                            "No such interface (id = " << iid << ")");
    tm->group_set(result);
}

void cb_set_ttl(session* self,
                hamcast::util::unit&,
                socket_id sid,
                boost::uint8_t ttl_value)
{
    HC_LOG_TRACE("sid = " << sid << ", ttl = " << ttl_value);
    self->socket(sid).set_ttl(ttl_value);
}

void cb_leave(session* self,
              hamcast::util::unit&,
              socket_id sid,
              const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("sid = " << sid << ", u = " << group.str());
    self->socket(sid).leave(group);
}

void cb_join(session* self,
             hamcast::util::unit&,
             socket_id sid,
             const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("sid = " << sid << ", u = " << group.str());
    self->socket(sid).join(group);
}

void cb_enable_events(session* self)
{
    HC_LOG_TRACE("");
    self->enable_events();
}

void cb_disable_events(session* self)
{
    HC_LOG_TRACE("");
    self->disable_events();
}

void cb_get_atomic_msg_size(session*,
                            boost::uint32_t& result,
                            interface_id iid)
{
    HC_LOG_TRACE("");
    tech_module::ptr tm = tech_module::instance_by_iid(iid);
    HC_REQUIRE_VERBOSE(tm.get() != 0,
                       "No such interface (id = " << iid << ")");
    result = tm->atomic_msg_size();
}

} // namespace <anonymous>

void session::enable_events()
{
    if (m_subscribed_events == false)
    {
        session_ptr self(this);
        subscribe_events(self);
        m_subscribed_events = true;
    }
}

void session::disable_events()
{
    if (m_subscribed_events == true)
    {
        session_ptr self(this);
        unsubscribe_events(self);
        m_subscribed_events = false;
    }
}

session_msg::~session_msg()
{
}

session::session(const util::source::ptr& in, const util::sink::ptr& out)
    : ipc::channel(in, out, force_ack_ms_interval)
    , m_subscribed_events(false), m_dispatcher(this)
    , m_serializer(out), m_deserializer(in)
{
    m_dispatcher.init(cb_cr_sock_stream,
                      cb_get_sock_ifs,
                      cb_designated_host,
                      cb_add_sock_if,
                      cb_set_sock_ifs,
                      cb_rm_sock_if,
                      cb_neighbor_set,
                      cb_children_set,
                      cb_get_ifs,
                      cb_cr_sock,
                      cb_rm_sock,
                      cb_parent_set,
                      cb_group_set,
                      cb_set_ttl,
                      cb_leave,
                      cb_join,
                      cb_enable_events,
                      cb_disable_events,
                      cb_get_atomic_msg_size,
                      cb_set_is_img,
                      cb_get_is_img);
}

void session::send(const ipc::message::ptr& what)
{
    if (what) push(new session_msg(what, outgoing_ipc_msg_t));
}

void session::create(const util::source::ptr& in, const util::sink::ptr& out)
{
    (new session(in, out))->run();
}

void session::on_exit(const std::string& err_str)
{
    HC_LOG_TRACE("err_str = " << err_str);
    disable_events();
    for (socket_map::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
    {
        i->second->leave_all();
    }
    m_sockets.clear();
}

socket_id session::add_new_socket()
{
    socket_id new_sid = m_socket_id_generator.next();
    m_sockets.insert(socket_map::value_type(new_sid, new socket_proxy(new_sid,
                                                                      this)));
    return new_sid;
}

socket_proxy& session::socket(socket_id sid)
{
    socket_map::iterator i = m_sockets.find(sid);
    HC_REQUIRE_VERBOSE(i != m_sockets.end(),
                            "No such socket (socket id = " << sid << ")");
    return *(i->second);
}

void session::remove_socket(socket_id sid)
{
    socket_map::iterator i = m_sockets.find(sid);
    if (i != m_sockets.end())
    {
        i->second->leave_all();
        m_sockets.erase(i);
    }
}

session::~session()
{
    session_msg* sm;
    while ((sm = m_queue.try_pop()) != 0)
    {
        delete sm;
    }
}

typedef std::vector<tech_module::ptr> tech_module_vec;

template<typename T>
inline void flush_if_needed(util::serializer& s, T& q)
{
    //static const boost::posix_time::microseconds wtime(50);
    //if (!q.wait_for_data(wtime)) s.flush();
    if (!q.front()) s.flush();
}

void session::handle_poll_timeout()
{
    HC_LOG_TRACE("");
    bool ack_forced = false;
    for (socket_map::iterator i = m_sockets.begin();
         i != m_sockets.end();
         ++i)
    {
        if (i->second->force_acks(m_serializer) && !ack_forced)
        {
            ack_forced = true;
        }
    }
    //flush_if_needed(s, m_queue);
    if (ack_forced)
    {
        HC_LOG_DEBUG("ACK forced");
        m_serializer.flush();
    }
    else flush_if_needed(m_serializer, m_queue);
}

void session::poll_messages(size_t num)
{
    HC_LOG_TRACE("num = " << num);
    session_ptr self_ptr(this);
    for (size_t i = 0; i < num; ++i)
    {
        session_msg* sm = m_queue.try_pop();
        HC_REQUIRE(sm != NULL);
        switch (sm->type)
        {

         case receive_loop_killed_t:
            HC_LOG_DEBUG("receive loop killed ... quit");
            delete sm;
            m_loop_done = true;
            return;

         case outgoing_ipc_msg_t:
            m_serializer << *(sm->msg);
            flush_if_needed(m_serializer, m_queue);
            //s.flush();
            break;

         case outgoing_mcast_pckt_t:
         {
            socket_map::iterator i = m_sockets.find(sm->sck_id);
            if (i != m_sockets.end())
            {
                this->socket(sm->sck_id).delivered(sm->data_size);
                ipc::message::serialize_as_message(m_serializer,
                                                   sm->sck_id,
                                                   sm->source,
                                                   sm->data_size,
                                                   sm->raw_data);
                flush_if_needed(m_serializer, m_queue);
            }
        }
        break;

         case incoming_sync_request_t:
            m_dispatcher.invoke(sm->msg, m_serializer);
            //flush_if_needed(s, m_queue);
            m_serializer.flush();
            break;

         case async_send_job_done_t:
            {
                ipc::async_send_view asv(*(sm->msg));
                socket_map::iterator i = m_sockets.find(asv.sck_id());
                if (i != m_sockets.end())
                {
                    i->second->async_job_done(asv, m_serializer);
                }
                else
                {
                    HC_LOG_DEBUG("Unknown socket: " << asv.sck_id());
                }
                flush_if_needed(m_serializer, m_queue);
            }
            break;

         case membership_event_t:
            ipc::message::serialize_as_message(m_serializer, sm->mbs_event);
            flush_if_needed(m_serializer, m_queue);
            break;

        }
        delete sm;
    }
}

void session::ipc_read()
{
    HC_LOG_TRACE("");
    session_ptr self_ptr(this);
    try
    {
        do
        {
            message::ptr msg;
            m_deserializer >> msg;
            if (msg->type() == ipc::sync_request)
            {
                HC_LOG_DEBUG("sync_request"
                             << "; request id = " << sync_request_view(*msg).req_id()
                             << ", fid = " << sync_request_view(*msg).fun_id());
                m_dispatcher.invoke(msg, m_serializer);
                m_serializer.flush();
            }
            else if (msg->type() == ipc::async_send)
            {
                ipc::async_send_view asv(*msg);
                HC_LOG_DEBUG("async_send"
                             << "; socket = " << asv.sck_id()
                             << ", stream = " << asv.stm_id()
                             << ", seq nr = " << asv.seq_nr());
                socket_map::iterator i = m_sockets.find(asv.sck_id());
                if (i != m_sockets.end())
                {
                    i->second->handle_async_send(asv, m_serializer, self_ptr);
                }
                else
                {
                    HC_LOG_DEBUG("Unknown socket: " << asv.sck_id());
                }
            }
            else
            {
                std::ostringstream err;
                err << "Client sendet illegal message type: " << msg->type();
                std::string err_str = err.str();
                HC_LOG_FATAL(err_str);
                throw std::runtime_error(err_str);
            }
        }
        while (m_deserializer.get_source()->has_buffered_data());
    }
    catch (...)
    {
        push(new session_msg(receive_loop_killed()));
        // rethrow exception (catched by channel::run_receive_loop)
        throw;
    }
}

void session::push(session_msg* msg)
{
    m_queue.unsafe_push(msg);
    HC_MEMORY_BARRIER();
    notify_message();
}

} } // namespace hamcast::middleware
