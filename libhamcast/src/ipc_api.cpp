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

#include "hamcast/ipc/api.hpp"

namespace hamcast { namespace ipc {

set_is_img_flag_t set_is_img_flag;
get_is_img_flag_t get_is_img_flag;

create_socket_t create_socket;
delete_socket_t delete_socket;
create_send_stream_t create_send_stream;
join_t join;
leave_t leave;
set_ttl_t set_ttl;
get_sock_interfaces_t get_sock_interfaces;
add_sock_interface_t add_sock_interface;
del_sock_interface_t del_sock_interface;
set_sock_interfaces_t set_sock_interfaces;
get_interfaces_t get_interfaces;
group_set_t group_set;
neighbor_set_t neighbor_set;
children_set_t children_set;
parent_set_t parent_set;
designated_host_t designated_host;
enable_events_t enable_events;
disable_events_t disable_events;
get_atomic_msg_size_t get_atomic_msg_size;

} } // namespace hamcast::ipc
