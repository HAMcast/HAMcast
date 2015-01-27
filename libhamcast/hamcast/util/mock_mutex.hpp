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

#ifndef HAMCAST_UTIL_MOCK_MUTEX_HPP
#define HAMCAST_UTIL_MOCK_MUTEX_HPP

#include <boost/thread/thread_time.hpp>

namespace hamcast { namespace util {

/**
 * @brief Implements the Lockable concepts without any
 *        behavior. To use this "mutex" means to disable thread safety.
 */
struct mock_mutex
{
    inline void lock() { }
    inline bool try_lock() { return true; }
    inline void unlock() { }
    inline void lock_shared() { }
    inline bool try_lock_shared() { return true; }
    inline bool timed_lock_shared(const boost::system_time&) { return true; }
    inline void unlock_shared() { }
    inline bool timed_lock(const boost::system_time&) { return true;}
    inline void lock_upgrade() { }
    inline void unlock_upgrade() { }
    inline void unlock_upgrade_and_lock() { }
    inline void unlock_and_lock_upgrade() { }
    inline void unlock_and_lock_shared() { }
    inline void unlock_upgrade_and_lock_shared() { }
};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_MOCK_MUTEX_HPP
