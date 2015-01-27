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

#ifndef TUNNEL_MESSAGE_HPP
#define TUNNEL_MESSAGE_HPP

#include "hamcast/hamcast.hpp"
#include <string>

namespace tunnel_module {

enum msg_type { TUN_MSG_INVALID,
                TUN_MSG_DATA,
                TUN_MSG_JOIN,
                TUN_MSG_JOIN_ACK,
                TUN_MSG_JOIN_ERROR,
                TUN_MSG_LEAVE,
                TUN_MSG_LEAVE_ACK,
                TUN_MSG_LEAVE_ERROR,
                TUN_MSG_QUERY,
                TUN_MSG_REPORT,
                TUN_MSG_MAX};




const size_t c_tun_msg_hdr_size = 4 * sizeof (uint16_t);

class tunnel_message
{
private:
    uint16_t        m_msg_type;
    uint16_t        m_msg_size;
    uint16_t        m_msg_port;
    uint16_t        m_grp_length;
    hamcast::uri    m_grp_uri;
    std::string     m_grp;
    char*           m_payload;
    char*           m_buffer;

    /**
     * @brief get_msg_type
     * @param msg
     * @return
     */
    uint16_t get_msg_type (const char* msg);

    /**
     * @brief get_msg_size
     * @param msg
     * @return
     */
    uint16_t get_msg_size (const char* msg);

    uint16_t get_msg_port (const char* msg);

    uint16_t get_grp_length (const char* msg);

    std::string get_grp_string (const char *msg, uint16_t grplen);

public:

    /**
     * @brief tunnel_message
     */
    tunnel_message();

    /**
     * @brief tunnel_message
     * @param t type of message
     */
    tunnel_message(const msg_type t);

    /**
     * @brief tunnel_message
     * @param t type of message, must be either TUN_MSG_JOIN or TUN_MSG_LEAVE
     * @param group create join/leave message for this group
     */
    tunnel_message(const msg_type t, const hamcast::uri& group);

    /**
     * @brief tunnel_message
     * @param payload
     * @param plen
     */
    tunnel_message(const hamcast::uri& group, const char *payload, const size_t plen);

    /**
     * @brief tunnel_message
     * @param t
     * @param payload
     * @param plen
     */
    tunnel_message(const msg_type t, const hamcast::uri& group, const char *payload, const size_t plen);

    /**
     * @brief Create tunnel_message from (received) message buffer
     * @param msg_buffer
     * @param msg_len
     */
    tunnel_message(char *msg_buffer, const size_t msg_len);

    ~tunnel_message()
    {
        //delete m_buffer;
    }

    /**
     * @brief Get message type
     * @return message type
     */
    inline uint16_t type () const
    {
        return m_msg_type;
    }


    /**
     * @brief Get message size
     * @return message size
     */
    inline uint16_t size () const
    {
        return m_msg_size;
    }

    inline uint16_t port () const
    {
        return m_msg_port;
    }

    inline const hamcast::uri group_uri () const
    {
        return m_grp_uri;
    }
    /**
     * @brief Get message payload
     * @return pointer to message payload
     */
    inline const char* payload () const
    {
        return m_payload;
    }

    inline uint16_t payload_size () const
    {
        return (m_msg_size - m_grp_length - c_tun_msg_hdr_size);
    }

    /**
     * @brief Get message buffer
     * @return pointer to message buffer
     */
    inline const char* buffer () const
    {
        return m_buffer;
    }

    /**
     * @brief Messsage as string
     * @return string representation of message
     */
    inline std::string str()
    {
        return std::string(m_buffer);
    }

    /**
     * @brief Set message type to data, nothing else allow for now
     */
    inline void set_type (msg_type mt)
    {
        if ((m_msg_type == TUN_MSG_INVALID) && (mt == TUN_MSG_DATA)) {
            m_msg_type = mt;
        }
    }

    inline std::string type_to_string (const msg_type t) const
    {
        switch (t) {
        case TUN_MSG_INVALID:
            return "INVALID";
        case TUN_MSG_DATA:
            return "DATA";
        case TUN_MSG_JOIN:
            return "JOIN";
        case TUN_MSG_JOIN_ACK:
            return "JOIN_ACK";
        case TUN_MSG_JOIN_ERROR:
            return "JOIN_ERROR";
        case TUN_MSG_LEAVE:
            return "LEAVE";
        case TUN_MSG_LEAVE_ACK:
            return "LEAVE ACK";
        case TUN_MSG_LEAVE_ERROR:
            return "LEAVE ERROR";
        case TUN_MSG_QUERY:
            return "QUERY";
        case TUN_MSG_REPORT:
            return "REPORT";
        case TUN_MSG_MAX:
            return "MAX MSG TYPE";
        default:
            return "UNKNOWN";
        }
    }
};

} // namepsace tunnel_module
#endif // TUNNEL_MESSAGE_HPP
