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

#include <boost/thread.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/multicast_packet.hpp"
#include "hamcast/interface_property.hpp"

#include "hamcast/ipc/message.hpp"

#include "hamcast/util/serialization.hpp"

namespace {

const size_t max_cache_size = 50;

typedef std::map<std::string, hamcast::uri> uri_cache;
boost::thread_specific_ptr<uri_cache> m_uri_cache;

// check if @p uri_str is cached
// if @p uri_str is cached then set storage to the cached uri object
// otherwise put a new instance to the cache and set storage afterwards
void uri_lookup(const std::string& uri_str, hamcast::uri& storage)
{
    uri_cache* cache = m_uri_cache.get();
    if (!cache)
    {
        cache = new uri_cache;
        m_uri_cache.reset(cache);
    }
    uri_cache::const_iterator i(cache->find(uri_str));
    if (i != cache->end())
    {
        storage = i->second;
    }
    else
    {
        storage = hamcast::uri(uri_str);
        // purge the cache if full
        if (cache->size() >= max_cache_size)
        {
            cache->clear();
        }
        cache->insert(uri_cache::value_type(uri_str, storage));
    }
}

} // namespace <anonymous>

namespace hamcast { namespace util {

/******************************************************************************
 *                                  integers                                  *
 ******************************************************************************/

deserializer& operator>>(deserializer& d, bool& storage)
{
    boost::uint8_t tmp;
    d >> tmp;
    storage = (tmp != 0);
    return d;
}


/******************************************************************************
 *                               HAMcast types                                *
 ******************************************************************************/

serializer& operator<<(serializer& s, const uri& what)
{
    return (s << what.str());
}

deserializer& operator>>(deserializer& d, uri& storage)
{
    std::string str;
    d >> str;
    uri_lookup(str, storage);
    return d;
}

serializer& operator<<(serializer& s, const multicast_packet& mp)
{
    s << mp.from() << mp.size();
    if (mp.size() > 0)
    {
        s.write(mp.size(), mp.data());
    }
    return s;
}

deserializer& operator>>(deserializer& d, multicast_packet& mp)
{
    uri mp_from;
    boost::uint32_t mp_size;
    d >> mp_from >> mp_size;
    char* mp_data = 0;
    if (mp_size > 0)
    {
        mp_data = new char[mp_size];
        d.read(mp_size, mp_data);
    }
    multicast_packet tmp(mp_from, mp_size, mp_data);
    mp = tmp;
    return d;
}

serializer& operator<<(serializer& s, const interface_property& ip)
{
    return (s << ip.id << ip.name << ip.address << ip.technology);
}

deserializer& operator>>(deserializer& d, interface_property& ip)
{
    return (d >> ip.id >> ip.name >> ip.address >> ip.technology);
}

serializer& operator<<(serializer& s, const ipc::message& what)
{
    return ipc::message::serialize(s, what);
}

deserializer& operator>>(deserializer& d, ipc::message::ptr& mptr)
{
    return ipc::message::deserialize(d, mptr);
}

/******************************************************************************
 *                      standard template library types                       *
 ******************************************************************************/

serializer& operator<<(serializer& s, const std::string& what)
{
    s << static_cast<boost::uint32_t>(what.size());
    s.write(what.size(), what.c_str());
    return s;
}

deserializer& operator>>(deserializer& d, std::string& storage)
{
    boost::uint32_t str_size;
    d >> str_size;
    storage.reserve(str_size);
    // read string in 128 byte chunks
    char chunk[129];
    while (str_size > 0)
    {
        size_t rd_size = std::min<size_t>(128, str_size);
        d.read(rd_size, chunk);
        chunk[rd_size] = '\0';
        storage += chunk;
        str_size -= rd_size;
    }
    return d;
}

} } // namespace hamcast::util
