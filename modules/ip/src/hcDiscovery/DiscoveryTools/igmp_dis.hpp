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

#ifndef BASIC_IGMP_DISCOVERY_H_
#define BASIC_IGMP_DISCOVERY_H_

#include "hcDiscovery/DiscoveryTools/basic_dis.hpp"
#include "hcDiscovery/Inet_Stuff/Protocol_Infos/mc_timers_values.hpp"
#include "hcDiscovery/Inet_Stuff/if_prop.hpp"

///@author Sebastian Woelke
///@brief discover IGMPv2/v3 on a spezific network interface

#define LOGMSG 1
#define IGMP_PACKET_LEN 100
#define IGMP_FILTER_EXP "igmp" //Pcap Filter expression 2= igmp
#define IGMP_IPV4_MULTICAST_TEST_GROUP_ADDR "238.99.99.99"

#define IGMP_SAFETY_TIME 2 //sec
#define IGMP_PASSIV_TIMEOUT (MC_TV_QUERY_INTERVAL*MC_TV_ROBUSTNESS_VARIABLE + IGMP_SAFETY_TIME) //sec ( 2*125sec + 2)
#define IGMP_AKTIVE_TIMEOUT (MC_TV_LAST_MEMBER_QUERY_INTEVAL * MC_TV_ROBUSTNESS_VARIABLE) //Last Member Query Interval (1 sec) * Robustness Variable (2)
#define IGMP_TIME_BETWEEN_JOIN_LEAVE_GROUP 1000000 //usec


class igmp_dis: public basic_dis {
private:
     string m_pcapFilter;
protected:
     bool ActiveFilter(const u_char* ipvXHdr);
     bool PassiveFilter(const u_char* ipvXHdr);
public:
     igmp_dis();
     virtual ~igmp_dis();

     bool sniffPassiv(ifreq *item, if_prop* ifInfo);
     bool sniffActive(ifreq *item, if_prop* ifInfo);
};

#endif /* BASIC_IGMP_DISCOVERY_H_ */
