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

#ifndef HAMCAST_INTRUSIVE_PTR_HPP
#define HAMCAST_INTRUSIVE_PTR_HPP

#include <cstddef>
#include <algorithm>
#include <stdexcept>

#include "hamcast/ref_counted.hpp"

namespace hamcast {

template<typename T>
struct default_add_ref
{
    inline void operator()(T* rc) { ref_counted::add_ref(rc); }
};

template<typename T>
struct default_release
{
    inline void operator()(T* rc) { ref_counted::release(rc); }
};

/**
 * @brief An intrusive, reference counting smart pointer impelementation.
 * @relates ref_counted
 */
template<typename T,
         typename AddRef = default_add_ref<T>,
         typename Release = default_release<T> >
class intrusive_ptr
{

    T* m_ptr;
    AddRef m_add_ref;
    Release m_release;

    inline void set_ptr(T* raw_ptr) {
        m_ptr = raw_ptr;
        m_add_ref(raw_ptr);
    }

 public:

    inline intrusive_ptr() : m_ptr(0) { }

    intrusive_ptr(T* raw_ptr) { set_ptr(raw_ptr); }

    intrusive_ptr(const intrusive_ptr& other) { set_ptr(other.m_ptr); }

    ~intrusive_ptr()
    {
        m_release(m_ptr);
    }

    inline T* get() { return m_ptr; }

    inline const T* get() const { return m_ptr; }

    T* take()
    {
        T* result = m_ptr;
        m_ptr = 0;
        return result;
    }

    inline void swap(intrusive_ptr& other)
    {
        std::swap(m_ptr, other.m_ptr);
    }

    void reset(T* new_value = 0)
    {
        m_release(m_ptr);
        set_ptr(new_value);
    }

    intrusive_ptr& operator=(T* ptr)
    {
        reset(ptr);
        return *this;
    }

    intrusive_ptr& operator=(const intrusive_ptr& other)
    {
        intrusive_ptr tmp(other);
        swap(tmp);
        return *this;
    }

    inline T& operator*() { return *m_ptr; }

    inline T* operator->() { return m_ptr; }

    inline const T& operator*() const { return *m_ptr; }

    inline const T* operator->() const { return m_ptr; }

    inline operator bool() const { return m_ptr != 0; }

};

template<typename X, typename AddRef, typename Release>
inline bool operator==(const intrusive_ptr<X, AddRef, Release>& lhs, const X* rhs)
{
    return lhs.get() == rhs;
}

template<typename X, typename AddRef, typename Release>
inline bool operator==(const X* lhs, const intrusive_ptr<X, AddRef, Release>& rhs)
{
    return rhs.get() == lhs;
}

template<typename X, typename AddRef, typename Release>
bool operator==(const intrusive_ptr<X, AddRef, Release>& lhs, const intrusive_ptr<X, AddRef, Release>& rhs) {
    return lhs.get() == rhs.get();
}

template<typename X, typename AddRef, typename Release>
inline bool operator!=(const intrusive_ptr<X, AddRef, Release>& lhs, const X* rhs)
{
    return !(lhs == rhs);
}

template<typename X, typename AddRef, typename Release>
inline bool operator!=(const X* lhs, const intrusive_ptr<X, AddRef, Release>& rhs)
{
    return !(lhs == rhs);
}

template<typename X, typename AddRef, typename Release>
inline bool operator!=(const intrusive_ptr<X, AddRef, Release>& lhs, const intrusive_ptr<X, AddRef, Release>& rhs)
{
    return !(lhs == rhs);
}

} // namespace hamcast

#endif // HAMCAST_INTRUSIVE_PTR_HPP
