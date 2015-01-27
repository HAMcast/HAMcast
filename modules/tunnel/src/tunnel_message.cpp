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

#include <arpa/inet.h>

#include "hamcast/hamcast.hpp"

#include "tunnel_message.hpp"

using namespace tunnel_module;
using hamcast::uri;

tunnel_message::tunnel_message ()
{
    HC_LOG_TRACE("");
    m_msg_type = TUN_MSG_INVALID;
    m_msg_size = c_tun_msg_hdr_size;
    m_msg_port = 0;
    m_grp_length = 0;
    m_payload = NULL;
    m_buffer = new char[m_msg_size];
    char* write = m_buffer;
    uint16_t n_type = htons(m_msg_type);
    uint16_t n_size = htons(m_msg_size);
    uint16_t n_port = htons(m_msg_port);
    uint16_t n_length = htons(m_grp_length);
    memcpy (write, &n_type, sizeof(uint16_t));
    write += sizeof(n_type);
    memcpy (write, &n_size, sizeof(uint16_t));
    write += sizeof(n_size);
    memcpy (write, &n_port, sizeof(uint16_t));
    write += sizeof(n_port);
    memcpy (write, &n_length, sizeof(uint16_t));
}

tunnel_message::tunnel_message(const msg_type t)
{
    HC_LOG_TRACE("");
    m_msg_type = t;
    m_msg_size = c_tun_msg_hdr_size;
    m_msg_port = 0;
    m_grp_length = 0;
    m_payload = NULL;
    m_buffer = new char[m_msg_size];
    char* write = m_buffer;
    uint16_t n_type = htons(m_msg_type);
    uint16_t n_size = htons(m_msg_size);
    uint16_t n_port = htons(m_msg_port);
    uint16_t n_length = htons(m_grp_length);
    memcpy (write, &n_type, sizeof(uint16_t));
    write += sizeof(n_type);
    memcpy (write, &n_size, sizeof(uint16_t));
    write += sizeof(n_size);
    memcpy (write, &n_port, sizeof(uint16_t));
    write += sizeof(n_port);
    memcpy (write, &n_length, sizeof(uint16_t));
}

tunnel_message::tunnel_message(const msg_type t, const hamcast::uri& group_uri)
{
    HC_LOG_TRACE("");
    if (!group_uri.empty()) {
        if ( (t > TUN_MSG_DATA) || (t < TUN_MSG_QUERY) ) {
            m_grp_uri = group_uri;
            m_grp = group_uri.scheme() + "://";
            if (!group_uri.user_information().empty()) {
                m_grp += group_uri.user_information();
                m_grp += "@";
            }
            m_grp += group_uri.host();
            m_grp_length = m_grp.length();
            m_msg_port = group_uri.port_as_int();
            m_msg_type = t;
            m_msg_size = c_tun_msg_hdr_size + m_grp_length;

            m_payload = NULL;
            m_buffer = new char[m_msg_size];
            char *write = m_buffer;
            uint16_t n_type = htons(m_msg_type);
            uint16_t n_size = htons(m_msg_size);
            uint16_t n_port = htons(m_msg_port);
            uint16_t n_length = htons(m_grp_length);
            memcpy (write, &n_type, sizeof(uint16_t));
            write += sizeof(n_type);
            memcpy (write, &n_size, sizeof(uint16_t));
            write += sizeof(n_size);
            memcpy (write, &n_port, sizeof(uint16_t));
            write += sizeof(n_port);
            memcpy (write, &n_length, sizeof(uint16_t));
            write += sizeof(n_length);
            memcpy (write, m_grp.c_str (), m_grp.length ());
        }
        else {
            HC_LOG_ERROR("Invalid message type: " << t);
            throw 42;
        }
    }
    else {
        HC_LOG_ERROR ("Invalid group URI: " << group_uri.str() << " msg type: " << tunnel_message::type_to_string (t));
        throw 42;
    }
}

tunnel_message::tunnel_message(const msg_type t, const hamcast::uri& group_uri,
                               const char* payload, const size_t plen)
{
    HC_LOG_TRACE("");
    m_grp_uri = group_uri;
    m_grp = group_uri.scheme() + "://";
    if (!group_uri.user_information().empty()) {
        m_grp += group_uri.user_information();
        m_grp += "@";
    }
    m_grp += group_uri.host();
    m_grp_length = m_grp.length();
    m_msg_port = group_uri.port_as_int();
    m_msg_type = t;
    // calc full msg size
    m_msg_size = c_tun_msg_hdr_size;
    m_msg_size += m_grp_length;
    m_msg_size += plen;

    m_buffer = new char[m_msg_size];
    m_payload = m_buffer + c_tun_msg_hdr_size + m_grp_length;
    char *write = m_buffer;
    uint16_t n_type = htons(m_msg_type);
    uint16_t n_size = htons(m_msg_size);
    uint16_t n_port = htons(m_msg_port);
    uint16_t n_length = htons(m_grp_length);
    memcpy (write, &n_type, sizeof(uint16_t));
    write += sizeof(n_type);
    memcpy (write, &n_size, sizeof(uint16_t));
    write += sizeof(n_size);
    memcpy (write, &n_port, sizeof(uint16_t));
    write += sizeof(n_port);
    memcpy (write, &n_length, sizeof(uint16_t));
    write += sizeof(n_length);
    memcpy (write, m_grp.c_str (), m_grp.length ());
    write += m_grp_length;
    memcpy (write, payload, plen);
}

tunnel_message::tunnel_message(char *msg_buffer, const size_t msg_len)
{
    HC_LOG_TRACE("");
    m_msg_size = msg_len;
    if (m_msg_size != get_msg_size(msg_buffer)) {
        HC_LOG_ERROR ("Invalid message size!");
        m_msg_size = 0;
        return;
    }
    m_buffer = msg_buffer;
    m_msg_type = get_msg_type (m_buffer);
    HC_LOG_DEBUG("MSG TYPE: " << type_to_string (static_cast<msg_type>(m_msg_type)));
    m_msg_port = get_msg_port(m_buffer);
    HC_LOG_DEBUG("MSG PORT: " << m_msg_port);
    m_grp_length = get_grp_length(m_buffer);
    HC_LOG_DEBUG("MSG GRPLEN: " << m_grp_length);
    m_grp = get_grp_string (m_buffer, m_grp_length);
    HC_LOG_DEBUG("MSG GRP: " << m_grp);
    std::stringstream ss;
    ss << m_grp << ":" << m_msg_port;
    m_grp_uri = hamcast::uri(ss.str ());
    m_payload = m_buffer + c_tun_msg_hdr_size + m_grp_length;
}

// begin of private stuff
uint16_t tunnel_message::get_msg_type(const char *msg)
{
    uint16_t n_type;
    memcpy (&n_type, msg, sizeof(uint16_t));
    return (ntohs(n_type));
}

uint16_t tunnel_message::get_msg_size (const char *msg)
{
    uint16_t n_size;
    const char* read = msg+sizeof(uint16_t);
    memcpy (&n_size, read, sizeof(uint16_t) );
    return (ntohs(n_size));
}

uint16_t tunnel_message::get_msg_port (const char *msg)
{
    uint16_t n_port;
    const char* read = msg+(2*sizeof(uint16_t));
    memcpy (&n_port, read, sizeof(uint16_t) );
    return (ntohs(n_port));
}

uint16_t tunnel_message::get_grp_length (const char *msg)
{
    uint16_t n_length;
    const char* read = msg+(3*sizeof(uint16_t));
    memcpy (&n_length, read, sizeof(uint16_t) );
    return (ntohs(n_length));
}

std::string tunnel_message::get_grp_string (const char *msg, uint16_t grplen)
{
    const char* read = msg+(4*sizeof(uint16_t));
    return std::string(read,grplen);
}

// end-of private stuff
