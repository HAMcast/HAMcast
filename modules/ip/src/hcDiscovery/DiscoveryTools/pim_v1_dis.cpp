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
#include "hcDiscovery/DiscoveryTools/pim_v1_dis.hpp"
#include "hcDiscovery/Inet_Stuff/mc_socket.hpp"

#include <netinet/igmp.h>
#include <arpa/inet.h>

pim_v1_dis::pim_v1_dis(){
     HC_LOG_TRACE("");
}

pim_v1_dis::~pim_v1_dis(){
     HC_LOG_TRACE("");
}

bool pim_v1_dis::sniffPassiv(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     m_pcapFilter = PIMv1_FILTER_EXP;

     if(!initSniffer(item, ifInfo, PIMv1_PASSIV_TIMEOUT, &m_pcapFilter, PASSIVE, PIMv1_PACKET_LEN, AF_INET, false)){
          return false;
     }

     startSniffer();

     return true;
}

bool pim_v1_dis::sniffActive(ifreq *item, if_prop* ifInfo) {
     HC_LOG_TRACE("");

     return false;
}

bool pim_v1_dis::ActiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     return false;
}

bool pim_v1_dis::PassiveFilter(const u_char* ipvXHdr) {
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET){
          struct ip* ipv4Hdr= (struct ip*)ipvXHdr;
          if (ipv4Hdr->ip_p == IPPROTO_IGMP) {
               struct igmp *igmpHdr = (struct igmp*) ((char*)ipv4Hdr + ipv4Hdr->ip_hl * 4);

               if (ipv4Hdr->ip_dst.s_addr == inet_addr(IPV4_ALL_IGMP_ROUTERS_ADDR)) {
                    if (igmpHdr->igmp_type == IGMP_PIM) {
                         setProtocolType(PIMv1);
                         return true;
                    }
               }
          }
     }

     return false;
}
