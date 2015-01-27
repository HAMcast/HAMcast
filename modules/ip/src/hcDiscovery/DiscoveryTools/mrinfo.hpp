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

#ifndef MRINFO_HPP
#define MRINFO_HPP

#include "hcDiscovery/DiscoveryTools/basic_dis.hpp"

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
using namespace std;

#define MRINFO_IPPROTO_DVMRP 0x13
#define MRINFO_ASK_NEIGHBORS2 0x05
#define MRINFO_NEIGHBORS2 0x06
#define MRINFO_TIMEOUT 2
#define MRINFO_PACKET_LEN 1000
#define MRINFO_IGMP_FILTER_EXP "igmp"

typedef vector<uint32_t*> LAddr_Nbr_value;
typedef map<struct Local_Addr*, LAddr_Nbr_value > LAddr_Nbr_map;
typedef pair<struct Local_Addr*, LAddr_Nbr_value > LAddr_Nbr_pair;


class mrinfo : private basic_dis
{
private:
     in_addr_t sourceAddr;
     string m_pcapFilter;

     bool send_ASK_Neighbors2(ifreq *item, if_prop* ifInfo, string& dst_ip);
     bool ActiveFilter(const u_char* ipvXHdr);

     string flagResolver(int flag);
     string capabilitiesResolver(int capabilities);

     //not used
     bool sniffActive(ifreq *item, if_prop* ifInfo){ return false; }
     bool sniffPassiv(ifreq *item, if_prop* ifInfo){ return false; }
     bool PassiveFilter(const u_char* ipvXHdr){ return false; }

public:
     mrinfo();
     ~mrinfo();

     bool sniff(ifreq *item, if_prop* ifInfo, string& dst_ip);
     void joinSniffer();
     bool foundProtocol();
     void print_NeighborsInfos();
};

struct ASK_Neighbors2{
     uint8_t type;
     uint8_t code;
     uint16_t chksum;
     uint16_t reserved;
     uint8_t minV;
     uint8_t majV;
};

struct Local_Addr{
     struct in_addr localAddr;
     uint8_t metric;
     uint8_t treshold;
     uint8_t flags;
     uint8_t nbrCount;
     //uint32_t* nbr;
};

struct Neighbors2{
     uint8_t type;
     uint8_t code;
     uint16_t chksum;
     uint8_t reserved;
     uint8_t capabilities;
     uint8_t minV;
     uint8_t majV;
     //struct Local_Addr* lAddr;
};

#endif // MRINFO_HPP
