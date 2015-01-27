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

#ifndef PIMV2_DISCOVERY_H
#define PIMV2_DISCOVERY_H

#include "hcDiscovery/DiscoveryTools/basic_dis.hpp"

///@author Sebastian Woelke
///@brief discover PIMv2 on a spezific network interface

#define LOGMSG 1
#define PIMv2_PACKET_LEN 100
#define PIMv2_FILTER_EXP "ip6" //Pcap Filter expression
//tcpdump bug at version 1.0.0 "ip6 protochain ipv6-icmp"
//ip6  proto  should  chase header chain, but at this moment it does not.
//ip6 protochain is supplied for this behavior.

#define PIMv2_PASSIV_TIMEOUT 65 //sec ( 2*30sec + 5)



class pim_v2_dis : public basic_dis
{
private:
     string m_pcapFilter;
protected:
     virtual bool ActiveFilter(const u_char* ipvXHdr);
     virtual bool PassiveFilter(const u_char* ipvXHdr);
public:
     pim_v2_dis();
     ~pim_v2_dis();
     bool sniffPassiv(ifreq *item, if_prop* ifInfo);
     bool sniffActive(ifreq *item, if_prop* ifInfo);
};

#endif // PIMV2_DISCOVERY_H
