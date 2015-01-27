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

#include "hamcast/membership_event.hpp"

#include "hamcast/ipc/api.hpp"
#include "hamcast/ipc/client_channel.hpp"

namespace hamcast {

membership_event::membership_event(const uri& grp,
                                   interface_id id,
                                   membership_event_type tp)
    : m_group(grp), m_iid(id), m_type(tp)
{
}

membership_event::membership_event()
    : m_group(), m_iid(0), m_type(invalid_event)
{
}

membership_event::membership_event(const membership_event& other)
    : m_group(other.m_group), m_iid(other.m_iid), m_type(other.m_type)
{
}

void register_event_callback(const membership_event_callback& cb)
{
    ipc::enable_events();
    ipc::client_channel::get()->register_callback(cb);
}

} // namespace hamcast
