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

#include <iostream>
#include <algorithm>

#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>

#include "hamcast/multicast_socket.hpp"

#include "hamcast/uri.hpp"
#include "hamcast/util/serializer.hpp"
#include "hamcast/util/read_buffer.hpp"
#include "hamcast/util/deserializer.hpp"
#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/storage_semaphore.hpp"

#include "hamcast/ipc/api.hpp"
#include "hamcast/ipc/client_channel.hpp"

using std::cout;
using std::endl;

namespace {

using namespace hamcast;
using namespace hamcast::ipc;
using namespace hamcast::util;

struct out_stream
{

    bool m_valid;
    boost::uint16_t m_id;

 public:

    out_stream() : m_valid(false) { }

    void set_id(boost::uint16_t new_id)
    {
        m_valid = true;
        m_id = new_id;
    }

    inline bool valid() const { return m_valid; }

    inline boost::uint16_t id() const { return m_id; }

};

typedef std::map<hamcast::uri, out_stream> out_stream_map;

struct mcast_packet_element
{
    mcast_packet_element* next;
    hamcast::multicast_packet mcast_packet;
    mcast_packet_element(const hamcast::multicast_packet& mp)
        : next(0), mcast_packet(mp)
    {
    }
};

} // namespace <anonymous>

namespace hamcast { namespace detail {

struct multicast_socket_private : public ref_counted
{

    socket_id m_id;
    // caches client_channel::get
    client_channel::ptr m_cc;

    boost::uint32_t m_max_msg_size;

    // in_stream m_in_stream;
    single_reader_queue<mcast_packet_element> m_in_stream;
    out_stream_map m_out_streams;

    util::storage_semaphore<default_max_buffer_size, util::nonblocking_t> m_in_buffer;
    util::storage_semaphore<default_max_buffer_size> m_out_buffer;

    multicast_socket_private(socket_id sid)
        : m_id(sid), m_cc(client_channel::get())
    {
        m_max_msg_size = m_cc->max_msg_size();
    }

    inline socket_id id() const { return m_id; }

    void send(const uri& u, size_t msg_len, const void* buf)
    {
        HC_REQUIRE_VERBOSE(msg_len <= m_max_msg_size, "message too big");
        boost::uint32_t buf_len = static_cast<boost::uint32_t>(msg_len);
        if (!u.empty())
        {
            out_stream& ostm = m_out_streams[u];
            if (!ostm.valid())
            {
                ostm.set_id(create_send_stream(m_id, u));
            }
            // copy content of buf
            char* buf_copy = new char[buf_len];
            memcpy(buf_copy, buf, buf_len);
            // decrease buffer size
            m_out_buffer.acquire(std::max(buf_len, static_cast<boost::uint32_t>(min_buffer_chunk)));
            // send
            m_cc->send_async_data(m_id, ostm.id(), buf_len, buf_copy);
        }
    }

    inline boost::uint32_t acquire_size(const multicast_packet& mp)
    {
        return mp.size();
    }

    void async_recv(const multicast_packet& mp)
    {
        if (m_in_buffer.try_acquire(acquire_size(mp)))
        {
            m_in_stream.push(new mcast_packet_element(mp));
        }
        else
        {
            HC_LOG_DEBUG("Package dropped (full buffer)");
            cout << "package dropped" << endl;
        }
    }

    void release_storage(boost::int32_t bytes)
    {
        m_out_buffer.release(std::max(bytes, static_cast<boost::int32_t>(min_buffer_chunk)));
    }

    inline bool try_receive(multicast_packet& mp)
    {
        mcast_packet_element* e = m_in_stream.try_pop();
        if (e)
        {
            m_in_buffer.release(acquire_size(e->mcast_packet));
            mp = e->mcast_packet;
            delete e;
            return true;
        }
        return false;
    }

    inline bool try_receive(multicast_packet& mp,
                            boost::uint16_t msecs)
    {
        mcast_packet_element* e = m_in_stream.try_pop(boost::posix_time::milliseconds(msecs));
        if (e)
        {
            m_in_buffer.release(acquire_size(e->mcast_packet));
            mp = e->mcast_packet;
            delete e;
            return true;
        }
        return false;
    }

    inline multicast_packet receive()
    {
        mcast_packet_element* e = m_in_stream.pop();
        m_in_buffer.release(acquire_size(e->mcast_packet));
        multicast_packet result = e->mcast_packet;
        delete e;
        return result;
    }

};

} } // namespace hamcast::detail

namespace hamcast {

multicast_socket::multicast_socket() : d()
{
    boost::uint32_t my_id = ipc::create_socket();
    d.reset(new detail::multicast_socket_private(my_id));
    client_channel::get()->register_d_ptr(my_id, d);
}

multicast_socket::~multicast_socket()
{
    ipc::delete_socket(d->m_id);
}

void multicast_socket::send(const uri& u, size_t msg_len, const void* buf)
{
    HC_REQUIRE(buf != 0 && msg_len > 0);
    HC_REQUIRE(!u.empty());
    d->send(u, msg_len, buf);
}

multicast_packet multicast_socket::receive()
{
    return d->receive();
}

bool multicast_socket::try_receive(multicast_packet& storage)
{
    return d->try_receive(storage);
}

bool multicast_socket::try_receive(multicast_packet& storage,
                                   boost::uint16_t milliseconds)
{
    return d->try_receive(storage, milliseconds);
}

void multicast_socket::join(const uri& what)
{
    ipc::join(d->m_id, what);
}

void multicast_socket::set_ttl(boost::uint32_t value)
{
    ipc::set_ttl(d->m_id, value);
}

void multicast_socket::leave(const uri& group)
{
    ipc::leave(d->m_id, group);
}

void multicast_socket::release_storage(d_ptr& sock_private,
                                       boost::int32_t bytes)
{
    sock_private->release_storage(bytes);
}

void multicast_socket::async_recv(d_ptr& sock_private,
                                  const multicast_packet& mp)
{
    sock_private->async_recv(mp);
}

void multicast_socket::set_interfaces(const std::vector<interface_id>& ifs)
{
    ipc::set_sock_interfaces(d->m_id, ifs);
}

void multicast_socket::set_interface(interface_id iid)
{
    std::vector<interface_id> tmp;
    tmp.push_back(iid);
    set_interfaces(tmp);
}

std::vector<interface_id> multicast_socket::interfaces()
{
    return ipc::get_sock_interfaces(d->m_id);
}

void multicast_socket::add_ref::operator()(detail::multicast_socket_private* rc)
{
    ref_counted::add_ref(rc);
}

void multicast_socket::release::operator()(detail::multicast_socket_private* rc)
{
    ref_counted::release(rc);
}

} // namespace hamcast
