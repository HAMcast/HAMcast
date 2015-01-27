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

#ifndef CLIENT_CHANNEL_MSG_HPP
#define CLIENT_CHANNEL_MSG_HPP

#include "hamcast/socket_id.hpp"
#include "hamcast/multicast_socket.hpp"

#include "hamcast/util/future.hpp"

#include "hamcast/ipc/message.hpp"
#include "hamcast/ipc/stream_id.hpp"
#include "hamcast/ipc/function_id.hpp"
#include "hamcast/ipc/sequence_number.hpp"

namespace hamcast { namespace detail {

enum client_channel_msg_type
{
    omp_t,            // outgoing multicast packet
    osm_t,            // outgoing synchronous message
    reg_d_ptr_t,      // register multicast_socket::d_ptr
    reg_cb_t,         // register callback
    reg_recv_cb_t,    // register receive callback
    imp_t             // incoming multicast packet (fake_client_channel only)
};

struct client_channel_msg
{

    client_channel_msg* next;
    const client_channel_msg_type type;

    inline client_channel_msg(client_channel_msg_type mtype)
        : next(0), type(mtype) { }

    virtual ~client_channel_msg();

    virtual void* addr() = 0;

    // @warning you have to check the type flag first
    template<typename T>
    inline T* downcast()
    {
        return reinterpret_cast<T*>(addr());
    }

};

struct outgoing_mcast_packet : client_channel_msg
{
    char* content;
    socket_id sck_id;
    ipc::stream_id stm_id;
    boost::uint32_t content_size;
    ipc::sequence_number seq_nr;
    outgoing_mcast_packet() : client_channel_msg(omp_t), content(0) { }
    outgoing_mcast_packet(socket_id sock,
                          ipc::stream_id stream,
                          boost::uint32_t ct_size,
                          char* ct)
        : client_channel_msg(omp_t), content(ct)
        , sck_id(sock), stm_id(stream), content_size(ct_size)
    {
    }
    ~outgoing_mcast_packet();
    void* addr();
};

typedef outgoing_mcast_packet* omp_ptr;

struct outgoing_synchronous_message : client_channel_msg
{

    ipc::function_id fid;
    const util::const_buffer* buf;
    util::future<ipc::message::ptr>* fmsg;

    outgoing_synchronous_message(ipc::function_id sync_fid,
                                 const util::const_buffer* cbuf,
                                 util::future<ipc::message::ptr>* msg_future)
        : client_channel_msg(osm_t), fid(sync_fid), buf(cbuf), fmsg(msg_future)
    {
        HC_REQUIRE(cbuf && msg_future);
    }
    void* addr();

};

struct reg_d_ptr : client_channel_msg
{
    socket_id sck_id;
    multicast_socket::d_ptr ptr;
    inline reg_d_ptr(socket_id sid, const multicast_socket::d_ptr& dp)
        : client_channel_msg(reg_d_ptr_t), sck_id(sid), ptr(dp)
    {
    }
    void* addr();
};

struct register_callback_msg : client_channel_msg
{
    membership_event_callback fun;
    inline register_callback_msg(const membership_event_callback& cb_fun)
        : client_channel_msg(reg_cb_t), fun(cb_fun)
    {
    }
    void* addr();
};

struct register_recv_cb_msg : client_channel_msg
{
    typedef boost::function<void (const uri&, size_t, const void*)> receive_callback;
    socket_id sck_id;
    receive_callback fun;
    inline register_recv_cb_msg(socket_id sockid, const receive_callback& cb_fun)
        : client_channel_msg(reg_recv_cb_t), sck_id(sockid), fun(cb_fun)
    {
    }
    void* addr();
};

struct incoming_mcast_packet : client_channel_msg
{
    multicast_packet mcast_packet;
    inline incoming_mcast_packet(const multicast_packet& mp)
        : client_channel_msg(imp_t), mcast_packet(mp)
    {
    }
    void* addr();
};

} } // namespace hamcast::detail

#endif // CLIENT_CHANNEL_MSG_HPP
