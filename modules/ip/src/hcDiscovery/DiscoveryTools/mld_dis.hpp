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

#ifndef BASIC_MLD_DISCOVERY_H
#define BASIC_MLD_DISCOVERY_H

#include "hcDiscovery/DiscoveryTools/basic_dis.hpp"

///@author Sebastian Woelke
///@brief abtract class to discover MLD

#define LOGMSG 1
#define MLD_PACKET_LEN 100
#define MLD_FILTER_EXP "ip6 proto 0" // ipv6 Hop-by-Hop Options
//tcpdump bug at version 1.0.0 "ip6 protochain ipv6-icmp"
//ip6  proto  should  chase header chain, but at this moment it does not.
//rfc 2460 4.1  Extension Header Order: ... Hop-by-Hop Options header which is restricted to appear immediately after an IPv6 header only. ...

#define MLD_IPV6_MULTICAST_TEST_GROUP_ADDR "FF02::99:99:99:99"

#define MLD_SAFETY_TIME 2 //sec
#define MLD_PASSIV_TIMEOUT (MC_TV_QUERY_INTERVAL*MC_TV_ROBUSTNESS_VARIABLE + MLD_SAFETY_TIME) //sec ( 2*125sec + 5)
#define MLD_AKTIVE_TIMEOUT (MC_TV_LAST_LISTENER_QUERY_INTERVAL*MC_TV_ROBUSTNESS_VARIABLE) //Last Member Query Interval (1 sec) * Robustness Variable (2)
#define MLD_TIME_BETWEEN_JOIN_LEAVE_GROUP 1000000 //usec

#define MLDv1_HDR_SIZE 24
#define MLD2_MIN_HDR_SIZE 28

#ifndef ICMPV6_MLD2_REPORT //available at kernel linux-2.6.35
#define ICMPV6_MLD2_REPORT 143
#endif


class mld_dis : public basic_dis
{
private:
     string m_pcapFilter;
protected:
     bool ActiveFilter(const u_char* ipvXHdr);
     bool PassiveFilter(const u_char* ipvXHdr);
public:
     mld_dis();
     ~mld_dis();
     bool sniffPassiv(ifreq *item, if_prop* ifInfo);
     bool sniffActive(ifreq *item, if_prop* ifInfo);
};

#endif // BASIC_MLD_DISCOVERY_H
