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

#include <limits>

#include "rw_spinlock.hpp"
#include "boost/thread.hpp"

namespace {

const boost::int32_t min_32 = std::numeric_limits<boost::int32_t>::min();

} // namespace <anonymous>

namespace hamcast { namespace middleware {

rw_spinlock::rw_spinlock() : m_flag(0)
{
}

void rw_spinlock::lock()
{
    boost::int32_t v = m_flag;
    for (;;)
    {
        if (v != 0)
        {
            boost::this_thread::yield();
            v = m_flag; // next iteration
        }
        else
        {
            if (util::atomic_cas(&m_flag, 0, min_32))
            {
                HC_MEMORY_BARRIER();
                return;
            }
            else
            {
                v = m_flag; // next iteration
            }
        }
    }
}

void rw_spinlock::unlock()
{
    for (;;)
    {
        if (util::atomic_cas(&m_flag, min_32, 0))
        {
            HC_MEMORY_BARRIER();
            return;
        }
    }
}

bool rw_spinlock::try_lock()
{
    return util::atomic_cas(&m_flag, 0, min_32);
}

void rw_spinlock::lock_shared()
{
    boost::int32_t v = m_flag;
    for (;;)
    {
        if (v < 0)
        {
            boost::this_thread::yield();
            v = m_flag;
        }
        else
        {
            if (util::atomic_cas(&m_flag, v, v + 1))
            {
                HC_MEMORY_BARRIER();
                return;
            }
            else
            {
                v = m_flag; // next iteration
            }
        }
    }
}

// todo: use atomic decrement operation instead
void rw_spinlock::unlock_shared()
{
    boost::int32_t v = m_flag;
    for (;;)
    {
        if (util::atomic_cas(&m_flag, v, v - 1))
        {
            HC_MEMORY_BARRIER();
            return;
        }
        else
        {
            v = m_flag; // next iteration
        }
    }
}

bool rw_spinlock::try_lock_shared()
{
    boost::int32_t v = m_flag;
    return (v >= 0) ? util::atomic_cas(&m_flag, v, v + 1) : false;
}

} } // namespace hamcast::middleware
