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

#include <cstring> //memset
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <unistd.h>

#include "hamcast/hamcast_logging.h"
#include "discovery/mc_socket.hpp"

using std::string;


mc_socket::mc_socket() :
          m_sock(0), m_addrFamily(-1), m_ifIndex(MC_SCOKET_IF_CHOOSE_INIT), m_own_socket(true) {
     HC_LOG_TRACE("");
}

bool mc_socket::create_UDP_IPv4_Socket() {
     HC_LOG_TRACE("");

     if (is_udp_valid()) {
          close(m_sock);
     }

     //			IP-Protokollv4, UDP,	Protokoll
     m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP); //SOCK_DGRAM //IPPROTO_IP
     if (m_sock < 0) {
          HC_LOG_ERROR("failed to create! Error: " << strerror(errno));
          return false; // failed
     } else {
          HC_LOG_DEBUG("get socket discriptor number: " << m_sock);
          m_addrFamily = AF_INET;
          m_own_socket = true;
          return true;
     }

}

bool mc_socket::create_UDP_IPv6_Socket() {
     HC_LOG_TRACE("");

     if (is_udp_valid()) {
          close(m_sock);
     }

     //			IP-Protokollv6, UDP,	Protokoll
     m_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP); //SOCK_DGRAM //IPPROTO_IP
     if (m_sock < 0) {
          HC_LOG_ERROR("failed to create! Error: " << strerror(errno));
          return false; // failed
     } else {
          HC_LOG_DEBUG("get socket discriptor number: " << m_sock);
          m_addrFamily = AF_INET6;
          m_own_socket = true;
          return true;
     }
}

bool mc_socket::set_own_socket(int sck, int addr_family){
     HC_LOG_TRACE("");

     if (is_udp_valid()) {
          close(m_sock);
     }

     if (sck < 0) {
          HC_LOG_ERROR("wrong socket discriptor! socket: " << sck);
          return false; // failed
     } else {
          if(addr_family == AF_INET || addr_family == AF_INET6){
               m_sock= sck;
               m_addrFamily = addr_family;
               m_own_socket = false;
          }else{
               HC_LOG_ERROR("wrong address family: " << addr_family);
               return false; // failed
          }
          return true;
     }
}

int mc_socket::get_addr_family(){
     return m_addrFamily;
}

bool mc_socket::bind_UDP_Socket(int port) {
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }

     //struct sockaddr_storage tmp;
     struct sockaddr* m_addr;
     struct sockaddr_in m_addr_v4;
     struct sockaddr_in6 m_addr_v6;
     int size;
     int rc;

     if(m_addrFamily==AF_INET){
          m_addr_v4.sin_family = AF_INET;
          m_addr_v4.sin_addr.s_addr = INADDR_ANY;
          m_addr_v4.sin_port = htons(port);
          m_addr = (sockaddr*) &m_addr_v4;
          size = sizeof(m_addr_v4);
     }else if(m_addrFamily==AF_INET6){
          m_addr_v6.sin6_family = AF_INET6;
          m_addr_v6.sin6_flowinfo = 0;
          m_addr_v6.sin6_port =  htons(port);
          m_addr_v6.sin6_addr = in6addr_any;
          m_addr = (sockaddr*) &m_addr_v6;
          size = sizeof(m_addr_v6);
     }else{
          HC_LOG_ERROR("Unknown Errno");
          return false;
     }

     rc = bind(m_sock, m_addr, size);
     if (rc == -1) {
          HC_LOG_ERROR("failed to bind! Error: " << strerror(errno));
          return false;
     } else {
          HC_LOG_DEBUG("bind to port: " << port);
          return true;
     }
}

bool mc_socket::setLoopBack(bool enable) {
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }

     int rc;
     int loopArg;
     int level;

     //u_char loop;
     int loop;
     if (enable == true) {
          loop = 1;
     } else {
          loop = 0;
     }

     if(m_addrFamily == AF_INET){
          level = IPPROTO_IP;
          loopArg = IP_MULTICAST_LOOP;
     }else if(m_addrFamily == AF_INET6){
          level = IPPROTO_IPV6;
          loopArg = IPV6_MULTICAST_LOOP;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

     rc = setsockopt(m_sock, level, loopArg, &loop, sizeof(loop));

     if (rc == -1) {
          HC_LOG_ERROR("failed to setLoopBack(on/off)! Error: " << strerror(errno));
          return false;
     } else {
          return true;
     }
}

bool mc_socket::send_Packet(const char* addr, int port, string data){
     return send_Packet(addr,port, (unsigned char*)data.c_str(),data.size());
}

bool mc_socket::send_Packet(const char* addr, int port, const unsigned char* data, unsigned int data_size) {
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }

     //struct sockaddr_storage tmp;
     struct sockaddr* addr_sendto;
     struct sockaddr_in addr_sendto_v4;
     struct sockaddr_in6 addr_sendto_v6;

     int rc;
     int size;

     if(m_addrFamily == AF_INET){
          //fill sockadrr_in
          addr_sendto_v4.sin_family = AF_INET;
          if(!inet_pton(AF_INET, addr,(void*)&addr_sendto_v4.sin_addr)>0){
               HC_LOG_ERROR("cannot convert addr");
          }

          //extern int inet_pton (int __af, __const char *__restrict __cp, void *__restrict __buf)
          //memcpy(&addr_sendto_v4.sin_addr.s_addr, h->h_addr_list[0], h->h_length); //addr_sento.addr = h.addr;
          addr_sendto_v4.sin_port = htons(port);

          addr_sendto = (sockaddr*) &addr_sendto_v4;
          size = sizeof(addr_sendto_v4);
     }else if(m_addrFamily == AF_INET6){
          //fill sockadrr_in6
          addr_sendto_v6.sin6_family = AF_INET6;
          if(!inet_pton(AF_INET6,addr, (void*)&addr_sendto_v6.sin6_addr)>0){
               HC_LOG_ERROR("cannot convert addr");
               return false;
          }
          //memcpy(&addr_sendto_v6.sin6_addr, h->h_addr_list[0], h->h_length); //addr_sento.addr = h.addr;
          addr_sendto_v6.sin6_port = htons(port);

          if(m_ifIndex!=MC_SCOKET_IF_CHOOSE_INIT){
               addr_sendto_v6.sin6_scope_id = m_ifIndex;
          }

          addr_sendto = (sockaddr*) &addr_sendto_v6;
          size = sizeof(addr_sendto_v6);
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

     //send
     rc = sendto(m_sock, data, data_size, 0, addr_sendto, size);

     if (rc == -1) {
          HC_LOG_ERROR("failed to send! Error: " << strerror(errno));
          return false; //failed to send
     } else {
          return true;
     }
}

bool mc_socket::receive_Packet(unsigned char* buf, int sizeOfBuf, int &sizeOfInfo) {
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }

     int rc;
     rc = recv(m_sock, buf, sizeOfBuf, 0);
     sizeOfInfo = rc;
     if (rc == -1) {
          if(errno == EAGAIN || errno == EWOULDBLOCK){
               sizeOfInfo = 0;
               return true;
          }else{
               HC_LOG_ERROR("failed to receive Error: " << strerror(errno));
               return false;
          }
     } else {
          return true;
     }
}

bool mc_socket::set_receive_timeout(long msec){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }

     struct timeval t;
     t.tv_sec = msec/1000;
     t.tv_usec = 1000 * (msec % 1000);;

     int proto;
     if(m_addrFamily == AF_INET){
          proto = IPPROTO_IP;
     }else if(m_addrFamily == AF_INET6){
          proto = IPPROTO_IPV6;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

     int rc= setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));

     if (rc == -1) {
          HC_LOG_ERROR("failed to set timeout! Error: " << strerror(errno));
          return false;
     } else {
          return true;
     }
}

//!! interface: IPv4 ==> InterfaceIpAddress , IPv6 ==> InterfaceName
bool mc_socket::chooseIf(const char* interface){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }

     if(m_addrFamily == AF_INET){
          struct in_addr interface_addr_v4;

          if(inet_pton (m_addrFamily, interface, &interface_addr_v4) < 1){
               HC_LOG_ERROR("MCSocket::chooseIf: failed to convert ifaddr: " << interface);
               return false;
          }

          int rc= setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_IF, &interface_addr_v4, sizeof(struct in_addr));
          if (rc == -1) {
               HC_LOG_ERROR("failed to choose Interface: " << interface<< "! Error: " << strerror(errno));
               return false;
          } else {
               m_ifIndex = if_nametoindex(interface);
               return true;
          }
     }else if(m_addrFamily == AF_INET6){
          m_ifIndex = if_nametoindex(interface);
          return true;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }
}

//!! interface: IPv4 ==> InterfaceIpAddress , IPv6 ==> InterfaceName
bool mc_socket::joinGroup(const char* addr, const char* interface) {
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }else{
          HC_LOG_DEBUG("use socket discriptor number: " << m_sock);
     }
     int rc;

     void* imr;
     int level;
     int joinArg;
     struct ip_mreq imr_v4; //multicast group information

     struct ipv6_mreq imr_v6;
     int size;

     if(m_addrFamily == AF_INET){
          level = IPPROTO_IP;
          joinArg = IP_ADD_MEMBERSHIP;

          if((imr_v4.imr_multiaddr.s_addr = inet_addr(addr)) == INADDR_NONE){ /* group addr */
               HC_LOG_ERROR("failed to convert IPv4 addr: " << addr);
               return false;
          }

          if((imr_v4.imr_interface.s_addr = inet_addr(interface)) == INADDR_NONE){ /* use default */
               HC_LOG_ERROR("failed to convert IPv4 ifaddr: " << interface);
               return false;
          }

          imr = &imr_v4;
          size= sizeof(imr_v4);
     }else if(m_addrFamily == AF_INET6){

          level = IPPROTO_IPV6;
          joinArg = IPV6_JOIN_GROUP;

          if(inet_pton(m_addrFamily, addr, &imr_v6.ipv6mr_multiaddr) < 1){
               HC_LOG_ERROR("failed to convert IPv6 addr: " << addr);
               return false;
          }

          if((imr_v6.ipv6mr_interface = if_nametoindex(interface)) == 0){
               HC_LOG_ERROR("failed to convert IPv6 ifname: " << interface);
               return false;
          }

          imr = &imr_v6;
          size = sizeof(imr_v6);
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

     rc = setsockopt(m_sock, level, joinArg, imr,size);

     if (rc == -1) {
          HC_LOG_ERROR("failed to join! Error: " << strerror(errno));
          return false;
     } else {
          return true;
     }
}

//!! interface: IPv4 ==> InterfaceIpAddress , IPv6 ==> InterfaceName
bool mc_socket::leaveGroup(const char* addr, const char* interface) {
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("udp_socket invalid");
          return false;
     }else{
          HC_LOG_DEBUG("use socket discriptor number: " << m_sock);
     }
     int rc;

     void* imr;
     int level;
     int leaveArg;
     struct ip_mreq imr_v4; //multicast group information

     struct ipv6_mreq imr_v6;
     int size;

     if(m_addrFamily == AF_INET){
          level = IPPROTO_IP;
          leaveArg = IP_DROP_MEMBERSHIP;

          if((imr_v4.imr_multiaddr.s_addr = inet_addr(addr)) == INADDR_NONE){ /* group addr */
               HC_LOG_ERROR("failed to convert IPv4 addr: " << addr);
               return false;
          }

          if((imr_v4.imr_interface.s_addr = inet_addr(interface)) == INADDR_NONE){ /* use default */
               HC_LOG_ERROR("failed to convert IPv4 ifaddr: " << interface);
               return false;
          }

          imr = &imr_v4;
          size= sizeof(imr_v4);
     }else if(m_addrFamily == AF_INET6){
          level = IPPROTO_IPV6;
          leaveArg = IPV6_LEAVE_GROUP;

          if(inet_pton(m_addrFamily, addr, &imr_v6.ipv6mr_multiaddr) < 1){
               HC_LOG_ERROR("failed to convert IPv6 addr:" << addr);
               return false;
          }

          if((imr_v6.ipv6mr_interface = if_nametoindex(interface)) == 0){
               HC_LOG_ERROR("failed to convert IPv6 ifname: " << interface);
               return false;
          }

          imr = &imr_v6;
          size = sizeof(imr_v6);
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

     rc = setsockopt(m_sock, level, leaveArg, imr,size);

     if (rc == -1) {
          HC_LOG_ERROR("failed to leave! Error: " << strerror(errno));
          return false;
     } else {
          return true;
     }
}

mc_socket::~mc_socket() {
     HC_LOG_TRACE("");

     if (is_udp_valid() && m_own_socket) {
          close(m_sock);
     }
}
