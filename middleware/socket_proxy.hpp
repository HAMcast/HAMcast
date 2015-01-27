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

#ifndef SOCKET_PROXY_HPP
#define SOCKET_PROXY_HPP

#include <set>
#include <boost/thread/mutex.hpp>

#include "tech_module.hpp"
#include "session_fwd.hpp"
#include "abstract_socket.hpp"

#include "hamcast/socket_id.hpp"

#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/id_generator.hpp"

#include "hamcast/ipc/channel.hpp"
#include "hamcast/ipc/stream_id.hpp"
#include "hamcast/ipc/sequence_number.hpp"

namespace hamcast { namespace middleware {

typedef std::vector<tech_module::ptr> tech_module_vec;

struct stream_data
{
    uri outgoing_group;
    inline stream_data(const uri& out_group) : outgoing_group(out_group) { }
};

class socket_proxy : public abstract_socket
{

    typedef util::storage_semaphore<default_max_buffer_size, util::nonblocking_t>
            storage_sem_type;

    ipc::sequence_number last_acknowledged;
    ipc::sequence_number next_expected_seq_nr;
    long ack_counter;

    boost::uint8_t m_ttl;
    session_ptr m_parent;
    tech_module_vec m_interfaces;
    std::set<uri> m_joined_groups;
    std::map<ipc::stream_id,stream_data> m_streams;
    util::id_generator<ipc::stream_id> m_stream_ids;
    storage_sem_type m_send_buf;
    storage_sem_type m_receive_buf;

 public:

    typedef intrusive_ptr<socket_proxy> ptr;

    inline boost::uint8_t ttl() const { return m_ttl; }

    inline void set_ttl(boost::uint8_t value) { m_ttl = value; }

    const tech_module_vec& interfaces() const { return m_interfaces; }

    void set_interfaces(const tech_module_vec& tms);

    socket_proxy(socket_id sid, const session_ptr& parent_session);

    ~socket_proxy();

    void leave_all(bool clear_joined_groups = true);

    /**
     * @brief Check if @p asv matches the next expected sequence number
     *        (otherwise this member function does nothing) and
     *        delegates @p asv to all assigned output channels if enough
     *        storage could be acquired. A retransmit message is written to
     *        @p s if the acquirement failed.
     */
    void handle_async_send(const ipc::async_send_view& asv,
                           util::serializer& s,
                           const session_ptr& sptr);

    /**
     * @brief Release storage and send ACK message if needed.
     */
    void async_job_done(const ipc::async_send_view& asv, util::serializer& s);

    /**
     * @brief Add a new output stream to the socket and return the ID of the
     *        created stream.
     */
    ipc::stream_id add_stream(const uri& group);

    void join(const uri& what);

    void leave(const uri& what);

    //virtual void async_receive(const hamcast::multicast_packet& mp);
    bool acquire_bytes(size_t num);

    void delivered(size_t num_bytes);

    bool force_acks(util::serializer& s);

};

} } // namespace hamcast::middleware

#endif // SOCKET_PROXY_HPP
