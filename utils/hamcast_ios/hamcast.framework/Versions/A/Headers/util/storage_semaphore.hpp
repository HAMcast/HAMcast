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

#ifndef HAMCAST_UTIL_STORAGE_SEMAPHORE_HPP
#define HAMCAST_UTIL_STORAGE_SEMAPHORE_HPP

#include <boost/cstdint.hpp>
#include <boost/thread.hpp>

#include "hamcast/util/atomic_operations.hpp"

namespace hamcast { namespace util {

enum storage_semaphore_t
{
    blocking_t,
    nonblocking_t
};

template<boost::int32_t max_size, storage_semaphore_t sem_type = blocking_t>
class storage_semaphore
{

    volatile boost::int32_t m_available;

    mutable boost::mutex m_mtx;
    mutable boost::condition_variable m_cv;

    typedef boost::unique_lock<boost::mutex> lock_type;

    void release_impl(boost::int32_t num_bytes, lock_type* guard = 0)
    {
        for (;;)
        {
            boost::int32_t av = m_available;
            boost::int32_t av_new = av + num_bytes;
            if (!guard && ((av < 0) && (av_new >= 0)))
            {
                // lock is required if we could wake up a waiting thread
                lock_type ulock(m_mtx);
                release_impl(num_bytes, &ulock);
                return;
            }
            else if (atomic_cas(&m_available, av, av_new))
            {
                if (guard) m_cv.notify_all();
                return;
            }
        }
    }

    void acquire_impl(boost::int32_t num_bytes, lock_type* guard = 0)
    {
        bool av_decreased = false;
        bool av_negative = false;
        do
        {
            boost::int32_t av = m_available;
            boost::int32_t av_new = av - num_bytes;
            // lock mutex if needed
            if (!guard && ((av_new < 0) || (av == max_size)))
            {
                lock_type ulock(m_mtx);
                acquire_impl(num_bytes, &ulock);
                return;
            }
            else if (atomic_cas(&m_available, av, av_new))
            {
                av_decreased = true;
                if (av_new < 0) av_negative = true;
                else if (av == max_size)
                {
                    // there's at most one thread waiting
                    // m_mtx is locked by guard in this case
                    m_cv.notify_one();
                }
            }
        }
        while (!av_decreased);
        // we set available to a negative value
        // wait until enough bytes become available again
        if (av_negative)
        {
            assert(guard != 0);
            // guard is locked
            while (m_available < 0)
            {
                m_cv.wait(*guard);
            }
        }
    }

 public:

    /**
     * @post full() == true
     */
    storage_semaphore() : m_available(max_size) { }

    /**
     * @post full() == false
     */
    void wait_for_not_full() const
    {
        if (m_available == max_size)
        {
            boost::unique_lock<boost::mutex> guard(m_mtx);
            while (m_available == max_size)
            {
                m_cv.wait(guard);
            }
        }
    }

    /**
     * @post on success: full() == false
     */
    bool wait_for_not_full(boost::uint16_t ms_timeout) const
    {
        if (m_available == max_size)
        {
            boost::system_time timeout = boost::get_system_time();
            timeout += boost::posix_time::milliseconds(ms_timeout);
            boost::unique_lock<boost::mutex> guard(m_mtx);
            while (m_available == max_size)
            {
                if (!m_cv.timed_wait(guard, timeout)) return false;
            }
        }
        return true;
    }

    inline boost::int32_t available() const { return m_available; }

    void acquire(boost::int32_t num_bytes)
    {
        acquire_impl(num_bytes);
    }

    void release(boost::int32_t num_bytes)
    {
        release_impl(num_bytes);
    }

    inline boost::int32_t maximum() const { return max_size; }

    inline boost::int32_t acquired() const { return maximum() - available(); }

};

template<boost::int32_t max_size>
class storage_semaphore<max_size, nonblocking_t>
{

    volatile boost::int32_t m_available;

 public:

    storage_semaphore() : m_available(max_size) { }

    bool try_acquire(boost::int32_t num_bytes)
    {
        boost::int32_t av = m_available;
        boost::int32_t av_new = av - num_bytes;
        for (;;)
        {
            if (av_new < 0) return false;
            else if (atomic_cas(&m_available, av, av_new))
            {
                return true;
            }
            // next iteration
            av = m_available;
            av_new = av - num_bytes;
        }
    }

    void release(boost::int32_t num_bytes)
    {
        add_and_fetch(&m_available, num_bytes);
        /*
        for (;;)
        {
            boost::int32_t av = m_available;
            boost::int32_t av_new = av + num_bytes;
            if (atomic_cas(&m_available, av, av_new))
            {
                return;
            }
        }
        */
    }

    inline boost::int32_t available() const { return m_available; }

    inline boost::int32_t maximum() const { return max_size; }

    inline boost::int32_t acquired() const { return maximum() - available(); }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_STORAGE_SEMAPHORE_HPP
