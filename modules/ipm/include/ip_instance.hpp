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

#ifndef IP_INSTANCE_HPP
#define IP_INSTANCE_HPP

#include <cstddef>
#include <sstream>
#include <string>
#include <iostream>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <set>

#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/algorithm/string.hpp>


#include "ip_socket.hpp"
#include "ip_exceptions.hpp"
#include "ip_module.hpp"

#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

namespace ipm
{
    const boost::uint32_t s_queue_event = 0x00;
    const boost::uint32_t s_quit_event = 0x01;

    enum ip_instance_operation
    {
        iio_join,
        iio_leave
    };

    struct ip_instance_msg
    {
        ip_instance_msg* next;
        ip_instance_operation operation;
        hamcast::uri uri_arg;
        ip_instance_msg(ip_instance_operation iio, const hamcast::uri& arg)
            : operation(iio), uri_arg(arg)
        {
        }
    };

    class ip_instance
    {

    private:
        hc_module_instance_handle_t     m_handle;   // module handle
        hc_log_fun_t                    m_log_fun;  // logging function
        hc_event_callback_t             m_event_cb; // event callback
        hc_recv_callback_t              m_recv_cb;  // receive callback
        hc_atomic_msg_size_callback_t   m_msg_cb;   // atomic msg size callback
        struct sockaddr_storage         m_if_addr;  // interface IP address
        unsigned int                    m_if_index; // interface index
        std::string                     m_if_name;  // interface name
        ip_socket*                      m_sendsock; // send socket
        int                             m_pipe[2];  // receive loop queue
        boost::thread                   m_loop;     // receive loop thread
        hamcast::uri                    m_querier;  // parent, querier
        std::set<hamcast::uri>          m_neighbors; // routing neighbors
        std::set<hamcast::uri>          m_groups;   // joined groups
        hamcast::util::single_reader_queue<ip_instance_msg> m_queue;

        /**
         * @brief Returns read file handle of receive loop pipe
         * @return file handle
         */
        inline int read_handle() const
        {
            return m_pipe[0];
        }

        /**
         * @brief Returns write file handle of receive loop pipe
         * @return file handle
         */
        inline int write_handle() const
        {
            return m_pipe[1];
        }

        /**
         * @brief Helper function to initialize IP instance
         */
        void init();

        /**
         * @brief Lock free receive loop, main thread of instance
         */
        void receive_loop();

        /**
         * @brief helper function to get receive loop
         * @param IP instance
         */
        static void receive_loop_helper(ip_instance* self)
        {
            self->receive_loop();
        }

        /**
         * @brief Helper function to notify receive loop
         */
        void notify_receive_loop();

    public:

        /**
         * @brief Minimal constructor of ip_instance
         */
        ip_instance(hc_log_fun_t, hc_event_callback_t,
                    hc_recv_callback_t, hc_atomic_msg_size_callback_t);

        /**
         * @brief Extended constructor of ip_instance
         */
        ip_instance(hc_log_fun_t, hc_event_callback_t,
                    hc_recv_callback_t, hc_atomic_msg_size_callback_t,
                    const unsigned int, const std::string&, struct sockaddr_storage);

        /**
         * @brief Destroy IP module instance and kill receive thread
         */
        ~ip_instance()
        {
            HC_LOG_TRACE("");
            kill_receive_loop();
        }

        /**
         *  @brief Stop receiv loop thread
         */
        void kill_receive_loop();

        /**
         * @brief set_handle
         * @param hdl
         */
        inline void set_handle (hc_module_instance_handle_t hdl)
        {
            m_handle = hdl;
        }

        /**
         * @brief get_querier
         * @return Querier address
         */
        inline hamcast::uri get_querier ()
        {
            return m_querier;
        }

        /**
         * @brief set_querier
         * @param Querier address
         */
        inline void set_querier (hamcast::uri& q)
        {
            m_querier = q;
        }

        /**
         * @brief Add a node as routing neighbor
         * @param Node address
         */
        inline void add_neighbor (hamcast::uri& n)
        {
            m_neighbors.insert(n);
        }

        /*** API CALLS ***/
        /**
         * @brief Join multicast group
         * @param group_uri Group URI object
         * @param group_str Group URI as C string
         * @return Error Code
         */
        int join(const hamcast::uri& group_uri);

        /**
         * @brief Leave multicast group
         * @param group_uri Group URI object
         * @param group_str Group URI as C string
         * @return Error Code
         */
        int leave(const hamcast::uri& group_uri);

        /**
         * @brief Send data to multicast group
         * @param buf Send buffer
         * @param len Number of bytes in buffer to be send
         * @return Bytes send on success, error code otherwise
         */
        int send(const  hamcast::uri& group_uri, const void* buf, int len, unsigned char ttl);

        /**
         * @brief Map URI or its string representation to a tech specific address
         * @param group_uri Group URI object
         * @param group_str Group URI as C string
         * @return Mapped address
         */
        hamcast::uri map(const hamcast::uri& group_uri);

        /**
         * @brief neighbor_set
         * @param result
         */
        void neighbor_set (std::vector<hamcast::uri>& result);

        /**
         * @brief group_set
         * @param result
         */
        void group_set (std::vector<std::pair<hamcast::uri, int> >& result);

        /**
         * @brief children_set
         * @param result
         * @param group_uri
         */
        void children_set(std::vector<hamcast::uri>& result, const hamcast::uri& group_uri);

        /**
         * @brief parent_set
         * @param result
         * @param group_uri
         */
        void parent_set(std::vector<hamcast::uri>& result, const hamcast::uri& group_uri);

        /**
         * @brief Check if this node is a multicast forwarder of a specific group
         * @param group_uri Group URI object
         * @param group_str Group URI as C string
         * @return Return true (1), if host is forwards group data, false (0) otherwise
         */
        int designated_host(const hamcast::uri& group_uri);

        inline std::string get_name ()
        {
            return m_if_name;
        }

        inline std::string get_address ()
        {
            char ifa_str[INET6_ADDRSTRLEN];
            std::string ret_str;
            if (m_if_addr.ss_family == AF_INET) {
                inet_ntop (AF_INET, &((struct sockaddr_in*)&m_if_addr)->sin_addr, ifa_str, sizeof(ifa_str));
            }
            else if (m_if_addr.ss_family == AF_INET6) {
                inet_ntop (AF_INET6, &((struct sockaddr_in6*)&m_if_addr)->sin6_addr, ifa_str, sizeof(ifa_str));
            }
            return std::string(ifa_str);
        }
    };
}

#endif // IP_INSTANCE_HPP
