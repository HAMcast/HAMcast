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

#ifndef HAMCAST_UTIL_ATOMIC_OPERATIONS_HPP
#define HAMCAST_UTIL_ATOMIC_OPERATIONS_HPP

#include "hamcast/config.hpp"

namespace hamcast { namespace util {

/**
 * @brief Atomically compare the content of @p ptr to @p ptr_expected and,
 *        if they are the same, modify it to @p ptr_new.
 *
 * Platform dependent, atomic compare-and-swap operation.
 *
 * @param ptr Memory location of the variable
 * @param ptr_expected The value you expect @p *ptr to be
 * @param ptr_new The value which should be stored in @p *ptr
 *
 * @returns true if @p ptr was set to @p ptr_new; otherwise false
 */
template<typename T>
inline bool atomic_cas(volatile T** ptr, T* ptr_expected, T* ptr_new);

inline bool atomic_cas(volatile boost::int32_t* ptr,
                       boost::int32_t expected_value,
                       boost::int32_t new_value);

/**
 * @brief Atomically increase the content of @p ptr by @p value and return
 *        the new content of @p ptr.
 * @param ptr Memory location of the variable.
 * @param value The value you want to add to @p ptr.
 * @returns The new content of @p ptr.
 */
inline boost::int32_t add_and_fetch(volatile boost::int32_t* ptr,
                                    boost::int32_t value);

inline boost::uint32_t add_and_fetch(volatile boost::uint32_t *ptr,
                                     boost::uint32_t value)
{
    return static_cast<boost::int32_t>(
                add_and_fetch(
                    reinterpret_cast<volatile boost::int32_t*>(ptr),
                    static_cast<boost::int32_t>(value)));
}

} } // namespace hamcast::util

#if defined(HAMCAST_MACOS)

#include <libkern/OSAtomic.h>

#define HC_MEMORY_BARRIER() OSMemoryBarrier()

namespace hamcast { namespace util {

template<typename T>
inline bool atomic_cas(volatile T** ptr, T* ptr_expected, T* ptr_new)
{
    return OSAtomicCompareAndSwapPtr((void*) ptr_expected,
                                     (void*) ptr_new,
                                     (void* volatile*) ptr);
}

inline bool atomic_cas(volatile boost::int32_t* ptr,
                       boost::int32_t expected_value,
                       boost::int32_t new_value)
{
    return OSAtomicCompareAndSwap32(expected_value, new_value, ptr);
}

inline boost::int32_t add_and_fetch(volatile boost::int32_t* ptr,
                                    boost::int32_t value)
{
    return OSAtomicAdd32(value, ptr);
}

} } // namespace hamcast::util

#elif defined(HAMCAST_LINUX)

#define HC_MEMORY_BARRIER() __sync_synchronize()

namespace hamcast { namespace util {

template<typename T>
inline bool atomic_cas(volatile T** ptr, T* ptr_expected, T* ptr_new)
{
    return __sync_bool_compare_and_swap(ptr, ptr_expected, ptr_new);
}

inline bool atomic_cas(volatile boost::int32_t* ptr,
                       boost::int32_t expected_value,
                       boost::int32_t new_value)
{
    return __sync_bool_compare_and_swap(ptr, expected_value, new_value);
}

inline boost::int32_t add_and_fetch(volatile boost::int32_t* ptr,
                                    boost::int32_t value)
{
    return __sync_add_and_fetch(ptr, value);
}

} }

#else
#error "No CAS for this plattform"
#endif // if defined(HAMCAST_MACOS)

#endif // HAMCAST_UTIL_ATOMIC_OPERATIONS_HPP
