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

#ifndef ABSTRACT_SOCKET_HPP
#define ABSTRACT_SOCKET_HPP

#include "hamcast/socket_id.hpp"
#include "hamcast/ref_counted.hpp"
#include "hamcast/multicast_packet.hpp"

namespace hamcast { namespace middleware {

class session;
class send_job;

/**
 * @brief This interface describes a listener that handles received packages.
 */
class abstract_socket : public ref_counted
{

 public:

    typedef intrusive_ptr<abstract_socket> ptr;

    inline session* parent_session() const { return m_parent_session; }

    inline socket_id id() const { return m_id; }

    virtual bool acquire_bytes(size_t num) = 0;

    abstract_socket(socket_id id, session* parent_session);

 private:

    socket_id m_id;
    session* m_parent_session;

};

} } // namespace hamcast::middleware

#endif // RECEIVE_LISTENER_HPP
