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

#include "hamcast_logging.h"
#include "hcDiscovery/Inet_Stuff/Protocol_Infos/mc_timers_values.hpp"
#include "hcDiscovery/DiscoveryTools/mld_dis.hpp"
#include "hcDiscovery/Inet_Stuff/mc_socket.hpp"
#include "hcDiscovery/Inet_Stuff/Protocol_Infos/ip_v6_tool.hpp"
#include <netinet/icmp6.h>
#include <arpa/inet.h>

//temp
#include <iostream>
using namespace std;

mld_dis::mld_dis(){
     HC_LOG_TRACE("");
}

mld_dis::~mld_dis(){
     HC_LOG_TRACE("");
}

bool mld_dis::sniffPassiv(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     m_pcapFilter = MLD_FILTER_EXP;

     if(!initSniffer(item, ifInfo, MLD_PASSIV_TIMEOUT, &m_pcapFilter, PASSIVE, MLD_PACKET_LEN, AF_INET6, false)){
          return false;
     }

     startSniffer();

     return true;
}

bool mld_dis::sniffActive(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     mc_socket mc; //for Join and leave Group

     m_pcapFilter = MLD_FILTER_EXP;

     //start sniffing
     if (mc.create_UDP_IPv6_Socket()) { //create Socket

          if(initSniffer(item, ifInfo, MLD_AKTIVE_TIMEOUT, &m_pcapFilter, ACTIVE, MLD_PACKET_LEN, AF_INET6, false)){

               if (mc.joinGroup(MLD_IPV6_MULTICAST_TEST_GROUP_ADDR, item->ifr_name)) { //join Group

                    usleep(MLD_TIME_BETWEEN_JOIN_LEAVE_GROUP); //wait TIME_BETWEEN_JOIN_LEAVE_GROUP

                    if (mc.leaveGroup(MLD_IPV6_MULTICAST_TEST_GROUP_ADDR, item->ifr_name)) { //leave Group

                         startSniffer();
                         return true;
                    }
               }
          }
     }
     return false;
}

//MLD
bool mld_dis::ActiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET6){

          //test_output::printPacket_IPv6_IcmpInfos(ipvXHdr);

          struct ip6_hdr* ipv6Hdr= (struct ip6_hdr*) ipvXHdr;
          struct icmp6_hdr* icmpHdr = (struct icmp6_hdr*) ip_v6_tool::getSpecificHdr(ipv6Hdr,IPPROTO_ICMPV6);

          if(icmpHdr != NULL){
               struct in6_addr mldv2_router_addr;
               inet_pton(AF_INET6, MLD_IPV6_MULTICAST_TEST_GROUP_ADDR, &mldv2_router_addr);

               if(ip_v6_tool::equalAddr(&(ipv6Hdr->ip6_dst),&mldv2_router_addr)){
                    if(icmpHdr->icmp6_type == MLD_LISTENER_QUERY ){
                         //ipv6.plen- (icmpHdr-ipv6Hdr-ipv6hdr.size)
                         int querySize=ntohs(ipv6Hdr->ip6_plen)-(((char*)icmpHdr)-((char*)ipv6Hdr)- sizeof(struct ip6_hdr));
                         if(querySize==MLDv1_HDR_SIZE){
                              setProtocolType(MLDv1);
                              return true;
                         }else if(querySize>=MLD2_MIN_HDR_SIZE){
                              setProtocolType(MLDv2);
                              return true;
                         }
                    }
               }

          }
     }
     return false;
}

//MLD
bool mld_dis::PassiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET6){
          struct ip6_hdr* ipv6Hdr= (struct ip6_hdr*) ipvXHdr;
          struct icmp6_hdr* icmpHdr = (struct icmp6_hdr*) ip_v6_tool::getSpecificHdr(ipv6Hdr,IPPROTO_ICMPV6);

          if(icmpHdr != NULL){

               struct in6_addr mldv2_router_addr;
               inet_pton(AF_INET6, IPV6_ALL_NODES_ADDR, &mldv2_router_addr);

               if(ip_v6_tool::equalAddr(&(ipv6Hdr->ip6_dst),&mldv2_router_addr)){
                    if(icmpHdr->icmp6_type == MLD_LISTENER_QUERY){
                         //ipv6.plen- (icmpHdr-ipv6Hdr-ipv6hdr.size)
                         int querySize=ntohs(ipv6Hdr->ip6_plen)-(((char*)icmpHdr)-((char*)ipv6Hdr)- sizeof(struct ip6_hdr));
                         if(querySize==MLDv1_HDR_SIZE){
                              setProtocolType(MLDv1);
                              return true;
                         }else if(querySize>=MLD2_MIN_HDR_SIZE){
                              setProtocolType(MLDv2);
                              return true;
                         }
                    }
               }
          }
     }
     return false;
}
