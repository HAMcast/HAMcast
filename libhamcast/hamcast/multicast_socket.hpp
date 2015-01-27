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

#ifndef HAMCAST_MULTICAST_SOCKET_HPP
#define HAMCAST_MULTICAST_SOCKET_HPP

#include <vector>
#include <boost/noncopyable.hpp>

#include "hamcast/config.hpp"
#include "hamcast/socket_id.hpp"
#include "hamcast/ipc/message.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/multicast_packet.hpp"
#include "hamcast/util/storage_semaphore.hpp"

// forward declarations
namespace hamcast {
class uri;
namespace ipc { class channel; class client_channel; }
namespace detail { struct multicast_socket_private; }
} // namespace hamcast

namespace hamcast {

/**
 * @brief Describes a hamcast multicast socket.
 * @warning This class is not thread safe.
 */
class multicast_socket : boost::noncopyable
{

 public:

    struct add_ref { void operator()(detail::multicast_socket_private*); };
    struct release { void operator()(detail::multicast_socket_private*); };

    typedef intrusive_ptr<detail::multicast_socket_private, add_ref, release> d_ptr;

    /**
     * @brief Create a new HAMcast socket.
     */
    multicast_socket();

    /**
     * @brief Releases all ressources and leaves all joined groups.
     */
    ~multicast_socket();

    /**
     * @brief Get the ID of this socket.
     * @returns The socket ID of this object.
     */
    socket_id id() const;

    /**
     * @brief Send a multicast message to @p group.
     * @param group Multicast group of the receivers
     * @param msg_len Length of @p buf in bytes.
     * @param buf Bytes to send.
     * @throw requirement_failed If
     *        <code>@p group.{@link hamcast::uri::empty() empty()}</code> or
     *        <code>@p msg_len == 0</code> or <code>@p buf == NULL</code>.
     */
    void send(const uri& group, size_t msg_len, const void* buf);

    /**
     * @brief Receive a multicast message (packet) from this socket.
     * @warning This is a blocking call.
     * @returns The received multicast packet.
     */
    multicast_packet receive();

    /**
     * @brief Try to receive a multicast message from this socket.
     * @param storage A {@link hamcast::multicast_packet multicast_packet}
     *                object that should contain the received data on success.
     * @returns <code>true</code> if a multicast packet was received;
     *         otherwise <code>false</code>.
     */
    bool try_receive(multicast_packet& storage);

    /**
     * @brief Try to receive a multicast message from this socket within
     *        the given timeout.
     * @param storage A {@link hamcast::multicast_packet multicast_packet}
     *                object that should contain the received data on success.
     * @param milliseconds The maximum time (in milliseconds) this member
     *                     function should block.
     * @returns <code>true</code> if a multicast packet was received;
     *         otherwise <code>false</code> (timeout occured).
     */
    bool try_receive(multicast_packet& storage, boost::uint16_t milliseconds);

    /**
     * @brief Join a multicast group.
     * @param group multicast group that you want to join
     */
    void join(const uri& group);

    /**
     * @brief Leave a multicast group.
     * @param group multicast group that you want to leave
     */
    void leave(const uri& group);

    /**
     * @brief Change the TTL value used to send data.
     *
     * The default value is 255.
     * @param value The new maximum hop count (TTL).
     */
    void set_ttl(boost::uint32_t value);

    /**
     * @brief Get all associated interfaces.
     * @returns A vector of all known interface IDs.
     */
    std::vector<interface_id> interfaces();

    /**
     * @brief Add @p iface to the distribution channel of the socket.
     * @param iface A hamcast multicast interface.
     */
    void add_interface(interface_id iface);

    /**
     * @brief Removes @p iface from the distribution channel of the socket.
     * @param iface A hamcast multicast interface.
     */
    void del_interface(interface_id iface);

    /**
     * @brief Set associated interfaces.
     * @param ifs The hamcast multicast interfaces.
     */
    void set_interfaces(const std::vector<interface_id>& ifs);

    /**
     * @brief Set associated interface.
     * @param iid The hamcast multicast interface that should be used
     *            exclusively.
     */
    void set_interface(interface_id iid);

    static void async_recv(d_ptr& sock_private, const multicast_packet& mp);

    static void release_storage(d_ptr& sock_private, boost::int32_t bytes);

 private:

    d_ptr d;

};

} // namespace hamcast

#endif // HAMCAST_MULTICAST_SOCKET_HPP
