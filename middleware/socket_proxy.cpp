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

#include <iostream>
#include <algorithm>

#include "hamcast/hamcast_logging.h"

#include "session.hpp"
#include "send_job.hpp"
#include "middleware.hpp"
#include "socket_proxy.hpp"

using hamcast::ipc::stream_id;

namespace hamcast { namespace middleware {

abstract_socket::abstract_socket(socket_id sck_id, session* ptr)
    : m_id(sck_id), m_parent_session(ptr)
{
}

bool socket_proxy::acquire_bytes(size_t num)
{
    return m_receive_buf.try_acquire(num);
}

void socket_proxy::delivered(size_t num_bytes)
{
    m_receive_buf.release(num_bytes);
}

ipc::stream_id socket_proxy::add_stream(const uri& group)
{
    HC_LOG_TRACE("u = " << group.str());
    ipc::stream_id result = m_stream_ids.next();
    m_streams.insert(std::make_pair(result, stream_data(group)));
    return result;
}

void socket_proxy::handle_async_send(const ipc::async_send_view& asv,
                                     util::serializer& s,
                                     const session_ptr& sptr)
{
    HC_LOG_TRACE("socket = " << id() << ", seq_nr = " << asv.seq_nr());
    //HC_REQUIRE(!m_outputs.empty());
    std::map<stream_id,stream_data>::iterator i = m_streams.find(asv.stm_id());
    if (i != m_streams.end())
    {
        if (next_expected_seq_nr == asv.seq_nr())
        {
            if (m_send_buf.try_acquire(asv.content_size()))
            {
                send_job::ptr sj(new send_job(asv.message_ptr(),
                                              i->second.outgoing_group,
                                              m_ttl,
                                              sptr));
                for (tech_module_vec::iterator t = m_interfaces.begin();
                     t != m_interfaces.end();
                     ++t)
                {
                    (*t)->add_job(sj);
                }
                next_expected_seq_nr += 1;
            }
            else
            {
                HC_LOG_DEBUG("try_acquire failed; seq_nr = " << asv.seq_nr());
                ipc::message::serialize_as_message(s,
                                                   ipc::retransmit,
                                                   asv.stm_id(),
                                                   asv.sck_id(),
                                                   asv.seq_nr(),
                                                   0,
                                                   0);
                s.flush();
            }
        }
        else
        {
            HC_LOG_DEBUG(asv.seq_nr() << " out-of-order");
        }
        // else: ignore out-of-order messages
    }
    else if (i == m_streams.end())
    {
        HC_LOG_ERROR("Unknown stream: " << asv.stm_id());
    }
}

void socket_proxy::async_job_done(const ipc::async_send_view& asv,
                                  util::serializer& s)
{
    HC_LOG_TRACE("socket = " << id());
    std::map<stream_id,stream_data>::iterator i = m_streams.find(asv.stm_id());
    if (i != m_streams.end())
    {
        stream_data& sdata = i->second;
        long ac = (ack_counter + 1) % max_ack_block_size;
        if (ac == 0 || (m_send_buf.acquired() * 2) >= m_send_buf.maximum())
        {
            HC_LOG_DEBUG("ACK sent; stream = " << i->first
                         << ", seq_nr = " << asv.seq_nr());
            ipc::message::serialize_as_message(s,
                                               ipc::cumulative_ack,
                                               i->first,
                                               id(),
                                               asv.seq_nr(),
                                               0,
                                               0);
            last_acknowledged = asv.seq_nr();
            s.flush();
        }
        m_send_buf.release(asv.content_size());
        ack_counter = ac;
    }
    else if (i == m_streams.end())
    {
        HC_LOG_DEBUG("Unknown stream: " << asv.stm_id());
    }
}

bool socket_proxy::force_acks(util::serializer& s)
{
    //HC_LOG_TRACE("socket = " << id());
    bool result = false;
    for (std::map<stream_id,stream_data>::iterator i = m_streams.begin();
         i != m_streams.end();
         ++i)
    {
        stream_data& sdata = i->second;
        if (ack_counter > 0)
        {
            result = true;
            ipc::sequence_number sqnr = last_acknowledged;
            sqnr += static_cast<ipc::sequence_number>(ack_counter);
            HC_LOG_DEBUG("ACK forced; stream = " << i->first
                         << ", seq_nr = " << sqnr);
            ipc::message::serialize_as_message(s,
                                               ipc::cumulative_ack,
                                               i->first,
                                               id(),
                                               sqnr,
                                               0,
                                               0);
            last_acknowledged = sqnr;
            ack_counter = 0;
        }
    }
    return result;
}

void socket_proxy::join(const uri& what)
{
    HC_REQUIRE(!m_interfaces.empty());
    if (m_joined_groups.insert(what).second)
    {
        abstract_socket::ptr self_ptr(this);
        for (tech_module_vec::iterator i = m_interfaces.begin();
             i != m_interfaces.end();
             ++i)
        {
            (*i)->join(what, self_ptr);
        }
    }
}

void socket_proxy::leave(const uri& what)
{
    HC_REQUIRE(!m_interfaces.empty());
    if (m_joined_groups.erase(what) > 0)
    {
        abstract_socket::ptr self_ptr(this);
        for (tech_module_vec::iterator i = m_interfaces.begin();
             i != m_interfaces.end();
             ++i)
        {
            (*i)->leave(what, self_ptr);
        }
    }
}

void socket_proxy::leave_all(bool clear_joined_groups)
{
    if (!m_interfaces.empty())
    {
        abstract_socket::ptr self_ptr(this);
        for (tech_module_vec::iterator i = m_interfaces.begin();
             i != m_interfaces.end();
             ++i)
        {
            (*i)->leave_all(self_ptr);
        }
        if (clear_joined_groups) m_joined_groups.clear();
    }
}

std::string vec_to_string(const tech_module_vec& vec)
{
    std::ostringstream res;
    res << "{ ";
    for (tech_module_vec::const_iterator i = vec.begin(); i != vec.end(); ++i)
    {
        if (i != vec.begin()) res << ", ";
        res << (*i)->iface_id();
    }
    res << " }";
    return res.str();
}

void find_and_erase(tech_module_vec& vec, const tech_module::ptr& ptr)
{
    tech_module_vec::iterator i = std::find(vec.begin(), vec.end(), ptr);
    if (i != vec.end()) vec.erase(i);
}

void socket_proxy::set_interfaces(const tech_module_vec& const_tms)
{
    HC_LOG_TRACE("current interfaces = " << vec_to_string(m_interfaces)
                 << ", new interfaces = " << vec_to_string(const_tms));
    tech_module_vec tms = const_tms;
    // get intersection of m_outputs and tms
    if (tms.size() > 1) std::sort(tms.begin(), tms.end());
    if (m_interfaces.size() > 1) std::sort(m_interfaces.begin(), m_interfaces.end());
    tech_module_vec intersection(tms.size() + m_interfaces.size());
    tech_module_vec::iterator intersection_end =
            std::set_intersection(tms.begin(), tms.end(),
                                  m_interfaces.begin(), m_interfaces.end(),
                                  intersection.begin());
    // erase intersection from m_outputs and tms
    for (tech_module_vec::iterator i = intersection.begin();
         i != intersection_end;
         ++i)
    {
        find_and_erase(m_interfaces, *i);
        find_and_erase(tms, *i);
    }
    // leave joined groups on all remaining output interfaces
    if (!m_interfaces.empty()) leave_all(false);
    // join groups on all remaining (distinct) elements of tms
    abstract_socket::ptr self_ptr(this);
    for (std::set<uri>::iterator i = m_joined_groups.begin();
         i != m_joined_groups.end();
         ++i)
    {
        for (tech_module_vec::iterator j = tms.begin(); j != tms.end(); ++j)
        {
            (*j)->join(*i, self_ptr);
        }
    }
    // set m_outputs = intersection + tms
    m_interfaces.clear();
    m_interfaces.insert(m_interfaces.end(), intersection.begin(), intersection_end);
    m_interfaces.insert(m_interfaces.end(), tms.begin(), tms.end());
}

socket_proxy::socket_proxy(socket_id sid, const session_ptr& parent_session)
    : abstract_socket(sid, parent_session.get()), last_acknowledged(0)
    , next_expected_seq_nr(0), ack_counter(0)
    , m_ttl(1), m_parent(parent_session)
{
    tech_module_vec possibilities = tech_module::instances();
    HC_REQUIRE(!possibilities.empty());
    m_interfaces.push_back(possibilities.front());
}

socket_proxy::~socket_proxy()
{
}

} } // namespace hamcast::middleware
