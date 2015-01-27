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

#include <cstdlib>

#include "hamcast/uri.hpp"
#include "hamcast/ipc/message.hpp"
#include "hamcast/util/read_buffer.hpp"
#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/serialization.hpp"
#include "hamcast/util/atomic_operations.hpp"

namespace hamcast { namespace ipc {

message::ptr message::create(message_type mtype,
                             boost::uint16_t field1,
                             boost::uint32_t field2,
                             boost::uint32_t field3,
                             boost::uint32_t ct_size,
                             char* ct)
{
    HC_REQUIRE(((ct) && ct_size > 0) || ((!ct) && ct_size == 0));
    return new message(mtype, field1, field2, field3, ct_size, ct);
}

#define COMPARE_FIELD(what)                                                    \
    if ( what > other. what ) return  1;                                       \
    if ( what < other. what ) return -1

int message::compare(const message& other) const
{
    COMPARE_FIELD(m_type);
    COMPARE_FIELD(m_field1);
    COMPARE_FIELD(m_field2);
    COMPARE_FIELD(m_field3);
    COMPARE_FIELD(m_content_size);
    COMPARE_FIELD(m_content);
    return 0;
}

util::deserializer& message::deserialize(util::deserializer& d,
                                         message::ptr& result)
{
    boost::uint16_t mtype;
    boost::uint16_t f1;
    boost::uint32_t f2;
    boost::uint32_t f3;
    boost::uint32_t ct_size;
    char* ct = 0;
    d >> mtype >> f1 >> f2 >> f3 >> ct_size;
    if (ct_size > 0)
    {
        ct = new char[ct_size];
        d.read(ct_size, ct);
    }
    result = create(static_cast<message_type>(mtype), f1, f2, f3, ct_size, ct);
    return d;
}

util::serializer& message::serialize(util::serializer& s,
                                     const message::ptr& mptr)
{
    return (mptr) ? serialize(s, *mptr) : s;
}

util::serializer& message::serialize(util::serializer& s, const message& m)
{
    s << static_cast<boost::uint16_t>(m.m_type)
      << m.m_field1 << m.m_field2 << m.m_field3
      << m.m_content_size;
    if (m.m_content_size > 0) s.write(m.m_content_size, m.m_content);
    return s;
}

void message::serialize_as_message(util::serializer& s,
                                   message_type mtype,
                                   boost::uint16_t field1,
                                   boost::uint32_t field2,
                                   boost::uint32_t field3,
                                   boost::uint32_t ct_size,
                                   const void* ct)
{
    s << static_cast<boost::uint16_t>(mtype)
      << field1 << field2 << field3
      << ct_size;
    if (ct_size > 0) s.write(ct_size, ct);
}

void message::serialize_as_message(util::serializer& s,
                                   const membership_event& event)
{
    // header
    s << static_cast<boost::uint16_t>(async_event)
      << static_cast<boost::uint16_t>(event.type())
      << static_cast<boost::uint32_t>(event.iface_id())
      << static_cast<boost::uint32_t>(0) // field3 unused
      // content size = size of group.str (uint32) + group.str.size
      << static_cast<boost::uint32_t>(  sizeof(boost::uint32_t)
                                      + event.group().str().size())
    // content
      << event.group();
}

void message::serialize_as_message(util::serializer& s,
                                   socket_id receiver_sid,
                                   const multicast_packet& pck)
{
    s // type
      << static_cast<boost::uint16_t>(async_recv)
      // field1 - unused
      << static_cast<boost::uint16_t>(0)
      // field2 - socket id of the receiving socket
      << receiver_sid
      // field3 - unused
      << static_cast<boost::uint32_t>(0)
      // content_size =
      //     sizeof(uint32) (contains the uri size)
      //   + uri size
      //   + sizeof(uint32) (contains the data size)
      //   + data size
      << static_cast<boost::uint32_t>(
              sizeof(boost::uint32_t)
            + pck.from().str().size()
            + sizeof(boost::uint32_t)
            + pck.size())
      // serialize uri (this writes an uint32 + uri.str().c_str())
      << pck.from()
      // serialize data + its size
      << static_cast<boost::uint32_t>(pck.size());
    s.write(pck.size(), pck.data());
}

void message::serialize_as_message(util::serializer& s,
                                   socket_id receiver_sid,
                                   const uri& source,
                                   size_t data_size,
                                   const void* data)
{
    s // type
      << static_cast<boost::uint16_t>(async_recv)
      // field1 - unused
      << static_cast<boost::uint16_t>(0)
      // field2 - socket id of the receiving socket
      << receiver_sid
      // field3 - unused
      << static_cast<boost::uint32_t>(0)
      // content_size =
      //     sizeof(uint32) (contains the uri size)
      //   + uri size
      //   + sizeof(uint32) (contains the data size)
      //   + data size
      << static_cast<boost::uint32_t>(
              sizeof(boost::uint32_t)
            + source.str().size()
            + sizeof(boost::uint32_t)
            + data_size)
      // serialize uri (this writes an uint32 + uri.str().c_str())
      << source
      // serialize data + its size
      << static_cast<boost::uint32_t>(data_size);
    s.write(data_size, data);
}

message::~message()
{
    if (m_owns_content && m_content) delete[] m_content;
}

bool valid(message_type what)
{
    switch (what)
    {

     case sync_request:
     case sync_response:
     case async_event:
     case async_send:
     case async_recv:
     case cumulative_ack:
     case retransmit:
        return true;

     default: return false;

    }
}

} } // namespace hamcast::ipc
