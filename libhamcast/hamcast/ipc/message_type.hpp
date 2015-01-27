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

#ifndef HAMCAST_IPC_MESSAGE_TYPE_HPP
#define HAMCAST_IPC_MESSAGE_TYPE_HPP

namespace hamcast { namespace ipc {

/**
 * @brief Describes the first header field of an IPC
 *        message and determines the interpretation of the other header fields.
 * @ingroup IPC
 *
 * For the general structure of an IPC message see @ref IPC_struct.
 */
enum message_type
{

    /**
     * @brief A synchronous request message.
     *
     * \n
     * \image html header_sync_request.png
     * \n
     *
     * This causes the middleware to invoke the requestet function
     * and respond with an <code>sync_response</code> message with either
     * the serialized result or an error message describing an occured
     * exception during invocation.
     */
    sync_request   = 0x00,

    /**
     * @brief A response to a synchronous request.
     *
     * \n
     * \image html header_sync_response.png
     * \n
     *
     * Belongs to the request message with equal <code>{request id}</code>
     * and contains either the serialized result value of the invoked function
     * or an error string describing an exception that occured during
     * invocation.
     */
    sync_response  = 0x01,

    /**
     * @brief Asynchronous transmission of an event.
     *
     * \n
     * \image html header_async_event.png
     * \n
     *
     * Asynchronous transmission of a memebership event.
     */
    async_event    = 0x02,

    /**
     * @brief Asynchronous transmission of an outgoing multicast packet.
     *
     * \n
     * \image html header_async_send.png
     * \n
     *
     * Send multicast data to the middleware with
     * outoing socket = <code>{socket id}</code>. The outgoing
     * group is identified by <code>{stream id}</code>.
     */
    async_send     = 0x03,

    /**
     * @brief Asynchronous transmission of an incoming multicast packet.
     *
     * \n
     * \image html header_async_recv.png
     * \n
     *
     * Transmission of a received multicast data packet for a joined group.
     * The client deserializes a {@link multicast_packet} object from
     * the <code>content</code> field.
     */
    async_recv     = 0x04,

    /**
     * @brief Indicates that the middleware has accepted outgoing multicast
     *        packets and will deliver them.
     * @note Picture is outdated, no longer contains {stream_id}.
     *
     * \n
     * \image html header_cumulative_ack.png
     * \n
     *
     * Forces the client to remove all outoing packages
     * with a smaller or equal <code>sequence nr</code> from its buffer.
     */
    cumulative_ack = 0x05,

    /**
     * @brief Indicates that the middleware has discarded multicast packets
     *        (e.g. because of a full buffer) and requests a retransmission.
     * @note Picture is outdated, no longer contains {stream_id}.
     *
     * \n
     * \image html header_retransmit.png
     * \n
     *
     * Forces the client to retransmit all packages in its buffer
     * starting with the package that has the given <code>{sequence nr}</code>.
     */
    retransmit     = 0x06

};

/**
 * @brief Check if @p what is a valid message type.
 * @param what An integer value interpreted as
 *             {@link hamcast::ipc::message_type message_type}.
 * @returns <code>true</code> if @p what has a valid value; otherwise false.
 * @note This function is used to verify, that a cast from an integer value
 *       to {@link hamcast::ipc::message_type message_type} results in a valid
 *       expression.
 */
bool valid(message_type what);

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_MESSAGE_TYPE_HPP
