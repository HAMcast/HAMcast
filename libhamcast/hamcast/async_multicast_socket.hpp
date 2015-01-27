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

#ifndef ASYNC_MULTICAST_SOCKET_HPP
#define ASYNC_MULTICAST_SOCKET_HPP

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/socket_id.hpp"
#include "hamcast/multicast_packet.hpp"

namespace hamcast {

class async_multicast_socket : boost::noncopyable
{

 public:

    typedef boost::function<void (const uri&, size_t, const void*)> receive_callback;

    async_multicast_socket(const receive_callback& cb);

    ~async_multicast_socket();

    /**
     * @brief Join a multicast group.
     * @param group multicast group that you want to join
     */
    void join(const uri& group);

 private:

    socket_id m_id;

};

} // namespace hamcast

#endif // ASYNC_MULTICAST_SOCKET_HPP
