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
#include "hcDiscovery/DiscoveryTools/pim_v2_dis.hpp"
#include "hcDiscovery/Inet_Stuff/Protocol_Infos/pim.h"
#include "hcDiscovery/Inet_Stuff/Protocol_Infos/ip_v6_tool.hpp"
#include "hcDiscovery/Inet_Stuff/mc_socket.hpp"
#include <arpa/inet.h>

pim_v2_dis::pim_v2_dis(){
     HC_LOG_TRACE("");
}

pim_v2_dis::~pim_v2_dis(){
     HC_LOG_TRACE("");
}

bool pim_v2_dis::sniffPassiv(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     m_pcapFilter = PIMv2_FILTER_EXP;

     if(!initSniffer(item, ifInfo, PIMv2_PASSIV_TIMEOUT, &m_pcapFilter, PASSIVE, PIMv2_PACKET_LEN, AF_INET6, false)){
          return false;
     }

     startSniffer();

     return true;
}

bool pim_v2_dis::sniffActive(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");
     return false;
}

bool pim_v2_dis::ActiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");
     return false;
}

bool pim_v2_dis::PassiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET6){

          struct ip6_hdr* ipv6Hdr= (struct ip6_hdr*)ipvXHdr;
          struct pim* pimHdr = ( struct pim*) ip_v6_tool::getSpecificHdr(ipv6Hdr,IPPROTO_PIM);

          if(pimHdr != NULL){

               struct in6_addr all_pim_router_addr;
               inet_pton(AF_INET6, IPV6_ALL_PIM_ROUTERS, &all_pim_router_addr);

               if(ip_v6_tool::equalAddr(&(ipv6Hdr->ip6_dst),&all_pim_router_addr)){
                    if(pimHdr->pim_vers == 2){
                         if(pimHdr->pim_type == PIM_HELLO){

                              setProtocolType(PIMv2);
                              return true;
                         }
                    }
               }
          }
     }
     return false;
}
