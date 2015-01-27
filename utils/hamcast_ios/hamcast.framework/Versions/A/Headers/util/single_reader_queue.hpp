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

#ifndef HAMCAST_UTIL_SINGLE_READER_QUEUE_HPP
#define HAMCAST_UTIL_SINGLE_READER_QUEUE_HPP

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/condition_variable.hpp>

#include "hamcast/config.hpp"
#include "hamcast/util/atomic_operations.hpp"

namespace hamcast { namespace util {

/**
 * @brief A thread safe single-reader-many-writer queue implementation.
 *
 * @param T The element type of the queue; must provide
 *          a <code>next</code> pointer.
 */
template<typename T>
class single_reader_queue
{

    typedef boost::unique_lock<boost::mutex> lock_type;

    void wait_for_data()
    {
        if (!m_cache && !m_stack)
        {
            lock_type guard(m_mtx);
            while (!m_stack) m_cv.wait(guard);
        }
    }

    template<typename Duration>
    bool wait_for_data(Duration max_wait_duration)
    {
        if (!m_cache && !m_stack)
        {
            boost::system_time timeout = boost::get_system_time();
            timeout += max_wait_duration;
            // lifetime scope of guard
            {
                lock_type guard(m_mtx);
                while (!m_stack)
                {
                    if (!m_cv.timed_wait(guard, timeout)) return false;
                }
            }
        }
        return true;
    }

 public:

    typedef T element_type;

    /**
     * @brief Remove the next element in the queue and return it.
     * @note This function blocks on an empty queue.
     * @returns The removed (previously the first) element.
     */
    element_type* pop()
    {
        wait_for_data();
        element_type* result = take_head();
        return result;
    }

    /**
     * @brief Equal to {@link pop()} but returns NULL if the queue is empty.
     * @returns NULL if the queue was empty; otherwise the removed element.
     */
    element_type* try_pop()
    {
        return take_head();
    }

    /**
     * @brief Equal to {@link pop()} but blocks at most @p max_wait_duration
     *        and returns NULL on a timeout.
     * @param max_wait_duration The maximum wait duration as a
     *                          boost posix time.
     * @returns NULL on a timeout; otherwise the removed element.
     */
    template<typename Duration>
    element_type* try_pop(Duration max_wait_duration)
    {
        return wait_for_data(max_wait_duration) ? take_head() : 0;
    }

    /**
     * @brief Return a pointer to the next element in the queue.
     * @returns NULL if the is empty; otherwise a pointer to the next element.
     */
    element_type* front()
    {
        return (m_cache || take_public_tail()) ? m_cache : 0;
    }

    // non-signaling version;
    void unsafe_push(element_type* head, element_type* tail)
    {
        for (;;)
        {
            element_type* e = const_cast<element_type*>(m_stack);
            tail->next = e;
            if (atomic_cas(&m_stack, e, head))
            {
                return;
            }
        }
    }

    void unsafe_push(element_type* new_element)
    {
        unsafe_push(new_element, new_element);
    }

    /**
     * @brief Add the (singly) linked list [@p head, @p tail]
     *        at the end of the queue.
     * @param head The first new element.
     * @param tail The last new element.
     * @warning The list [@p head, @p tail] should have LIFO order, because
     *          it will be read from tail to head.
     */
    void push(element_type* head, element_type* tail)
    {
        for (;;)
        {
            element_type* e = const_cast<element_type*>(m_stack);
            tail->next = e;
            if (!e)
            {
                lock_type guard(m_mtx);
                if (atomic_cas(&m_stack, (element_type*) 0, head))
                {
                    m_cv.notify_one();
                    return;
                }
            }
            else
            {
                if (atomic_cas(&m_stack, e, head))
                {
                    return;
                }
            }
        }
    }

    /**
     * @brief Add @p new_element at the end of the queue.
     * @param new_element The element that should become the new tail of
     *                    the queue.
     */
    void push(element_type* new_element)
    {
        push(new_element, new_element);
    }

    single_reader_queue() : m_stack(0), m_cache(0) { }

 private:

    // exposed to "outside" access
    volatile element_type* m_stack;

    // accessed only by the owner
    element_type* m_cache;

    // locked on enqueue/dequeue operations to/from an empty list
    boost::mutex m_mtx;
    boost::condition_variable m_cv;

    // atomically set public_tail to nullptr and enqueue all
    bool take_public_tail()
    {
        element_type* e = const_cast<element_type*>(m_stack);
        while (e)
        {
            if (atomic_cas(&m_stack, e, (element_type*) 0))
            {
                // public_tail (e) has LIFO order,
                // but private_head requires FIFO order
                while (e)
                {
                    // next iteration element
                    element_type* next = e->next;
                    // enqueue e to private_head
                    e->next = m_cache;
                    m_cache = e;
                    // next iteration
                    e = next;
                }
                return true;
            }
            // next iteration
            e = const_cast<element_type*>(m_stack);
        }
        // !public_tail
        return false;
    }

    element_type* take_head()
    {
        if (m_cache || take_public_tail())
        {
            element_type* result = m_cache;
            m_cache = result->next;
            return result;
        }
        return 0;
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_SINGLE_READER_QUEUE_HPP
