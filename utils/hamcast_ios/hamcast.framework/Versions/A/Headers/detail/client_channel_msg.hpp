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
    imp_t,            // incoming multicast packet
    osm_t,            // outgoing synchronous message
    ism_t,            // incoming synchronous message
    reg_d_ptr_t,      // register multicast_socket::d_ptr
    rtm_t,            // retransmit message
    ack_t,            // cumulative ack message
    event_t,          // incoming event
    reg_cb_t          // register callback
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

struct incoming_mcast_packet : client_channel_msg
{
    socket_id sck_id;
    hamcast::multicast_packet mcast_packet;
    incoming_mcast_packet(socket_id sid, const hamcast::multicast_packet& mp)
        : client_channel_msg(imp_t), sck_id(sid), mcast_packet(mp)
    {
    }
    void* addr();
};

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

struct incoming_synchronous_message : client_channel_msg
{
    ipc::message::ptr msg;
    inline incoming_synchronous_message(const ipc::message::ptr& outgoing_msg)
        : client_channel_msg(ism_t)
        , msg(outgoing_msg)
    {
        HC_REQUIRE(outgoing_msg.get() != 0);
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

struct ack_or_rtm_msg : client_channel_msg
{
    ipc::stream_id stm_id;
    socket_id sck_id;
    ipc::sequence_number seq_nr;
    inline ack_or_rtm_msg(client_channel_msg_type mt,
                          ipc::stream_id v1, socket_id v2,
                          ipc::sequence_number v3)
        : client_channel_msg(mt), stm_id(v1), sck_id(v2), seq_nr(v3)
    {
        HC_REQUIRE(mt == rtm_t || mt == ack_t);
    }
    void* addr();
};

struct event_msg : client_channel_msg
{
    membership_event event;
    event_msg(const membership_event& mev)
        : client_channel_msg(event_t), event(mev)
    {
    }
    void* addr();
};

struct register_callback_msg : client_channel_msg
{
    membership_event_callback fun;
    register_callback_msg(const membership_event_callback& cb_fun)
        : client_channel_msg(reg_cb_t), fun(cb_fun)
    {
    }
    void* addr();
};

} } // namespace hamcast::detail

#endif // CLIENT_CHANNEL_MSG_HPP
