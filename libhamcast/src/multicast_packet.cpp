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

#include "hamcast/uri.hpp"
#include "hamcast/ref_counted.hpp"
#include "hamcast/multicast_packet.hpp"


namespace { typedef boost::uint32_t ui32; }

namespace hamcast { namespace detail {

// implicit reference counting
struct multicast_packet_private : public ref_counted
{

    char* data;
    const ui32 size;
    const uri source_uri;

    multicast_packet_private() : data(0), size(0) { }

    multicast_packet_private(char* mdata, ui32 msize, uri muri) :
            data(mdata), size(msize), source_uri(muri) { }

    ~multicast_packet_private()
    {
        if (data) delete[] data;
    }

};

} } // namespace hamcast::detail

namespace hamcast {

multicast_packet::multicast_packet() : d(new detail::multicast_packet_private) { }

multicast_packet::multicast_packet(const uri& source,
                                   boost::uint32_t buf_size,
                                   void* buf) :
    d(new detail::multicast_packet_private(reinterpret_cast<char*>(buf),
                                           buf_size, source))
{
}

multicast_packet& multicast_packet::operator=(const multicast_packet& other)
{
    d = other.d;
    return *this;
}

const uri& multicast_packet::from() const
{
    return d->source_uri;
}

const void* multicast_packet::data() const
{
    return const_cast<const void*>(reinterpret_cast<void*>(d->data));
}

boost::uint32_t multicast_packet::size() const
{
    return d->size;
}

void multicast_packet::add_ref::operator()(detail::multicast_packet_private* rc)
{
    ref_counted::add_ref(rc);
}

void multicast_packet::release::operator()(detail::multicast_packet_private* rc)
{
    ref_counted::release(rc);
}

} // namespace hamcast

/*
void operator>>(hamcast::details::deserializer& d,
                hamcast::multicast_packet& mp)
{
    using namespace hamcast;
    using namespace hamcast::details;

    std::string uri_str;
    d >> uri_str;
    boost::uint32_t data_size;
    d >> data_size;
    char* data = new char[data_size];
    d.read(data, data_size);
    mp.d.set(new multicast_packet_private(data, data_size, uri_str));
}
*/
