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

#ifndef HAMCAST_REFERENCE_COUNTED_HPP
#define HAMCAST_REFERENCE_COUNTED_HPP

#include <boost/noncopyable.hpp>
#include <boost/detail/atomic_count.hpp>

namespace hamcast {

/**
 * @brief This class implements intrusive reference counting to use
 *        derived classes in a <code>boost::intrusive_ptr</code>.
 */
class ref_counted : boost::noncopyable
{

 public:

    /**
     * @brief Get the current reference count.
     * @returns The current reference count as long.
     */
    inline long ref_count() { return m_count; }

    virtual ~ref_counted();

    /**
     * @brief Convenience function that increments the reference count of
     *        @p rc.
     *
     * Equal to <code>if (rc) rc->ref();</code>
     * @param rc A native pointer to a reference counted object.
     */
    inline static void add_ref(ref_counted* rc)
    {
        if (rc) rc->ref();
    }

    /**
     * @brief Convenience function that decrements the reference count of
     *        @p rc and deletes @p rc if needed.
     *
     * Equal to <code>if (rc && !rc->deref()) delete rc;</code>
     * @param rc A native pointer to a reference counted object.
     */
    inline static void release(ref_counted* rc)
    {
        if (rc && !rc->deref()) delete rc;
    }

 protected:

    /**
     * @brief Initializes the reference count with 0.
     */
    inline ref_counted() : m_count(0) { }

    /**
     * @brief Atomically increment the reference count.
     */
    inline void ref() { ++m_count; }

    /**
     * @brief Atomically decrement the reference count.
     * @returns <b>true</b> if there are still references to this object;
     *         otherwise <b>false</b>.
     */
    inline bool deref() { return 0 < --m_count; }

 private:

    boost::detail::atomic_count m_count;

};

} // namespace hamcast

#endif // HAMCAST_REFERENCE_COUNTED_HPP
