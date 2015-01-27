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

#ifndef HAMCAST_MULTICAST_PACKET_HPP
#define HAMCAST_MULTICAST_PACKET_HPP

#include <boost/cstdint.hpp>

#include "hamcast/intrusive_ptr.hpp"

namespace hamcast {

// forward declarations
class uri;
namespace detail { struct multicast_packet_private; }

/**
 * @brief Describes a received multicast packet.
 */
class multicast_packet
{

    struct add_ref { void operator()(detail::multicast_packet_private*); };
    struct release { void operator()(detail::multicast_packet_private*); };

    intrusive_ptr<detail::multicast_packet_private, add_ref, release> d;

    multicast_packet(detail::multicast_packet_private*);

 public:

    /**
     * @brief Construct an empty multicast packet.
     */
    multicast_packet();

    /**
     * @brief Construct a new multicast packet.
     * @param source the source of this packet
     * @param size the size of @p buf
     * @param buf the data of this packet
     * @warning multicast_packet takes ownership of @p buf.
     */
    multicast_packet(const uri& source, boost::uint32_t size, void* buf);

    multicast_packet& operator=(const multicast_packet& other);

    /**
     * @brief Get the source of this packet.
     * @returns The source group as {@link hamcast::uri uri} object.
     */
    const uri& from() const;

    /**
     * @brief Get the content/data of the packet.
     * @returns A pointer to the internal C-buffer.
     */
    const void* data() const;

    /**
     * @brief Get the size of {@link data()}.
     * @returns The size of {@link data()} in bytes.
     */
    boost::uint32_t size() const;

    /**
     * @brief Check if this packet is empty.
     * @returns <code>true</code> if <code>{@link size()} == 0</code>;
     *         otherwise <code>false</code>.
     */
    inline bool empty() const { return size() == 0; }

};

} // namespace hamcast

#endif // HAMCAST_MULTICAST_PACKET_HPP
