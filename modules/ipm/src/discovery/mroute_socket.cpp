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
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <errno.h>
#ifdef __APPLE__
 #include <netinet/ip_mroute.h>
 #include <netinet6/ip6_mroute.h>
#else
 #include <linux/mroute.h>
 #include <linux/mroute6.h>
#endif
#include <string>
#include <cstring>
#include <unistd.h>

#include "hamcast/hamcast_logging.h"
#include "discovery/mroute_socket.hpp"

using std::string;

mroute_socket::mroute_socket(){
     HC_LOG_TRACE("");
}


bool mroute_socket::create_RAW_IPv4_Socket(){
     HC_LOG_TRACE("");

     if (is_udp_valid()) {
          close(m_sock);
     }

     //			IP-Protokollv4, UDP,	Protokoll
     m_sock = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);
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

bool mroute_socket::create_RAW_IPv6_Socket(){
     HC_LOG_TRACE("");

     if (is_udp_valid()) {
          close(m_sock);
     }

     //			IP-Protokollv6, UDP,	Protokoll
     m_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6); //SOCK_DGRAM //IPPROTO_IP
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

bool mroute_socket::set_no_ip_hdr(){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int proto;
     if(m_addrFamily == AF_INET){
          proto = IPPROTO_IP;
     }else if(m_addrFamily == AF_INET6){
          proto =IPPROTO_IPV6;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }
     int one = 1;
     if (setsockopt (m_sock, proto, IP_HDRINCL, &one, sizeof (one)) < 0){
          HC_LOG_ERROR("failed to set no ip header! Error: " << strerror(errno));
     }

     return true;
}

u_int16_t mroute_socket::calc_checksum(const unsigned char* buf, int buf_size){
     HC_LOG_TRACE("");

     u_int16_t* b=(u_int16_t*)buf;
     int sum=0;

     for(int i=0; i<buf_size/2;i++){
          ADD_SIGNED_NUM_U16(sum,b[i]);
          //sum +=b[i];
     }

     if(buf_size%2==1){
          //sum += buf[buf_size-1];
          ADD_SIGNED_NUM_U16(sum,buf[buf_size-1]);
     }

     return ~sum;
}

bool mroute_socket::set_default_ICMP6_checksum_calc(bool enable){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     if(m_addrFamily == AF_INET){
          HC_LOG_ERROR("this funktion is only available vor IPv6 sockets ");
          return false;
     }else if(m_addrFamily == AF_INET6){
          int offset = enable? 2 : -1;
          if (setsockopt (m_sock, IPPROTO_IPV6, IP_HDRINCL, &offset, sizeof (offset)) < 0){
               HC_LOG_ERROR("failed to set default ICMP6 checksum! Error: " << strerror(errno));
          }

          return true;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

}

#if 0
bool mroute_socket::add_extension_header(const unsigned char* buf, unsigned int buf_size){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     if(m_addrFamily == AF_INET){
          HC_LOG_ERROR("this funktion is only available vor IPv6 sockets ");
          return false;
     }else if(m_addrFamily == AF_INET6){
          int rc= setsockopt(m_sock,IPPROTO_IPV6, IPV6_HOPOPTS, buf, buf_size);

          if(rc == -1){
               HC_LOG_ERROR("failed to add extension header! Error: " << strerror(errno));
               return false;
          }else{
               return true;
          }
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }


     //#######################################
/*     struct icmp6_filter myfilter;

     ICMP6_FILTER_SETPASSALL(&myfilter);

     rc = setsockopt(m_sock,IPPROTO_ICMPV6,ICMP6_FILTER, &myfilter,sizeof(myfilter));

     if(rc == -1){
          HC_LOG_ERROR("failed to add extension header! Error: " << strerror(errno));
          return false;
     }else{
          return true;
     }
*/
}
#endif

bool mroute_socket::setMRouter_flag(bool enable){
     HC_LOG_TRACE("enable: " << enable);

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int rc;
     int proto;
     int mrt_cmd;

     if(enable){
          if(m_addrFamily == AF_INET){
               proto = IPPROTO_IP;
               mrt_cmd = MRT_INIT;
          }else if(m_addrFamily == AF_INET6){
               proto = IPPROTO_IPV6;
               mrt_cmd = MRT6_INIT;
          }else{
               HC_LOG_ERROR("wrong address family");
               return false;
          }

          int val=1;
          rc = setsockopt(m_sock,proto, mrt_cmd, (void*)&val, sizeof(val));

          if(rc == -1){
               HC_LOG_ERROR("failed to set MRT flag! Error: " << strerror(errno));
               return false;
          }else{
               return true;
          }
     }else{
          if(m_addrFamily == AF_INET){
               proto = IPPROTO_IP;
               mrt_cmd = MRT_DONE;
          }else if(m_addrFamily == AF_INET6){
               proto = IPPROTO_IPV6;
               mrt_cmd = MRT6_DONE;
          }else{
               HC_LOG_ERROR("wrong address family");
               return false;
          }

          rc = setsockopt(m_sock,proto, mrt_cmd, NULL, 0);

          if(rc == -1){
               HC_LOG_ERROR("failed to reset MRT flag! Error: " << strerror(errno));
               return false;
          }else{
               return true;
          }
     }
}

//vifNum musst the same uniqueName  on delVIF (0 > vifNum < MAXVIF ==32)
//iff_register = true if used for PIM Register encap/decap
//
//following params only used for ipv4
//ipSrcRouting = true if tunnel uses IP src routing
//ipTunnel = true if vif represents a tunnel end-point
//ipTunnelRemoteAddr ignored if ipTunnel == false
#if 0
bool mroute_socket::addVIF(int vifNum, const char* ifName, bool iff_register, bool ipSrcRouting, bool ipTunnel,const char* ipTunnelRemoteAddr){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int rc;

     if(m_addrFamily == AF_INET){
          struct vifctl vc;
          vifi_t index=if_nametoindex(ifName);

          //VIFF_TUNNEL   /* vif represents a tunnel end-point */
          //VIFF_SRCRT    /* tunnel uses IP src routing */
          //VIFF_REGISTER /* used for PIM Register encap/decap */
          unsigned char flags;
          flags = ((ipTunnel)? VIFF_TUNNEL: 0) | ((ipSrcRouting)? VIFF_SRCRT: 0) | ((iff_register)? VIFF_REGISTER: 0) | VIFF_USE_IFINDEX;

          memset(&vc, 0, sizeof(vc));
          vc.vifc_vifi = vifNum;
          vc.vifc_flags = flags;
          vc.vifc_threshold = MROUTE_TTL_THRESHOLD;
          vc.vifc_rate_limit = MROUTE_RATE_LIMIT_ENDLESS;
          vc.vifc_lcl_ifindex =index;

          if(ipTunnel){
               if(!inet_pton(AF_INET, ipTunnelRemoteAddr, (void*)&vc.vifc_rmt_addr)>0){
                    HC_LOG_ERROR("cannot convert ipTunnelRemoteAddr: " << ipTunnelRemoteAddr);
               }
          }

          rc = setsockopt(m_sock,IPPROTO_IP,MRT_ADD_VIF,(void *)&vc,sizeof(vc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to add VIF! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }

     }else if(m_addrFamily == AF_INET6){
          struct mif6ctl mc;
          mifi_t index=if_nametoindex(ifName);

          unsigned char flags;
          flags = ((iff_register)? MIFF_REGISTER: 0) ;

          memset(&mc, 0, sizeof(mc));
          mc.mif6c_mifi = vifNum;
          mc.mif6c_flags = flags;
          mc.vifc_rate_limit = MROUTE_RATE_LIMIT_ENDLESS;
          mc.vifc_threshold = MROUTE_TTL_THRESHOLD;
          mc.mif6c_pifi = index;

          rc = setsockopt(m_sock, IPPROTO_IPV6, MRT6_ADD_MIF, (void *)&mc,sizeof(mc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to add VIF! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }

     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }
}

bool mroute_socket::delVIF(int vifNum){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int rc;

     if(m_addrFamily == AF_INET){
          struct vifctl vifc;
          memset(&vifc, 0, sizeof(vifc));

          vifc.vifc_vifi= vifNum;
          rc = setsockopt(m_sock, IPPROTO_IP, MRT_DEL_VIF, (char *)&vifc, sizeof(vifc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to del VIF! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }
     }else if(m_addrFamily == AF_INET6){
          struct mif6ctl mc;
          memset(&mc, 0, sizeof(mc));

          mc.mif6c_mifi = vifNum;
          rc = setsockopt(m_sock, IPPROTO_IPV6, MRT6_DEL_MIF, (char *)&mc, sizeof(mc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to del VIF! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }
}

//source_addr is the source address of the received multicast packet
//group_addr group address of the received multicast packet
bool mroute_socket::addMRoute(int input_vifNum, const char* source_addr, const char* group_addr, unsigned int* output_vifTTL, unsigned int output_vifTTL_Ncount){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int rc;

     if(m_addrFamily == AF_INET){
          struct mfcctl mc;
          memset(&mc, 0, sizeof(mc));


          if(!inet_pton(m_addrFamily, source_addr, &mc.mfcc_origin)>0){
               HC_LOG_ERROR("cannot convert source_addr: " << source_addr);
               return false;
          }

          if(!inet_pton(m_addrFamily, group_addr, &mc.mfcc_mcastgrp)>0){
               HC_LOG_ERROR("cannot convert group_addr: " << group_addr);
               return false;
          }

          mc.mfcc_parent = input_vifNum;

          if(output_vifTTL_Ncount >= MAXVIFS){
               HC_LOG_ERROR("output_vifNum_size to large: " << output_vifTTL_Ncount);
               return false;
          }

          for (unsigned int i = 0; i < output_vifTTL_Ncount; i++){
               mc.mfcc_ttls[output_vifTTL[i]] = MROUTE_DEFAULT_TTL;
          }

          rc = setsockopt(m_sock, IPPROTO_IP, MRT_ADD_MFC,(void *)&mc, sizeof(mc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to add multicast route! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }

     }else if(m_addrFamily == AF_INET6){
          struct mf6cctl mc;
          memset(&mc, 0, sizeof(mc));

          if(!inet_pton(m_addrFamily, source_addr, &mc.mf6cc_origin.sin6_addr)>0){
               HC_LOG_ERROR("cannot convert source_addr: " << source_addr);
               return false;
          }

          if(!inet_pton(m_addrFamily, group_addr, &mc.mf6cc_mcastgrp.sin6_addr)>0){
               HC_LOG_ERROR("cannot convert group_addr: " << group_addr);
               return false;
          }

          if(output_vifTTL_Ncount >= MAXMIFS){
               HC_LOG_ERROR("output_vifNum_size to large: " << output_vifTTL_Ncount);
               return false;
          }

          for (unsigned int i = 0; i < output_vifTTL_Ncount; i++){
               if(output_vifTTL[i] > 0){
                    IF_SET(output_vifTTL[i],&mc.mf6cc_ifset);
               }
          }

          //print_struct_mf6cctl(&mc);

          rc = setsockopt(m_sock, IPPROTO_IPV6, MRT6_ADD_MFC,(void *)&mc, sizeof(mc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to add multicast route! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }

     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

}

bool mroute_socket::delMRoute(int input_vifNum, const char* source_addr, const char* group_addr){
     HC_LOG_TRACE("");

     if (!is_udp_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int rc;

     if(m_addrFamily == AF_INET){
          struct mfcctl mc;
          memset(&mc, 0, sizeof(mc));

          if(!inet_pton(m_addrFamily, source_addr, &mc.mfcc_origin)>0){
               HC_LOG_ERROR("cannot convert source_addr: " << source_addr);
               return false;
          }

          if(!inet_pton(m_addrFamily, group_addr, &mc.mfcc_mcastgrp)>0){
               HC_LOG_ERROR("cannot convert group_addr: " << group_addr);
               return false;
          }

          mc.mfcc_parent = input_vifNum;

          rc = setsockopt(m_sock, IPPROTO_IP, MRT_DEL_MFC,(void *)&mc, sizeof(mc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to add multicast route! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }

     }else if(m_addrFamily == AF_INET6){
          struct mf6cctl mc;
          memset(&mc, 0, sizeof(mc));

          if(!inet_pton(m_addrFamily, source_addr, &mc.mf6cc_origin.sin6_addr)>0){
               HC_LOG_ERROR("cannot convert source_addr: " << source_addr);
               return false;
          }

          if(!inet_pton(m_addrFamily, group_addr, &mc.mf6cc_mcastgrp.sin6_addr)>0){
               HC_LOG_ERROR("cannot convert group_addr: " << group_addr);
               return false;
          }

          mc.mf6cc_parent = input_vifNum;

          rc = setsockopt(m_sock, IPPROTO_IPV6, MRT6_DEL_MFC,(void *)&mc, sizeof(mc));
          if (rc == -1) {
               HC_LOG_ERROR("failed to add multicast route! Error: " << strerror(errno));
               return false;
          } else {
               return true;
          }
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }
     return false;
}
#endif
mroute_socket::~mroute_socket() {
     HC_LOG_TRACE("");
}
