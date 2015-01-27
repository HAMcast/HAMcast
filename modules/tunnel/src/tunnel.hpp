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

#ifndef _TUNNEL_HPP_
#define _TUNNEL_HPP_

#include <cstring>

#include <netinet/in.h>

#include <set>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

#include "tunnel_message.hpp"

#define TUNNEL_DEFAULT_PORT "1607"

typedef struct sockaddr_storage SA;

namespace tunnel_module {

class tunnel
{
private:
    hc_module_instance_handle_t     m_handle;               // module handle
    hc_log_fun_t                    m_log_fun;              // logging function
    hc_event_callback_t             m_event_cb;             // event callback
    hc_recv_callback_t              m_recv_cb;              // receive callback
    hc_atomic_msg_size_callback_t   m_msize_cb;             // atomic msg callback

    bool                            m_open;
    int                             m_sendsock;             // send socket
    int                             m_recvsock;             // recv/listen socket
    //int                             m_sockfd;               // socket
    struct sockaddr_storage         m_local_addr;           // local src addr
    struct sockaddr_storage         m_remote_addr;          // remote dst addr
    std::set<hamcast::uri>          m_local_groups;         // local joined grps
    boost::mutex                    m_local_groups_mutex;   // local grp mutex
    std::map<hamcast::uri,std::set<SA> > m_remote_groups;        // remote joined grps
    boost::mutex                    m_remote_groups_mutex;   // remote grp mutex
    std::set<hamcast::uri>          m_tmp_groups;           // local tmp grps
    boost::mutex                    m_tmp_groups_mutex;      // local tmp mutex
    boost::thread                   m_loop;                 // receive loop thread

    void init();

    /**
     * @brief Well, tear down the tunnel and shutdown this interface
     */
    void shutdown();

    /**
     * @brief Main thread to receive data from the tunnel
     */
    void receive_loop();

    /**
     * @brief helper function to get receive loop
     * @param IP instance
     */
    static void receive_loop_helper(tunnel* self)
    {
        self->receive_loop();
    }

    /**
     * @brief Helper function to notify receive loop
     */
    void notify_receive_loop();

    int sendto (const tunnel_message& msg, SA dst);

    void send_general_query ();

    void send_group_query (hamcast::uri& group_uri);

    void send_general_report ();

    void send_group_report (hamcast::uri& group_uri);

public:

    /**
     * @brief tunnel
     * @param lf
     * @param rcb
     * @param ecb
     * @param mcb
     */
    tunnel (hc_log_fun_t lf, hc_event_callback_t ecb,
            hc_recv_callback_t rcb, hc_atomic_msg_size_callback_t mcb);

    /**
     * @brief tunnel
     * @param lf
     * @param rcb
     * @param ecb
     * @param mcb
     * @param la
     * @param lp
     * @param ra
     * @param rp
     */
    tunnel (hc_log_fun_t lf, hc_event_callback_t ecb,
            hc_recv_callback_t rcb, hc_atomic_msg_size_callback_t mcb,
            const std::string& la, const std::string& lp,
            const std::string& ra, const std::string& rp);

    /**
     * Destructor tearing down this tunnel
     */
    ~tunnel ()
    {
        this->shutdown();
    }

    /**
     * @brief set_handle
     * @param hdl
     */
    inline void set_handle (hc_module_instance_handle_t hdl)
    {
        m_handle = hdl;
    }

    inline hc_module_instance_handle_t get_handle ()
    {
        return m_handle;
    }

    /**
     * @brief send
     * @param group_uri
     * @param buf
     * @param len
     * @param ttl
     * @return
     */
    int send (const hamcast::uri& group_uri, const void* buf, size_t len, size_t);

    /**
     * @brief join
     * @param group_uri
     * @return
     */
    int join (const hamcast::uri& group_uri);

    /**
     * @brief leave
     * @param group_uri
     * @return
     */
    int leave (const hamcast::uri& group_uri);

    /**
     * @brief map
     * @param group_uri
     * @return
     */
    hamcast::uri map (const hamcast::uri& group_uri);

    /**
     * @brief parent_set
     * @param result
     * @param group_uri
     */
    void parent_set(std::vector<hamcast::uri>& result, const hamcast::uri&);

    /**
     * @brief children_set
     * @param result
     * @param group_uri
     */
    void children_set(std::vector<hamcast::uri>& result, const hamcast::uri& group_uri);

    /**
     * @brief designated_host
     * @param group_uri
     * @return
     */
    bool designated_host (const hamcast::uri& group_uri);

    /**
     * @brief neighbor_set
     * @param result
     */
    void neighbor_set(std::vector<hamcast::uri>& result);

    /**
     * @brief group_set
     * @param result
     */
    void group_set(std::vector<std::pair<hamcast::uri, int> >& result);

    inline bool open()
    {
        return m_open;
    }

    inline std::string get_ifname ()
    {
        return "tun0";
    }

    std::string get_ifaddr ();

    /* XXX: DEBUG FUNCTIONS FOR TESTING ONLY */
    void __add_remote_listener (hamcast::uri &group_uri, SA &node);

}; // class tunnel

} // namespace tunnel_module

inline bool operator<(const SA& lhs, const SA& rhs)
{
    if (lhs.ss_family == rhs.ss_family)
    {
        if (lhs.ss_family == AF_INET) {
            if(memcmp(&(((struct sockaddr_in*)&lhs)->sin_addr),
                      &(((struct sockaddr_in*)&rhs)->sin_addr),
                      sizeof(struct in_addr))<0)
                return true;
        }
        else // address family is AF_INET6
        {
            if(memcmp(&(((struct sockaddr_in6*)&lhs)->sin6_addr),
                      &(((struct sockaddr_in6*)&rhs)->sin6_addr),
                      sizeof(struct in6_addr))<0)
                return true;
        }
    }
    return false;
}

#endif // _TUNNEL_HPP_
