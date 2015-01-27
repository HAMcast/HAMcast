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

#ifndef HAMCAST_UTIL_URI_MAPPINGS_HPP
#define HAMCAST_UTIL_URI_MAPPINGS_HPP

#include <map>
#include <boost/thread/locks.hpp>
#include <boost/noncopyable.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/exception.hpp"
#include "hamcast/util/mock_mutex.hpp"

namespace hamcast { namespace util {

template<class MutexType = mock_mutex>
class uri_mappings : boost::noncopyable
{

    typedef std::map<uri,uri> map_t;
    typedef MutexType mutex_t;
    typedef boost::shared_lock<mutex_t> shared_lock_t;
    typedef boost::lock_guard<mutex_t> exclusive_lock_t;
    typedef boost::upgrade_lock<mutex_t> upgrade_lock_t;
    typedef boost::upgrade_to_unique_lock<mutex_t> upgrade_to_unique_lock_t;

    mutable mutex_t m_mtx;

    map_t m_forward_map;
    map_t m_reverse_map;

    uri lookup(const map_t& where, const uri& what) const
    {
        shared_lock_t guard(m_mtx);
        map_t::const_iterator i(where.find(what));
        if (i != where.end())
        {
            return i->second;
        }
        return uri();
    }

    void erase(const uri& what, map_t& first, map_t& second)
    {
        exclusive_lock_t guard(m_mtx);
        map_t::iterator i(first.find(what));
        if (i == first.end()) return;
        second.erase(i->second);
        first.erase(i);
    }

 public:

    inline uri forward_lookup(const uri& what) const
    {
        return lookup(m_forward_map, what);
    }

    template<typename MappingFunctor>
    uri forward_lookup(const uri &what, MappingFunctor& mf)
    {
        upgrade_lock_t guard(m_mtx);
        map_t::const_iterator i(m_forward_map.find(what));
        if (i != m_forward_map.end())
        {
            return i->second;
        }
        else
        {
            upgrade_to_unique_lock_t super_guard(guard);
            // try again to find what
            map_t::const_iterator j(m_forward_map.find(what));
            if (j != m_forward_map.end())
            {
                // someone else was faster
                return j->second;
            }
            else
            {
                // it's up to us
                uri mapped = mf(what);
                HC_REQUIRE(!mapped.empty());
                HC_REQUIRE(m_reverse_map.count(mapped) == 0);
                m_forward_map.insert(map_t::value_type(what, mapped));
                m_reverse_map.insert(map_t::value_type(mapped, what));
                return mapped;
            }
        }
    }

    bool empty() const
    {
        shared_lock_t guard(m_mtx);
        return m_forward_map.empty();
    }

    void clear()
    {
        exclusive_lock_t guard(m_mtx);
        m_forward_map.clear();
        m_reverse_map.clear();
    }

    inline void forward_erase(const uri& what)
    {
        erase(what, m_forward_map, m_reverse_map);
    }

    inline uri reverse_lookup(const uri& what) const
    {
        return lookup(m_reverse_map, what);
    }

    inline void reverse_erase(const uri& what)
    {
        erase(what, m_reverse_map, m_forward_map);
    }

    void insert(const uri& original, const uri& mapped)
    {
        HC_REQUIRE(!original.empty() && !mapped.empty())
        exclusive_lock_t guard(m_mtx);
        HC_REQUIRE(m_forward_map.count(original) == 0);
        HC_REQUIRE(m_reverse_map.count(mapped) == 0);
        m_forward_map.insert(map_t::value_type(original, mapped));
        m_reverse_map.insert(map_t::value_type(mapped, original));
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_URI_MAPPINGS_HPP
