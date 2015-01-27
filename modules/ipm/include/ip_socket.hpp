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
#ifndef IP_SOCKET_HPP
#define IP_SOCKET_HPP

#include <netinet/in.h>
#include <string>
#include <unistd.h>

#include "ip_module.hpp"
#include "ip_exceptions.hpp"

#include "hamcast/hamcast_logging.h"
#include "hamcast/hamcast.hpp"

// FIXME: TTL 1 restrict message to local subnet, better set 64 here?
#define DEFAULT_MULTICAST_TTL 1

namespace ipm 
{

    class ip_socket
    {
        friend class ip_instance;
    private:
        int             m_sockfd;           // socket file discriptor
        int             m_port;             // port
        struct sockaddr_storage m_if_addr;  // ip address
        unsigned int    m_if_index;         // ip interface index
        std::string     m_if_name;          // ip interface name
        unsigned char   m_ttl;              // multicast ttl
        
        /**
          * @brief Helper function for constructur
          * @deprecated Use init() instead
          * @param af IP address family
          * @return Error code
          */
        int init (const int af);

        /**
         * @brief Helper function to init IP socket
         * @return Error code
         */
        int init ();

        /**
         * @brief Get multicast ttl via getsockopt from real socket
         * @return multicast ttl of socket
         */
        unsigned int __get_multicast_ttl();

        /* Helper functions for ASM and SSM join/leave, called by
         * join/leave see above
         */
        /**
         * @brief ASM join
         * @return Error Code
         */
        int mcast_join (const struct sockaddr*, socklen_t);

        /**
         * @brief SSM join
         * @return Error Code
         */
        int mcast_join_source_group (const struct sockaddr*, socklen_t, const struct sockaddr*, socklen_t);

        /**
         * @brief ASM leave
         * @return Error Code
         */
        int mcast_leave (const struct sockaddr*, socklen_t);

        /**
         * @brief SSM leave
         * @return Error Code
         */
        int mcast_leave_source_group (const struct sockaddr*, socklen_t, const struct sockaddr*, socklen_t);
        
    public:
        
        /**
          * @brief Standard constructor
          */
        ip_socket ();

        /**
          * @brief Destroy IP socket and close file handle
          */
        ~ip_socket ()
        {
            // FIXME: not sure if this good
            //close (m_sockfd);
        }

        /**
          * @brief Standard constructor to create socket from HAMcast URI
          * @param ifuri uri for this socket
          * @param ifindex IP interface index
          * @param ifname IP interface name
          */
        ip_socket (const hamcast::uri&, const int& ifindex, const std::string& ifname);

        /**
          * @brief Standard constructor to create socket from sockaddr struct
          * @param ifaddr IP interface address
          * @param ifindex IP interface index
          * @param ifname IP interface name
          */
        ip_socket (struct sockaddr_storage&, const int& ifindex, const std::string& ifname);
        
        /**
          * @brief bind this socket to m_bind_ip:port
          * @param port Port number to bind socket
          * @throws bind_exception if bind() call returns an error
          */
        void bind(const hamcast::uri&);

        /**
          * @brief Join multicast group
          * @param group URI
          * @throws ipm_address_exception
          * @throws ipm_mcast_operation_exception
          */
        void join (const hamcast::uri&);

        /**
          * @brief Leave multicast group
          * @param group URI
          * @throws ipm_address_exception
          * @throws ipm_mcast_operation_exception
          */
        void leave (const hamcast::uri&);

        /**
          * @brief non-blocking recieve
          * @param buf Pointer to buffer 
          * @param len Length of buffer
          * @exception recv_exception will be thrown on an recv error: e.g.: buffer to small
          * @return number of bytes received or -1 if EAGAIN occured(out of data)
          */
        int recv_async(void* buf, const int& len);
        
        /**
          * @brief Send (multicast) data to group address
          * @param addr Destination IP address for data
          * @param buf Pointer to send buffer
          * @param buf_len Length of send buffer
          * @return number of bytes sent
          * @throws sendto_exception
          */
        int sendto(const struct sockaddr_storage* addr, const void *buf, const int buf_len);
    
        /**
          * @brief sets TTL for multicast packets to passed value
          * @param ttl TTL value to be used for all sockets of this module instance
          * @throws setsockopt_exception
          */
        void set_multicast_ttl(const unsigned int& ttl);
    
        /**
          * @brief gets multicast TTL value
          * @return multicast packet TTL value
          */
        inline unsigned int get_multicast_ttl()
        {
            return m_ttl;
        }

        /**
         * @brief Returns file handle of this socket
         * @return socket file handle
         */
        inline int get_sockfd ()
        {
            return m_sockfd;
        }
    };
}

#endif // IP_SOCKET_HPP
