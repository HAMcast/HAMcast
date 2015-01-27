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

#ifndef HAMCAST_IPC_FUNCTION_ID_HPP
#define HAMCAST_IPC_FUNCTION_ID_HPP

namespace hamcast { namespace ipc {

/**
 * @brief Holds the function ID of an IPC request.
 * @ingroup IPC
 */
enum function_id
{

    // socket operations

    /**
     * @brief Create a new multicast socket.
     *
     * The result of this function is a {@link hamcast::socket_id socket_id}.
     *
     */
    fid_create_socket                    = 0x0001,

    /**
     * @brief Delete a multicast socket.
     *
     * This function has no result.
     */
    fid_delete_socket                    = 0x0002,

    /**
     * @brief Create a new (outgoing) stream on a multicast socket.
     *
     * The result of this function is a {@link hamcast::socket_id socket_id}.
     */
    fid_create_send_stream               = 0x0003,
    fid_join                             = 0x0004,
    fid_leave                            = 0x0005,
    fid_set_ttl                          = 0x0006,
    fid_get_sock_interfaces              = 0x0007,
    fid_add_sock_interface               = 0x0008,
    fid_del_sock_interface               = 0x0009,
    fid_set_sock_interfaces              = 0x000A,

    // service calls
    fid_get_interfaces                   = 0x0100,
    fid_group_set                        = 0x0101,
    fid_neighbor_set                     = 0x0102,
    fid_children_set                     = 0x0103,
    fid_parent_set                       = 0x0104,
    fid_designated_host                  = 0x0105,

    // others
    fid_enable_events                    = 0x0200,
    fid_disable_events                   = 0x0201,
    fid_get_atomic_msg_size              = 0x0202,

    // "private API parts"
    fid_set_is_img_flag                  = 0xF001,
    fid_get_is_img_flag                  = 0xF002
};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_FUNCTION_ID_HPP
