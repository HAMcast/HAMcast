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

#ifndef HAMCAST_IPC_CLIENT_CHANNEL_HPP
#define HAMCAST_IPC_CLIENT_CHANNEL_HPP

#include <map>

#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "hamcast/socket_id.hpp"
#include "hamcast/ipc/channel.hpp"
#include "hamcast/multicast_socket.hpp"
#include "hamcast/membership_event.hpp"
#include "hamcast/async_multicast_socket.hpp"

#include "hamcast/util/id_generator.hpp"
#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/single_reader_queue.hpp"

#include "hamcast/detail/client_channel_msg.hpp"

namespace hamcast { namespace ipc {

enum socket_state_t
{
    uninitialized_state,
    socket_open_state,
    socket_about_to_close_state,
    socket_closed_state
};

struct socket_data
{
    long pending_sends;
    sequence_number last_sent;
    util::id_generator<sequence_number> seq_nrs;
    std::map<sequence_number, detail::omp_ptr> buffer;
    socket_state_t state;
    detail::outgoing_synchronous_message* delayed_close_message;
    hamcast::multicast_socket::d_ptr mcast_obj;
    async_multicast_socket::receive_callback recv_cb;
    inline socket_data() : pending_sends(0), state(uninitialized_state), delayed_close_message(0) { }
    bool has_pending_sends();
};

typedef std::map<socket_id,socket_data> socket_data_map;

/**
 * @brief An IPC channel describing the connection
 *        from a client to the middleware.
 * @ingroup IPC
 */
class client_channel : public channel
{

    friend class multicast_socket;

    boost::uint32_t m_max_msg_size;

    // hidden to enforce usage of factory member function
    client_channel();

 protected:

    // buffer for outgoing messages
    util::single_reader_queue<detail::client_channel_msg> m_queue;

    // multicast_socket needs access to register_socket() & unregister_socket()
    friend class ::hamcast::multicast_socket;

    // used by fake_client_channel
    client_channel(const util::source::ptr& in, const util::sink::ptr& out);


    typedef std::map<boost::uint32_t, sync_response_view> responses_map;

    multicast_socket::d_ptr create_new_socket();

    virtual void ipc_read();

    virtual void poll_messages(size_t num);

    virtual void on_exit(const std::string&);

 public:

    typedef boost::function<void (const uri&, size_t, const void*)> receive_callback;

    inline boost::uint32_t max_msg_size() const { return m_max_msg_size; }

    // "private"
    void register_d_ptr(socket_id sid, const multicast_socket::d_ptr& ptr);

    /**
     * @brief Send an asynchronous data package to the middleware.
     * @param sock The ID of the outgoing multicast socket.
     * @param stream The ID of the output stream.
     * @param ct_size The size of @p ct.
     * @param ct The content of the data package.
     * @warning client_channel takes ownership of @p ct
     */
    void send_async_data(socket_id sock, boost::uint16_t stream,
                         boost::uint32_t ct_size, char* ct);

    /**
     * @brief Send a synchronous request to the middleware.
     * @param sync_fun_id The ID of the function this IP-call should invoke.
     * @param send_buffer The serialized arguments of this IP-call.
     * @returns The result of the IP-call
     * @throws Any exception that might occur during the invocation
     *         of the IP-call.
     */
    message::ptr send_sync_request(function_id sync_fun_id,
                                   const util::const_buffer& send_buffer);

    /**
     * @brief Register @p cb.
     * @param cb Membership event callback.
     */
    virtual void register_callback(const membership_event_callback& cb);

    virtual void register_receive_callback(socket_id sock,
                                           const receive_callback& cb);

    /**
     * @brief A smart pointer for {@link client_channel} instances.
     */
    typedef intrusive_ptr<client_channel> ptr;

    /**
     * @brief Get a channel that is connected to the middleware.
     * @returns A pointer to the singleton that guards the connection
     *         to the middleware.
     */
    static ptr get();

    util::id_generator<request_id> req_ids;
    socket_data_map m_sockets;
    std::map<request_id, detail::outgoing_synchronous_message*> m_requests;
    std::list<membership_event_callback> m_cbs;
    util::write_buffer<> wb;

    enum rd_state_type
    {
        rd_msg_type,
        rd_field1,
        rd_field2,
        rd_field3,
        rd_content_size,
        rd_content
    }
    rd_state;

    boost::uint16_t m_msg_type_field;
    boost::uint16_t m_field1;
    boost::uint32_t m_field2;
    boost::uint32_t m_field3;
    boost::uint32_t m_content_size;

    util::read_buffer rd_buf;

    template<typename T>
    inline void try_read(T& what, bool& finished, rd_state_type next_state)
    {
        if (rd_buf.available() >= sizeof(T))
        {
            rd_buf.read(sizeof(T), &what);
            rd_state = next_state;
        }
        else
        {
            finished = true;
        }
    }

 protected:

    void push(detail::client_channel_msg* msg);

};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_CLIENT_CHANNEL_HPP
