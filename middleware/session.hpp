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

#ifndef SESSION_HPP
#define SESSION_HPP

#include <queue>
#include <vector>
#include <utility>
#include <boost/thread.hpp>
#include "hamcast/hamcast.hpp"

#include "middleware.hpp"
#include "session_fwd.hpp"
#include "socket_proxy.hpp"

#include "hamcast/socket_id.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/interface_property.hpp"

#include "hamcast/ipc/api.hpp"
#include "hamcast/ipc/channel.hpp"

#include "hamcast/util/id_generator.hpp"
#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/single_reader_queue.hpp"

namespace hamcast { namespace middleware {

enum session_msg_t
{
    outgoing_ipc_msg_t,
    outgoing_mcast_pckt_t,
    incoming_sync_request_t,
    async_send_job_done_t,
    receive_loop_killed_t,
    membership_event_t
};

struct force_acks { };
struct receive_loop_killed { };

struct session_msg
{

    session_msg* next;
    session_msg_t type;

    // used if type == outgoing_mcast_pckt_t
    size_t data_size;
    void* raw_data;
    socket_id sck_id;
    uri source;

    ipc::message::ptr msg;
    membership_event mbs_event;

    inline session_msg(receive_loop_killed)
        : next(0), type(receive_loop_killed_t)
    {
    }

    inline session_msg(const ipc::message::ptr& mptr, session_msg_t msg_type)
        : next(0), type(msg_type), msg(mptr)
    {
    }

    inline session_msg(session_msg_t msg_type) : next(0), type(msg_type)
    {
    }

    inline session_msg(const membership_event& me)
        : next(0), type(membership_event_t), mbs_event(me)
    {
    }

    virtual ~session_msg();

};

template<size_t DataSize>
struct mcast_packet_msg : session_msg
{

    char data[DataSize];

    mcast_packet_msg() : session_msg(outgoing_mcast_pckt_t) {
        raw_data = data;
    }

};

typedef std::map<socket_id, socket_proxy::ptr> socket_map;

/**
 * @brief Describes a client-specific session.
 *
 * A session consist of a socket_id_generator, an IPC connection and a map
 * of socket_proxys that the user created with create_socket().
 */
class session : public ipc::channel
{

    bool m_subscribed_events;

    socket_map m_sockets;
    util::id_generator<socket_id> m_socket_id_generator;
    util::single_reader_queue<session_msg> m_queue;

    ipc::sync_function_dispatcher<session> m_dispatcher;

    util::serializer m_serializer;
    util::deserializer m_deserializer;

    session(const util::source::ptr& in, const util::sink::ptr& out);

 protected:

    //virtual void send_loop();

    //virtual void receive_loop();

    virtual void ipc_read();

    virtual void poll_messages(size_t num);

    virtual void handle_poll_timeout();


    virtual void on_exit(const std::string &err_str);

 public:

    ~session();

    virtual void send(const ipc::message::ptr& what);

    inline void handle_event(const membership_event& mevent)
    {
        push(new session_msg(mevent));
    }

    inline void async_job_done(const ipc::message::ptr& what)
    {
        push(new session_msg(what, async_send_job_done_t));
    }

    /**
     * @brief Create a new session that uses @p io.
     * @note The created session runs asynchronously.
     * @pre @p io is connected to a verified (compatible) client.
     */
    static void create(const util::source::ptr& in,
                       const util::sink::ptr& out);

    /**
     * @brief Add a new socket to the session.
     */
    socket_id add_new_socket();

    /**
     * @brief Get the socket with id @p sid from the session.
     */
    socket_proxy& socket(socket_id sid);

    inline bool has_socket(socket_id sid)
    {
        return m_sockets.count(sid) != 0;
    }

    /**
     * @brief Remove the socket with id @p sid from this session.
     * @pre @p sid is a valid socket id for this session.
     */
    void remove_socket(socket_id sid);

    void enable_events();

    void disable_events();

    void push(session_msg* msg);

};

} } // namespace hamcast::middleware

#endif // SESSION_HPP
