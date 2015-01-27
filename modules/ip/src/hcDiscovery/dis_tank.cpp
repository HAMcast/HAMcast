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
#include "hcDiscovery/dis_tank.hpp"
#include "hcDiscovery/DiscoveryTools/igmp_dis.hpp"
#include "hcDiscovery/DiscoveryTools/igmp_dis.hpp"
#include "hcDiscovery/DiscoveryTools/mld_dis.hpp"
#include "hcDiscovery/DiscoveryTools/pim_v1_dis.hpp"
#include "hcDiscovery/DiscoveryTools/pim_v2_dis.hpp"
#include "hcDiscovery/DiscoveryTools/mrinfo.hpp"
#include "hcDiscovery/DiscoveryTools/igmp_light_dis.hpp"
#include "hcDiscovery/DiscoveryTools/mld_light_dis.hpp"

#include <arpa/inet.h>
#include <iostream>

//###########################
//####-- DiscoveryTank --####
//###########################
dis_tank::dis_tank() :
     m_initialised(false) {
     HC_LOG_TRACE("");
}

dis_tank::~dis_tank() {
     HC_LOG_TRACE("");
     cleanTank();
}

dis_tank* dis_tank::getInstance(){
     HC_LOG_TRACE("");
     static dis_tank instance;
     return &instance;
}

void dis_tank::cleanTank() {
     HC_LOG_TRACE("");

     for (unsigned int i = 0; i < m_dtItem.size(); i++) {
          delete m_dtItem[i];
     }
     m_dtItem.clear();
}

bool dis_tank::basicTankRefresh() {
     HC_LOG_TRACE("");

     cleanTank();

     if (!m_ifInfo.refresh_network_interfaces()) {
          return false;
     }

     unsigned int ifcount = m_ifInfo.getInterfaceCount();
     for (unsigned int i = 0; i < ifcount; i++) {
          m_dtItem.push_back(new dis_tank_item(&m_ifInfo, m_ifInfo.getInterfaceItem(i), &m_tables_v4, &m_tables_v6));
     }

     return true;
}

bool dis_tank::refreshTankFast() {
     HC_LOG_TRACE("");

     if (!basicTankRefresh()) {
          return false;
     }

     return true;
}

bool dis_tank::refreshTankFriendly() {
     HC_LOG_TRACE("");

     if (!basicTankRefresh()) {
          return false;
     }

     return true;
}

bool dis_tank::init_DiscoveryTank() {
     HC_LOG_TRACE("");

     if (!m_ifInfo.init_IfInfo()) {
          return false;
     }

     m_tables_v4.init_tables(AF_INET);
     m_tables_v6.init_tables(AF_INET6);

     m_initialised = true;
     return true;
}

unsigned int dis_tank::getIfCount() {
     HC_LOG_TRACE("");
     return m_dtItem.size();
}

dis_tank_item* dis_tank::getIf(unsigned int index) {
     HC_LOG_TRACE("");

     if (index >= m_dtItem.size()) {
          return 0;
     }

     return m_dtItem[index];
}

dis_tank_item* dis_tank::getIf(const string &ifName){
     HC_LOG_TRACE("");

     for(unsigned int i=0; i < getIfCount(); i++){
          dis_tank_item* dti = getIf(i);
          if(dti->getName().compare(ifName)==0){
               return dti;
          }
     }

     return 0;
}

bool dis_tank::refreshJoinedGroups(){
     HC_LOG_TRACE("");
     return m_tables_v4.refresh_joined_groups() && m_tables_v6.refresh_joined_groups();
}

//###############################
//####-- DiscoveryTankItem --####
//###############################
dis_tank_item::dis_tank_item(if_prop* ifInfo,struct ifreq *item, mc_tables* m_tables_v4, mc_tables* m_tables_v6) :
     m_ifInfo(ifInfo), m_item(item), m_tables_v4(m_tables_v4), m_tables_v6(m_tables_v6) {
     HC_LOG_TRACE("");
}

dis_tank_item::~dis_tank_item() {
     HC_LOG_TRACE("");
     cleanProtocols();
}

void dis_tank_item::cleanProtocols() {
     HC_LOG_TRACE("");

     for (unsigned int i = 0; i < m_dpItem.size(); i++) {
          delete m_dpItem[i];
     }
     m_dpItem.clear();
}

void dis_tank_item::addProtocol(dis_proto* discoveryProtocol) {
     HC_LOG_TRACE("");
     m_dpItem.push_back(discoveryProtocol);
}

string dis_tank_item::getName() {
     HC_LOG_TRACE("");
     return string(m_item->ifr_ifrn.ifrn_name);
}


string dis_tank_item::getMask() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getMask(m_item)){
          return string(inet_ntoa(((struct sockaddr_in *) &m_item->ifr_netmask)->sin_addr));
     }else{
          return string();
     }
}


string dis_tank_item::getBroadcast() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getBroadcast(m_item)){
          return string(inet_ntoa(((struct sockaddr_in *) &m_item->ifr_broadaddr)->sin_addr));
     }else{
          return string();
     }
}

string dis_tank_item::getIPv4_Addr() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getAddr(m_item)){
          return string(inet_ntoa(((struct sockaddr_in *) &m_item->ifr_addr)->sin_addr));
     }else{
          return string();
     }
}

vector<string> dis_tank_item::getIPv6_Addr() {
     HC_LOG_TRACE("");
     char addressBuffer[INET6_ADDRSTRLEN];
     vector<string> ipv6Addr;

     string name=getName();
     vector<struct in6_addr*> tmpAddr=m_ifInfo->getIPv6Addr(name);

     for(unsigned int i=0; i< tmpAddr.size(); i++){
          const char *ptr = inet_ntop(AF_INET6, tmpAddr[i], addressBuffer, sizeof(addressBuffer));
          if(ptr!=NULL){
               ipv6Addr.push_back(string(ptr));
          }
     }

     return ipv6Addr;
}

string dis_tank_item::getMAC_Addr() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getHwAddr(m_item)){
          return m_ifInfo->convertMAC_Addr(m_item);
     }else{
          return string();
     }
}

bool dis_tank_item::isUP() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getFlags(m_item)){
          return (m_item->ifr_flags & IFF_UP) > 0;
     }else{
          return false;
     }
}

bool dis_tank_item::isRunning() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getFlags(m_item)){
          return (m_item->ifr_flags & IFF_RUNNING) > 0;
     }else{
          return false;
     }
}

bool dis_tank_item::isLoopback() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getFlags(m_item)){
          return (m_item->ifr_flags & IFF_LOOPBACK) > 0;
     }else{
          return false;
     }
}

bool dis_tank_item::isPointToPoint() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getFlags(m_item)){
          return (m_item->ifr_flags & IFF_POINTOPOINT) > 0;
     }else{
          return false;
     }
}

bool dis_tank_item::isAllMulti() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getFlags(m_item)){
          return (m_item->ifr_flags & IFF_ALLMULTI) > 0;
     }else{
          return false;
     }
}

bool dis_tank_item::hasMulticast() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getFlags(m_item)){
          return (m_item->ifr_flags & IFF_MULTICAST) > 0;
     }else{
          return false;
     }
}

int dis_tank_item::getMTU() {
     HC_LOG_TRACE("");

     if(m_ifInfo->getMTU(m_item)){
          return (m_item->ifr_mtu);
     }else{
          return -1;
     }
}

int dis_tank_item::getNetworkSpeed() {
     HC_LOG_TRACE("ERROR nicht implementiert");
     //TODO DiscoveryTank
     return -1;
}

int dis_tank_item::getIfIndex(){
     HC_LOG_TRACE("");

     return if_nametoindex(getName().c_str());
}

bool dis_tank_item::refresh_protocol_lite(discovery_ip_version version){
     HC_LOG_TRACE("");

     igmp_light_dis igmp_l_dis;
     mld_light_dis mld_l_dis;

     vector<basic_dis_interface*> basic_dis;
     switch(version){
     case IPvALL: basic_dis.push_back(&igmp_l_dis); basic_dis.push_back(&mld_l_dis); break;
     case IPv6: basic_dis.push_back(&mld_l_dis); break;
     case IPv4: basic_dis.push_back(&igmp_l_dis); break;
     default: HC_LOG_ERROR("unknown ip version"); return false;
     }

     bool rc[basic_dis.size()];

     for(unsigned int i=0; i< basic_dis.size(); i++){
          rc[i] = basic_dis[i]->sniffPassiv(m_item, m_ifInfo);
     }

     for(unsigned int i=0; i< basic_dis.size(); i++){
          basic_dis[i]->joinSniffer();

          if(rc[i]){
               if(basic_dis[i]->foundProtocol()){
                    dis_proto* disPro= new dis_proto;
                    *disPro=basic_dis[i]->getfoundProtocol();
                    addProtocol(disPro);
               }
          }
     }

     for(unsigned int i=0; i< sizeof(basic_dis)/ sizeof(basic_dis_interface*); i++){
          if(!rc[i]){
               return false;
          }
     }

     return true;
}

bool dis_tank_item::refreshProtocolFast(discovery_ip_version version) {
     HC_LOG_TRACE("");

     igmp_dis igmp_Dis;
     mld_dis mld_Dis;

     vector<basic_dis_interface*> basic_dis;
     switch(version) {
     case IPvALL: basic_dis.push_back(&igmp_Dis); basic_dis.push_back(&mld_Dis); break;
     case IPv6: basic_dis.push_back(&mld_Dis); break;
     case IPv4: basic_dis.push_back(&igmp_Dis); break;
     default: HC_LOG_ERROR("unknown ip version"); return false;
     }

     bool rc[basic_dis.size()];

     for(unsigned int i=0; i< basic_dis.size(); i++){
          rc[i] = basic_dis[i]->sniffActive(m_item, m_ifInfo);
     }

     for(unsigned int i=0; i< basic_dis.size(); i++){
          basic_dis[i]->joinSniffer();

          if(rc[i]){
               if(basic_dis[i]->foundProtocol()){
                    dis_proto* disPro= new dis_proto;
                    *disPro=basic_dis[i]->getfoundProtocol();
                    addProtocol(disPro);
               }
          }
     }

     for(unsigned int i=0; i< basic_dis.size(); i++){
          if(!rc[i]){
               return false;
          }
     }

     return true;
}

bool dis_tank_item::refreshProtocolFriendly(discovery_ip_version version) {
     HC_LOG_TRACE("");

     igmp_dis igmp_Dis;
     mld_dis mld_Dis;
     pim_v1_dis pim_v1_Dis;
     pim_v2_dis pim_v2_Dis;

     vector<basic_dis_interface*> basic_dis;
     switch(version){
     case IPvALL: basic_dis.push_back(&pim_v1_Dis); basic_dis.push_back(&pim_v2_Dis); basic_dis.push_back(&igmp_Dis); basic_dis.push_back(&mld_Dis); break;
     case IPv6: basic_dis.push_back(&mld_Dis); basic_dis.push_back(&pim_v2_Dis); break;
     case IPv4: basic_dis.push_back(&igmp_Dis); basic_dis.push_back(&pim_v1_Dis); break;
     default: HC_LOG_ERROR("unknown ip version"); return false;
     }

     bool rc[basic_dis.size()];

     for(unsigned int i=0; i< basic_dis.size(); i++){
          rc[i] = basic_dis[i]->sniffPassiv(m_item, m_ifInfo);
     }

     for(unsigned int i=0; i< basic_dis.size(); i++){
          basic_dis[i]->joinSniffer();

          if(rc[i]){
               if(basic_dis[i]->foundProtocol()){
                    dis_proto* disPro= new dis_proto;
                    *disPro=basic_dis[i]->getfoundProtocol();
                    addProtocol(disPro);
               }
          }
     }

     for(unsigned int i=0; i< basic_dis.size(); i++){
          if(!rc[i]){
               return false;
          }
     }

     return true;
}

unsigned int dis_tank_item::getProtocolCount() {
     HC_LOG_TRACE("");
     return m_dpItem.size();
}

dis_proto* dis_tank_item::getProtocol(unsigned int index) {
     HC_LOG_TRACE("");

     if (index >= m_dpItem.size()) {
          return 0;
     }

     return m_dpItem[index];
}

unsigned int dis_tank_item::getJoinedGroupCount(){
     string tmp=this->getName();
     HC_LOG_TRACE("for interface: " << tmp);

     return m_tables_v4->get_joined_groups_count(tmp)+m_tables_v6->get_joined_groups_count(tmp);
}

string dis_tank_item::getJoinedGroup(unsigned int index){
     string tmp=this->getName();
     HC_LOG_TRACE("for interface: " << tmp << " index: " << index);

     unsigned int tmp_Count= m_tables_v4->get_joined_groups_count(tmp);

     if(index >= this->getJoinedGroupCount()){
          return string();
     }else if(index < tmp_Count){
          return m_tables_v4->get_joined_group(tmp,index).to_string();
     }else if(index >= tmp_Count){
          return m_tables_v6->get_joined_group(tmp,index-tmp_Count).to_string();
     }else{
          HC_LOG_ERROR("wrong index: " << index);
          return string();
     }
}

bool dis_tank_item::print_mrinfo(){
     mrinfo mri;
     dis_proto* disp = 0;
     string dst_ip;

     for(unsigned int i=0;i< getProtocolCount(); i++){

          if(getProtocol(i)->getType() == IGMPv2 || getProtocol(i)->getType() == IGMPv3){
               disp = getProtocol(i);
          }
     }

     if(disp){
          dst_ip = disp->getHostAddr();
          if(mri.sniff(m_item, m_ifInfo, dst_ip)){
               mri.joinSniffer();
               if(mri.foundProtocol()){
                    mri.print_NeighborsInfos();
                    return true;
               }
          }
     }

     return false;
}
