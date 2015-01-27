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

#ifndef HAMCAST_UTIL_SOCKET_IO_HPP
#define HAMCAST_UTIL_SOCKET_IO_HPP

#include <boost/cstdint.hpp>

#include "hamcast/util/sink.hpp"
#include "hamcast/util/source.hpp"

namespace hamcast { namespace util {

#ifdef WIN_32
#else
typedef int native_socket;
inline void closesocket(native_socket s) { ::close(s); }
#endif

struct socket_io : sink, source
{
    /**
     * @brief An intrusive pointer to an instance of {@link socket_io}.
     */
    typedef intrusive_ptr<socket_io> ptr;

    /**
     * @brief Create an (unbuffered) socket sink/source connected to the
     *        ip @p host_ip on port @p host_port.
     */
    static ptr create(const char* host_ip, boost::uint16_t host_port);

    /**
     * @brief Create an (unbuffered) socket sink/source from the native
     *        socket @p s.
     */
    static ptr create(native_socket s);

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_SOCKET_IO_HPP
