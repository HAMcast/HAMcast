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
#include "hcDiscovery/DiscoveryTools/mrinfo.hpp"
#include "hcDiscovery/Inet_Stuff/raw_socket.hpp"
#include "hcDiscovery/Inet_Stuff/Protocol_Infos/test_output.hpp"
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <iostream>
using namespace std;

mrinfo::mrinfo(){
     HC_LOG_TRACE("");
}

mrinfo::~mrinfo(){
     HC_LOG_TRACE("");
}

bool mrinfo::sniff(ifreq *item, if_prop* ifInfo, string& dst_ip){
     HC_LOG_TRACE("");

     m_pcapFilter = MRINFO_IGMP_FILTER_EXP;

     //start sniffing
     if(initSniffer(item, ifInfo, MRINFO_TIMEOUT, &m_pcapFilter, ACTIVE, MRINFO_PACKET_LEN, AF_INET, true)){
          send_ASK_Neighbors2(item, ifInfo, dst_ip);
          startSniffer();
          return true;
     }

     return false;
}

void mrinfo::joinSniffer(){
     HC_LOG_TRACE("");
     basic_dis::joinSniffer();
}

bool mrinfo::foundProtocol(){
     HC_LOG_TRACE("");
     return basic_dis::foundProtocol();
}

void mrinfo::print_NeighborsInfos(){
     HC_LOG_TRACE("");
     if(foundProtocol()){
          dis_proto dp= getfoundProtocol();
          u_char* m_rawData = dp.getRawData();
          unsigned int m_rawData_size = dp.getRawData_size();

          LAddr_Nbr_map lAddr_nbr_map;
          LAddr_Nbr_value current_nbr_value;
          struct Local_Addr* current_lAddr;

          u_char* tmpPointer = 0;

          struct ip* ipv4Hdr= (struct ip*) m_rawData;
          struct Neighbors2 *neighbors = (struct Neighbors2*) ((char*)ipv4Hdr + ipv4Hdr->ip_hl * 4);

          //create structure
          tmpPointer = ((u_char*)neighbors) + sizeof(struct Neighbors2);
          while(tmpPointer < (m_rawData + m_rawData_size)){
               current_nbr_value.clear();
               current_lAddr = (struct Local_Addr*)tmpPointer;
               tmpPointer= tmpPointer+ sizeof(struct Local_Addr);

               for(int i=0; i< current_lAddr->nbrCount; i++){
                    current_nbr_value.push_back((uint32_t*)tmpPointer);
                    tmpPointer= tmpPointer+ sizeof(uint32_t);
               }

               lAddr_nbr_map.insert(LAddr_Nbr_pair(current_lAddr,current_nbr_value));

          }

          //test_output::printBuf(m_rawData,dp.getRawData_size());
          cout << "Capabilities: [" << (int)neighbors->capabilities << "] " << capabilitiesResolver((int)neighbors->capabilities) << endl;
          //output
          LAddr_Nbr_map::const_iterator iter = lAddr_nbr_map.begin();
          while(iter != lAddr_nbr_map.end()){
               current_lAddr = (*iter).first;
               current_nbr_value = (*iter).second;
               cout << "local Addr: " << inet_ntoa(current_lAddr->localAddr) << endl;
               cout << "\tMetric: " << (int)current_lAddr->metric << endl;
               cout << "\tTreshold: " << (int)current_lAddr->treshold << endl;
               cout << "\tFlags: " << "[" <<(int)current_lAddr->flags << "] "  << flagResolver((int)current_lAddr->flags)<< endl;
               cout << "\tNbr Count: " << (int)current_lAddr->nbrCount << endl;
               for(unsigned int i=0; i< current_nbr_value.size(); i++){
                    cout << "\t\t-Nbr[" << i << "]: " << inet_ntoa(*((struct in_addr*)current_nbr_value[i])) << endl;
               }
               iter++;
          }
     }
}

bool mrinfo::send_ASK_Neighbors2( ifreq *item, if_prop* ifInfo, string& dst_ip){
     HC_LOG_TRACE("");
     raw_socket rs;
     unsigned int size= sizeof(struct ip)+ sizeof(struct ASK_Neighbors2);
     unsigned char mem[size];
     struct ip* ip_Hdr = (struct ip*)mem;
     struct ASK_Neighbors2* ask_Hdr = (struct ASK_Neighbors2*)(mem + sizeof(struct ip));

     ifInfo->getAddr(item);
     sourceAddr = ((struct sockaddr_in *) &item->ifr_addr)->sin_addr.s_addr;

     rs.create_Raw_IPv4_Socket(IPPROTO_UDP);
     rs.setNoIP_Hdr();

     for(unsigned int i=0; i< size;i++){
          mem[i]=0;
     }

     ip_Hdr->ip_v = 4;
     ip_Hdr->ip_hl = 5;
     ip_Hdr->ip_tos = 0;
     ip_Hdr->ip_len = htons(28);
     ip_Hdr->ip_id = htons(0x268f);
     ip_Hdr->ip_off = 0;
     ip_Hdr->ip_ttl = 128;
     ip_Hdr->ip_p = IPPROTO_IGMP;
     ip_Hdr->ip_sum = 0;
     ip_Hdr->ip_src.s_addr = sourceAddr;
     ip_Hdr->ip_dst.s_addr = inet_addr(dst_ip.c_str());

     ask_Hdr->type = MRINFO_IPPROTO_DVMRP;
     ask_Hdr->code = MRINFO_ASK_NEIGHBORS2;
     ask_Hdr->chksum = htons(0xede8);
     ask_Hdr->reserved = htons(0xe);
     ask_Hdr->minV = 0xff;
     ask_Hdr->majV = 0x03;

     if(rs.send_Raw_Packet(mem,size,25,dst_ip)){
          return true;
     }

     return false;
}

bool mrinfo::ActiveFilter(const u_char* ipvXHdr){
     HC_LOG_TRACE("");

     if(getAddrFamily() == AF_INET){

          struct ip* ipv4Hdr= (struct ip*) ipvXHdr;

          if (ipv4Hdr->ip_p == IPPROTO_IGMP) {

               struct Neighbors2 *neighbors = (struct Neighbors2*) ((char*)ipv4Hdr + ipv4Hdr->ip_hl * 4);

               if (ipv4Hdr->ip_dst.s_addr == sourceAddr) {
                    if(neighbors->type == MRINFO_IPPROTO_DVMRP && neighbors->code == MRINFO_NEIGHBORS2){
                         setProtocolType(DVMRP);
                         return true;
                    }
               }
          }
     }
     return false;
}

string mrinfo::flagResolver(int flags){
     string props[]={
          "Tunnel",
          "Source Route",
          "Reserved",
          "Reserved",
          "Down",
          "Disabled",
          "Querier",
          "Leaf"
     };
     int size = 8;

     vector<string> result;
     string tmp;

     for(int i=0; i< size;i++){
          if(flags & 1<<i){
               result.push_back(props[i]);
          }
     }

     if(result.size()>0){
          tmp.append(result[0]);
     }

     for(unsigned int i=1; i< result.size();i++){
          tmp.append("/");
          tmp.append(result[i]);
     }

     return tmp;
}

string mrinfo::capabilitiesResolver(int capabilities){
     string props[]={
          "Leaf",
          "Prune",
          "GenID",
          "Mtrace",
          "Snmp",
     };
     int size = 5;

     vector<string> result;
     string tmp;

     for(int i=0; i< size;i++){
          if(capabilities & 1<<i){
               result.push_back(props[i]);
          }
     }

     if(result.size()>0){
          tmp.append(result[0]);
     }

     for(unsigned int i=1; i< result.size();i++){
          tmp.append("/");
          tmp.append(result[i]);
     }

     return tmp;
}
