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

#include "hamcast/detail/client_channel_msg.hpp"

namespace hamcast { namespace detail {

outgoing_mcast_packet::~outgoing_mcast_packet()
{
    if (content) delete[] content;
}

void* outgoing_mcast_packet::addr()
{
    return this;
}

void* outgoing_synchronous_message::addr()
{
    return this;
}

void* reg_d_ptr::addr()
{
    return this;
}

void* register_callback_msg::addr()
{
    return this;
}

void* register_recv_cb_msg::addr()
{
    return this;
}

void* incoming_mcast_packet::addr()
{
    return this;
}

} } // namespace cppa::detail
