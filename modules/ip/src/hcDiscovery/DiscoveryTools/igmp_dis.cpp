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

#include "hcDiscovery/hamcast_logging.h"
#include "hcDiscovery/DiscoveryTools/igmp_dis.hpp"
#include "hcDiscovery/Inet_Stuff/mc_socket.hpp"
#include <arpa/inet.h>
#include <netinet/igmp.h>

//temp
#include <iostream>
using namespace std;

igmp_dis::igmp_dis(){
     HC_LOG_TRACE("");
}

igmp_dis::~igmp_dis() {
     HC_LOG_TRACE("");
}


bool igmp_dis::sniffPassiv(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     m_pcapFilter = IGMP_FILTER_EXP;

     if(!initSniffer(item, ifInfo, IGMP_PASSIV_TIMEOUT, &m_pcapFilter, PASSIVE, IGMP_PACKET_LEN, AF_INET, false)){
          return false;
     }

     startSniffer();

     return true;
}

bool igmp_dis::sniffActive(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     mc_socket mc; //for Join and leave Group

     m_pcapFilter = IGMP_FILTER_EXP;

     //Interface name to Interface address need for join/leave group
     char* ifaddr;
     ifInfo->getAddr(item);
     ifaddr = inet_ntoa(((struct sockaddr_in *) &item->ifr_addr)->sin_addr);

     //start sniffing
     if (mc.create_UDP_IPv4_Socket()) { //create Socket

          if(initSniffer(item, ifInfo, IGMP_AKTIVE_TIMEOUT, &m_pcapFilter, ACTIVE, IGMP_PACKET_LEN, AF_INET, false)){

               if (mc.joinGroup(IGMP_IPV4_MULTICAST_TEST_GROUP_ADDR, ifaddr)) { //join Group

                    usleep(IGMP_TIME_BETWEEN_JOIN_LEAVE_GROUP); //wait TIME_BETWEEN_JOIN_LEAVE_GROUP

                    if (mc.leaveGroup(IGMP_IPV4_MULTICAST_TEST_GROUP_ADDR, ifaddr)) { //leave Group

                         startSniffer();
                         return true;

                    }
               }
          }
     }
     return false;
}


bool igmp_dis::ActiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET){

          struct ip* ipv4Hdr= (struct ip*) ipvXHdr;

          if (ipv4Hdr->ip_p == IPPROTO_IGMP) {

               struct igmp *igmpHdr = (struct igmp*) ((char*)ipv4Hdr + ipv4Hdr->ip_hl * 4);

               if (ipv4Hdr->ip_dst.s_addr == inet_addr(IGMP_IPV4_MULTICAST_TEST_GROUP_ADDR)) {
                    if(igmpHdr->igmp_type == IGMP_MEMBERSHIP_QUERY ){
                         if((ntohs(ipv4Hdr->ip_len) - ipv4Hdr->ip_hl*4) > IGMP_MINLEN){ //IGMPv3 gefunde
                              setProtocolType(IGMPv3);
                              return true;
                         }else if((ntohs(ipv4Hdr->ip_len) - ipv4Hdr->ip_hl*4) == IGMP_MINLEN){ //IGMPv2 gefunden
                              setProtocolType(IGMPv2);
                              return true;
                         }

                    }
               }
          }
     }

     return false;
}

bool igmp_dis::PassiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET){

          struct ip* ipv4Hdr= (struct ip*) ipvXHdr;

          if (ipv4Hdr->ip_p == IPPROTO_IGMP) {

               struct igmp *igmpHdr = (struct igmp*) ((char*)ipv4Hdr + ipv4Hdr->ip_hl * 4);

               if (ipv4Hdr->ip_dst.s_addr == inet_addr(IPV4_ALL_HOST_ADDR)) {
                    if (igmpHdr->igmp_type == IGMP_MEMBERSHIP_QUERY) { //IGMPv3 gefunden
                         setProtocolType(IGMPv3);
                         return true;
                    }else if (igmpHdr->igmp_type == IGMP_V2_MEMBERSHIP_REPORT) { //IGMPv2 gefunden
                         setProtocolType(IGMPv2);
                         return true;
                    }
               }
          }
     }
     return false;
}
