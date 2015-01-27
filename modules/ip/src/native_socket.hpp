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

#ifndef NATIVE_SOCKET_HPP
#define NATIVE_SOCKET_HPP

#include <map>
#include <netinet/in.h>
#include <string>

#include "defines.hpp"
#include "ipmodule_exceptions.hpp"

namespace ip_module
{
    /**
      * @author Fabian Hollfer <hamcast (at) fholler.de>
      * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
      *
      * @brief native socket wrapper for IPv4/IPv6 module 
      */
class native_socket {
    
    int m_socketfd;
    const std::string m_bind_ip;
    const unsigned int m_ifindex;
    int multicast_ttl;

    /**
     * @brief calls setsockopt to get multicast TTL value
     * @throws setsockopt_exception
     * @return multicast packet TTL value
     */
    int getsockopt_multicast_ttl();

    /**
      * @brief Excecute/set socket option for a group address, this means join or leave
      * @param addr Group address to be affected
      * @param optname Socket option value
      * @throws setsockopt_exception
      * @throws inet_pton_exception
     */
    void mcast_group_operation(const struct sockaddr_storage* addr, const int& optname);

    /**
     * @throws setsockopt_exception
     */
    void ssm_group_operation(const struct sockaddr_storage* groupaddr, const struct sockaddr_storage* ssm_src, const int& optname);

    /**
     * @brief set the interface for outgoing multicast packets to m_ifindex
     */
    void set_outgoing_mcast_if();

    // copy operations aren't allowed, else problem occurs with the destructor: 
    // eg socket will be copied, destructor of original socket will be called 
    // => closes also IP-Socket of the still existent IP-Socket
    native_socket(const native_socket&);
    native_socket& operator=(const native_socket&);

    /**
     * @brief creates new UDP socket and sets SO_REUSEADDR
     * @throws socket_create_exception if socket() call returns an error
     */
    void init(); 

 public:

    /**
      * @brief calls init() 
      */
    native_socket();

    /**
      * @brief calls init() 
      * @param bind_ip IP address of interface to bind this module instance to
      */
    native_socket(const std::string& bind_ip);

    ~native_socket();

    /**
      * @brief Returns the filedescriptor of send socket
      * @return socket filedescriptor
      */
    inline const int get_socketfd() const{ return m_socketfd; }

    /**
      * @brief bind this socket to m_bind_ip:port
      * @param port Port number to bind socket
      * @throws bind_exception if bind() call returns an error
      */
    void bind(const int& port);

    /**
      * @brief subscribes to the multicast group
      * @param group Multicast address of group to join
      */
    void join(const struct sockaddr_storage* group);

    /**
      * @brief leaves the multicast group
      * @param group Multicast address of group to leave
      */
    void leave(const struct sockaddr_storage* group);

    /**
      * @brief SSM-subscribe on group and sender ssm_src
      * @param group Multicast address of group to join
      * @param ssm_src Source address for SSM join
      * @exception not_implemented_exception will be thrown on OSX, OSX doesnt support ssm join operation
      */
    void join(const struct sockaddr_storage* group, const struct sockaddr_storage* ssm_src);

    /**
      * @brief SSM-leave on group and sender ssm_src
      * @param group Multicast address of group to leave
      * @param ssm_src Source address for SSM leave
      * @exception not_implemented_exception will be thrown on OSX, OSX doesnt support ssm join operation
      */
    void leave(const struct sockaddr_storage* group, const struct sockaddr_storage* ssm_src);

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
    int sendto(const struct sockaddr_storage* addr, const void *buf, const int& buf_len);

    /**
      * @brief sets TTL for multicast packets to passed value
      * @param ttl TTL value to be used for all sockets of this module instance
      * @throws setsockopt_exception
      */
    void set_multicast_ttl(const int ttl);

    /**
      * @brief gets multicast TTL value
      * @return multicast packet TTL value
      */
    inline unsigned int get_multicast_ttl() { return multicast_ttl; };
};

}
#endif
