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

#ifndef MROUTE_SOCKET_HPP
#define MROUTE_SOCKET_HPP


#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "discovery/mc_socket.hpp"

#define MROUTE_RATE_LIMIT_ENDLESS 0
#define MROUTE_TTL_THRESHOLD 1
#define MROUTE_DEFAULT_TTL 1

#define ADD_SIGNED_NUM_U16(r,a) (r)+= (a); (r)+= ((r)>>16)

class mroute_socket: public mc_socket
{
private:
     mroute_socket(const mroute_socket &copy);

     //not used
     bool create_UDP_IPv4_Socket(){
          return false;
     }

     //not used
     bool create_UDP_IPv6_Socket(){
          return false;
     }

public:
     mroute_socket();
     /**
     * @brief close datagram socket automaticly
     */
     virtual ~mroute_socket();

     /**
     * @brief create IPv4 Raw socket
     */
     bool create_RAW_IPv4_Socket();

     /**
      * @brief create IPv6 Raw socket (RFC 3542 Section 3)
      */
     bool create_RAW_IPv6_Socket();

     /**
      * @brief The IPv4 layer generates an IP header when
      * sending a packet unless the IP_HDRINCL socket option
      * is enabled on the socket. When it is enabled, the
      * packet must contain an IP header. For receiving the
      * IP header is always included in the packet.
      */
     bool set_no_ip_hdr();

     /**
      * @brief calculate an internet checksum need for IPv4 IGMP header
      */
     u_int16_t calc_checksum(const unsigned char* buf, int buf_size);

     /**
      * @brief include per default the ICMP6 checksum (RFC 3542 Section 3.1)
      */
     bool set_default_ICMP6_checksum_calc(bool enable);

     //bool add_extension_header(const unsigned char* buf, unsigned int buf_size);

     /**
      * @brief enable or disable MRT flag
      *        (sysctl net.ipv4.conf.all.mc_forwarding will be set/reset)
      */
     bool setMRouter_flag(bool enable);

     /**
      * @brief Adds the virtual interface to the mrouted API
      *        - sysctl net.ipv4.conf.eth0.mc_forwarding will be set
      *        - /proc/net$ cat ip_mr_vif displays the added interface
      *
      * @param vifNum musst the same unique number as delVIF (0 > uniqueNumber < MAXVIF ==32)
      *        ifName is physical interface name e. g. "eth0"
      *        iff_register = true if used for PIM Register encap/decap
      *
      *        following params only used for ipv4
      *        ipSrcRouting = true if tunnel uses IP src routing
      *        ipTunnel = true if vif represents a tunnel end-point
      *        ipTunnelRemoteAddr ignored if ipTunnel == false
      */
     //bool addVIF(int vifNum, const char* ifName, bool iff_register, bool ipSrcRouting, bool ipTunnel,const char* ipTunnelRemoteAddr);

     /**
      * @brief Delete the virtual interface from the mrouted API
      */
     //bool delVIF(int vifNum);

     /**
      * @brief Adds a multicast route to the kernel
      *        /proc/net$ cat ip_mr_cache display the route
      *
      * @param input_vifNum have to be the same value as in addVIF set
      *        source_addr from the receiving packet
      *        group_addr from the receiving packet
      *        output_vifNum forward to this virutal interfaces
      *
      */
     //bool addMRoute(int input_vifNum, const char* source_addr, const char* group_addr, unsigned int* output_vifNum, unsigned int output_vifNum_size);

     /**
      * @brief Delete the multicast routed '*Dp' to the kernel routes
      */
     //bool delMRoute(int input_vifNum, const char* source_addr, const char* group_addr);

     /**
      * @brief simple test outputs
      */
#define  MROUTE_SOCKET_SRC_ADDR_V4 "141.22.27.157"
#define  MROUTE_SOCKET_G_ADDR_V4 "238.99.99.99"
#define  MROUTE_SOCKET_SRC_ADDR_V6 "fe80::5e26:aff:fe23:8dc1"
#define  MROUTE_SOCKET_G_ADDR_V6 "FF02:0:0:0:99:99:99:99"

#define  MROUTE_SOCKET_IF_NUM_ONE 0
#define  MROUTE_SOCKET_IF_NUM_TWO 1
#define  MROUTE_SOCKET_IF_NUM_THREE 2
#define  MROUTE_SOCKET_IF_STR_ONE "eth0"
#define  MROUTE_SOCKET_IF_STR_TWO "wlan0"
#define  MROUTE_SOCKET_IF_STR_THREE "tun0"

};

#endif // MROUTE_SOCKET_HPP
