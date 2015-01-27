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

#ifndef HAMCAST_IPC_HPP
#define HAMCAST_IPC_HPP

#include "hamcast/ipc/api.hpp"
#include "hamcast/ipc/channel.hpp"
#include "hamcast/ipc/client_channel.hpp"
#include "hamcast/ipc/exception_id.hpp"
#include "hamcast/ipc/function_id.hpp"
#include "hamcast/ipc/message.hpp"
#include "hamcast/ipc/message_type.hpp"
#include "hamcast/ipc/middleware_configuration.hpp"
#include "hamcast/ipc/sync_function.hpp"
#include "hamcast/interface_property.hpp"

/**
 * @defgroup IPC Inter-process communication.
 * @brief {@link hamcast::multicast_socket HAMcast sockets} don't access
 *        any network interface. They serve as stub interface only.
 *        All member function calls are forwarded to the HAMcast middleware.
 *        This section describes the (binary) IPC protocol.
 *
 * @section IPC_protocol_init Connection establishment
 *
 * \n
 * \image html ipc_init.png
 * \n
 *
 * @section IPC_struct Structure of an IPC message
 *
 * \n
 * \image html header_general.png
 * \n
 *
 * @section IPC_seq_diagram Example protocol sequence diagrams
 *
 * To send asynchronous multicast packages, you first need to create
 * a socket and create a send stream on that socket with synchronous
 * request messages.
 *
 * This example shows a sequence, where a client sends seven multicast
 * message to the group <code>"ip://239.0.0.1"</code>. Note, that the
 * client has to cache each send message, until he received an ACK
 * message. The middleware discards messages, if the internal send
 * buffer is full. In this case, the middleware sends a <code>retransmit</code>
 * message with the first discarded sequence number.
 *
 * The middleware expects serially numbered messages starting with 0 and
 * <code>"out-of-order"</code> are ignored.
 *
 * \n
 * \image html ipc_sequence.png
 * \n
 *
 * <code>create_socket()</code> is a
 * {@link hamcast::ipc::sync_request synchronous request message} with
 * <code>function_id =
 * {@link hamcast::ipc::fid_create_socket fid_create_socket}</code> and
 * an empty content field (<code>cs = 0</code>). The middleware then
 * sends a {@link hamcast::ipc::sync_response synchronous response message}
 * with a serialized {@link hamcast::socket_id socket_id} in the content
 * field:
 *
 * @include example_protocol_sequence_pseudocode.txt
 *
 *
 *
 */

namespace hamcast { namespace ipc {

// Backwards compatibility.
// Until hamcast 0.2 interface_property was part of the namespace ipc.
using ::hamcast::interface_property;

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_HPP
