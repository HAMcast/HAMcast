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

#ifndef HAMCAST_UTIL_FUTURE_HPP
#define HAMCAST_UTIL_FUTURE_HPP

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace hamcast { namespace util {

/**
 * @brief Describes a simple synchronous future value.
 */
template<typename T>
class future
{

    T m_value;
    volatile bool m_is_set;

    boost::mutex m_mtx;
    boost::condition_variable m_cv;

 public:

    future() : m_value(), m_is_set(false) { }

    /**
     * @brief Read the produced value.
     * @returns A reference to the produced value.
     * @warning This function blocks until {@link set(const T&) set}
     *          was called.
     */
    T& get()
    {
        boost::mutex::scoped_lock guard(m_mtx);
        while (!m_is_set)
        {
            m_cv.wait(guard);
        }
        return m_value;
    }

    /**
     * @brief Set the value to @p val and wakeup waiting consumer thread.
     * @param val The produced value.
     */
    void set(const T& val)
    {
        boost::mutex::scoped_lock guard(m_mtx);
        m_is_set = true;
        m_value = val;
        m_cv.notify_one();
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_FUTURE_HPP
