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

#ifndef SEND_JOB_HPP
#define SEND_JOB_HPP

#include "session_fwd.hpp"
#include "abstract_socket.hpp"

#include "hamcast/uri.hpp"
#include "hamcast/ref_counted.hpp"
#include "hamcast/ipc/message.hpp"

namespace hamcast { namespace middleware {

class send_job : public ref_counted
{

    //ipc::async_send_view m_msg;
    ipc::message::ptr m_msg;
    uri m_receiver;
    boost::uint8_t m_ttl;
    session_ptr m_client;

 public:

    typedef intrusive_ptr<send_job> ptr;

    send_job(const ipc::message::ptr& msg,
             const uri& receiver_group,
             boost::uint8_t ttl,
             const session_ptr& client);

    ~send_job();

    inline ipc::async_send_view send_message()
    {
        return ipc::async_send_view(*m_msg);
    }

    inline const uri& receiver() const { return m_receiver; }

    inline boost::uint8_t ttl() const { return m_ttl; }

};

} } // namespace hamcast::middleware

#endif // SEND_JOB_HPP
