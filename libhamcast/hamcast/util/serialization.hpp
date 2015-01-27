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

#ifndef HAMCAST_UTIL_SERIALIZATION_HPP
#define HAMCAST_UTIL_SERIALIZATION_HPP

#include <map>
#include <list>
#include <vector>
#include <string>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>

#include "hamcast/util/unit.hpp"
#include "hamcast/util/serializer.hpp"
#include "hamcast/util/deserializer.hpp"

#include "hamcast/ipc/message.hpp"

// forward declarations
namespace hamcast {
class uri;
class multicast_packet;
struct interface_property;
}

//namespace hamcast { namespace ipc { class message; } }

namespace hamcast { namespace util {

/******************************************************************************
 *                                    unit                                    *
 ******************************************************************************/

inline serializer&   operator<<(serializer& s,   const unit&) { return s; }
inline deserializer& operator>>(deserializer& d, unit&) { return d; }


/******************************************************************************
 *                               HAMcast types                                *
 ******************************************************************************/

serializer&   operator<<(serializer&,   const uri&);
deserializer& operator>>(deserializer&, uri&);

serializer&   operator<<(serializer&,   const multicast_packet&);
deserializer& operator>>(deserializer&, multicast_packet&);

serializer&   operator<<(serializer&,   const interface_property&);
deserializer& operator>>(deserializer&, interface_property&);

serializer& operator<<(serializer&, const ipc::message&);

inline serializer& operator<<(serializer& s, const ipc::message::ptr& mptr)
{
    return (s << *mptr);
}

deserializer& operator>>(deserializer&, ipc::message::ptr&);

/******************************************************************************
 *                                  integers                                  *
 ******************************************************************************/

// deserialization of integers
template<typename T>
typename boost::enable_if<boost::is_integral<T>, serializer&>::type
operator<<(serializer& s, T what)
{
    s.write(sizeof(T), reinterpret_cast<const void*>(&what));
    return s;
}

inline serializer& operator<<(serializer& s, bool what)
{
    return (s << static_cast<boost::uint8_t>(what ? 1 : 0));
}

template<typename T>
typename boost::enable_if<boost::is_integral<T>, deserializer&>::type
operator>>(deserializer& d, T& storage)
{
    d.read(sizeof(T), reinterpret_cast<void*>(&storage));
    return d;
}

deserializer& operator>>(deserializer&, bool&);


/******************************************************************************
 *                    standard template library containers                    *
 ******************************************************************************/

serializer&   operator<<(serializer&,   const std::string&);
deserializer& operator>>(deserializer&, std::string&);

template<typename T1, typename T2>
serializer& operator<<(serializer& s, const std::pair<T1,T2>& what)
{
    return (s << what.first << what.second);
}

template<typename T1, typename T2>
serializer& operator<<(serializer& s, const std::pair<const T1, T2>& what)
{
    return (s << what.first << what.second);
}

template<typename T1, typename T2>
deserializer& operator>>(deserializer& d, std::pair<T1,T2>& storage)
{
    return (d >> storage.first >> storage.second);
}

template<typename Container>
serializer& serialize_container(serializer& s, const Container& what)
{
    s << (static_cast<boost::uint32_t>(what.size()));
    for (typename Container::const_iterator i(what.begin());
         i != what.end();
         ++i)
    {
        s << (*i);
    }
    return s;
}

template<typename K, typename V>
serializer& operator<<(serializer& s, const std::map<K,V>& what)
{
    return serialize_container(s, what);
}

template<typename T>
serializer& operator<<(serializer& s, const std::vector<T>& what)
{
    return serialize_container(s, what);
}

template<typename T>
serializer& operator<<(serializer& s, const std::list<T>& what)
{
    return serialize_container(s, what);
}

template<typename Container>
inline void reserve_storage(Container&, boost::uint32_t) { }

template<typename T>
inline void reserve_storage(std::vector<T>& vec, boost::uint32_t vec_size)
{
    vec.reserve(vec_size);
}

template<typename K, typename V>
void deserialize_element(deserializer& d, std::map<K,V>& container)
{
    K key;
    V value;
    d >> key >> value;
    container.insert(std::map<K,V>::value_type(key, value));
}

template<typename Container>
void deserialize_element(deserializer& d, Container& container)
{
    typename Container::value_type element;
    d >> element;
    container.push_back(element);
}

template<typename Container>
deserializer& deserialize_container(deserializer& d, Container& container)
{
    boost::uint32_t num_elements;
    d >> num_elements;
    reserve_storage(container, num_elements);
    for (boost::uint32_t i = 0; i < num_elements; ++i)
    {
        deserialize_element(d, container);
    }
    return d;
}

template<typename K, typename V>
deserializer& operator>>(deserializer& d, std::map<K,V>& container)
{
    return deserialize_container(d, container);
}

template<typename T>
deserializer& operator>>(deserializer& d, std::vector<T>& container)
{
    return deserialize_container(d, container);
}

template<typename T>
deserializer& operator>>(deserializer& d, std::list<T>& container)
{
    return deserialize_container(d, container);
}

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_SERIALIZATION_HPP
