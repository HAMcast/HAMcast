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

#if 0

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/thread.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"
#include "receive_listener.hpp"

namespace hamcast { namespace middleware {

/**
 * @brief Encapsulates an IPC session and monitors read and write access
 *        to the IPC channel.
 */
class connection : public hamcast::ref_counted
{

    hamcast::detail::stream_io_accessor::ptr m_sio;

    // prohibit copy and assignment
    connection(const connection&);
    connection& operator=(const connection&);

 public:

    /**
     * @brief A smart pointer to an instance of connection.
     */
    typedef hamcast::ptr<connection>::type ptr;

    connection(const hamcast::detail::stream_io_accessor::ptr& sio);

    /**
     * @brief Connect @p s to the monitored IPC channel.
     */
    void get_write_access(hamcast::ipc::serializer& s);

    /**
     * @brief Connect @p d to the monitored IPC channel.
     */
    void get_read_access(hamcast::ipc::deserializer& d);

};

} } // namespace hamcast::middleware

#endif // CONNECTION_HPP

#endif
