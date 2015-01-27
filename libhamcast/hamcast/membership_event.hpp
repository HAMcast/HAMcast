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

#ifndef MEMBERSHIP_EVENT_HPP
#define MEMBERSHIP_EVENT_HPP

#include <boost/function.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/membership_event_type.hpp"

namespace hamcast {

class membership_event
{

    uri m_group;
    interface_id m_iid;
    membership_event_type m_type;

    // prohibit assignment
    membership_event& operator=(const membership_event&);

 public:

    membership_event();

    membership_event(const uri& group_name,
                     interface_id iid,
                     membership_event_type event_type);

    membership_event(const membership_event& other);

    /**
     * @brief Get the related group name.
     */
    inline const uri& group() const { return m_group; }

    /**
     * @brief Get the event type.
     */
    inline membership_event_type type() const { return m_type; }

    /**
     * @brief Get the related interface ID.
     */
    inline interface_id iface_id() const { return m_iid; }

    /**
     * @brief Check if this event has a valid event type.
     */
    inline bool valid() const { return m_type != invalid_event; }

};

typedef boost::function<void (const membership_event&)>
        membership_event_callback;

/**
 * @brief Register @p cb.
 * @param cb Membership event callback.
 */
void register_event_callback(const membership_event_callback& cb);

} // namespace hamcast

#endif // MEMBERSHIP_EVENT_HPP
