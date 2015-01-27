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

#include "service_discovery.hpp"
#include "hcDiscovery/hamcast_logging.h"

#include <boost/thread.hpp>
#include "iostream"
using namespace std;
#include <arpa/inet.h>

//###########################
//##-- service_discovery --##
//###########################
service_discovery::service_discovery(discovery_type type, discovery_ip_version version){
     HC_LOG_TRACE("");

     m_discovery_type = type;
     m_discovery_ip_version = version;

     m_dis_tank= dis_tank::getInstance();
     if(m_dis_tank->init_DiscoveryTank()){
          refreshSD();
     }

}

vector<sd_if> service_discovery::getSD_if(){
     HC_LOG_TRACE("");

     vector<sd_if> vif;

     for(unsigned int i=0; i<m_dis_tank->getIfCount();i++){
          dis_tank_item* dtItem = m_dis_tank->getIf(i);

          // running, no loopback, multicast capable interfaces
          if(dtItem->isUP() && dtItem->isRunning() && !dtItem->isLoopback() && dtItem->hasMulticast()){
               dis_proto* p_igmp=0;
               dis_proto* p_mld=0;

               if(m_discovery_type != OFF){

                    //search protocols
                    for(unsigned int j=0; j< dtItem->getProtocolCount();j++){
                         dis_proto* p_tmp= dtItem->getProtocol(j);
                         if(p_tmp->getType() == IGMPvX || p_tmp->getType() == IGMPv1 || p_tmp->getType() == IGMPv2 || p_tmp->getType()== IGMPv3){
                              p_igmp =p_tmp;
                         }else if(p_tmp->getType() == MLDvX || p_tmp->getType() == MLDv1 || p_tmp->getType()== MLDv2){
                              p_mld=p_tmp;
                         }
                    }

                    if(p_igmp){
                         sd_if sif;

                         sif.m_ifname = dtItem->getName();
                         sif.m_ifindex = dtItem->getIfIndex();

                         dtItem->m_ifInfo->getAddr(dtItem->m_item);
                         *(struct in_addr*)(&sif.m_ifaddr.__ss_align) = ((struct sockaddr_in*) &dtItem->m_item->ifr_addr)->sin_addr;
                         sif.m_ifaddr.ss_family = AF_INET;

                         sif.m_addr_family = AF_INET;

                         switch(p_igmp->getType()){
                         case IGMPvX: sif.m_mcast_ver = SD_IGMPvX_VERION; break;
                         case IGMPv1: sif.m_mcast_ver = SD_IGMPv1_VERSION; break;
                         case IGMPv2: sif.m_mcast_ver = SD_IGMPv2_VERSION; break;
                         case IGMPv3: sif.m_mcast_ver = SD_IGMPv3_VERSION; break;
                         default: sif.m_mcast_ver = SD_UNKNOWN_PROTO; HC_LOG_ERROR("unknown igmp version, unreachable state");
                         }

                         sif.m_parent = p_igmp->m_hostAddr;

                         vif.push_back(sif);
                    }

                    if(p_mld){
                         sd_if sif;
                         string name = dtItem->getName();
                         sif.m_ifname = name;
                         sif.m_ifindex = dtItem->getIfIndex();

                         vector<struct in6_addr*> tmpAddr = dtItem->m_ifInfo->getIPv6Addr(name);
                         if(tmpAddr.size()>0){
                              sif.m_ifaddr.ss_family = AF_INET6;
                              *(struct in6_addr*)(&sif.m_ifaddr.__ss_align) = *tmpAddr[0];
                         }else{
                              memset(&sif.m_ifaddr,0,sizeof(sif.m_ifaddr));
                              sif.m_ifaddr.ss_family = AF_INET6;
                         }

                         sif.m_addr_family = AF_INET6;

                         switch(p_mld->getType()){
                         case MLDvX: sif.m_mcast_ver = SD_MLDvX_VERSION; break;
                         case MLDv1: sif.m_mcast_ver = SD_MLDv1_VERSION; break;
                         case MLDv2: sif.m_mcast_ver = SD_MLDv2_VERSION; break;
                         default: sif.m_mcast_ver= SD_UNKNOWN_PROTO; HC_LOG_ERROR("unknown mld version, unreachable state");
                         }

                         sif.m_parent = p_mld->m_hostAddr;

                         vif.push_back(sif);
                    }

               }else{ //m_discovery_type == off
                    sd_if sif;
                    string name = dtItem->getName();
                    sif.m_ifname = name;
                    sif.m_ifindex = dtItem->getIfIndex();

                    sif.m_mcast_ver= SD_UNKNOWN_PROTO;

                    memset(&sif.m_parent,0,sizeof(sif.m_parent));

                    if(m_discovery_ip_version == IPv6 || m_discovery_ip_version == IPvALL){
                         vector<struct in6_addr*> tmpAddr = dtItem->m_ifInfo->getIPv6Addr(name);
                         if(tmpAddr.size()>0){
                              sif.m_ifaddr.ss_family = AF_INET6;
                              *(struct in6_addr*)(&sif.m_ifaddr.__ss_align) = *tmpAddr[0];
                         }else{
                              memset(&sif.m_ifaddr,0,sizeof(sif.m_ifaddr));
                              sif.m_ifaddr.ss_family = AF_INET6;
                         }

                         sif.m_addr_family = AF_INET6;

                         sif.m_parent.ss_family = AF_INET6;

                         vif.push_back(sif);
                    }

                    if(m_discovery_ip_version == IPv4 || m_discovery_ip_version == IPvALL){
                         dtItem->m_ifInfo->getAddr(dtItem->m_item);
                         *(struct in_addr*)(&sif.m_ifaddr.__ss_align) = ((struct sockaddr_in*) &dtItem->m_item->ifr_addr)->sin_addr;
                         sif.m_ifaddr.ss_family = AF_INET;


                         sif.m_addr_family = AF_INET;

                         sif.m_parent.ss_family = AF_INET;

                         vif.push_back(sif);
                    }
               }

          }
     }

     return vif;
}

void service_discovery::refreshSD(){
     HC_LOG_TRACE("");

     if(m_dis_tank->refreshTankFast()){
          if(m_discovery_type != OFF){

               boost::thread* worker_thread[m_dis_tank->getIfCount()];
               struct worker_thread_param param[m_dis_tank->getIfCount()];

               //start thread and create
               for(unsigned int i=0; i<m_dis_tank->getIfCount();i++){
                    dis_tank_item* dtItem = m_dis_tank->getIf(i);
                    if(dtItem->isUP() && dtItem->isRunning() && !dtItem->isLoopback() && dtItem->hasMulticast()){
                         param[i].dt_item = dtItem;
                         param[i].sd = this;
                         worker_thread[i] = new boost::thread(service_discovery::thread_worker,&param[i]);

                    }else{
                         worker_thread[i]=0;
                    }
               }

               //join thread and delete
               for(unsigned int i=0; i< m_dis_tank->getIfCount(); i++){
                    if(worker_thread[i]!=0){
                         worker_thread[i]->join();
                         delete worker_thread[i];
                    }
               }


          }
     }
}

void service_discovery::thread_worker(void *arg){
     HC_LOG_TRACE("");

     struct worker_thread_param* param = (struct worker_thread_param*) arg;

     if(param->sd->m_discovery_type == LIGHT){
          param->dt_item->refresh_protocol_lite(param->sd->m_discovery_ip_version);
     }else if(param->sd->m_discovery_type == FULL){

          // Check that we are root
          if (geteuid() == 0) { //root
               param->dt_item->refreshProtocolFast(param->sd->m_discovery_ip_version);
          }else{ //no root privilegis
               HC_LOG_WARN("no root privilegis, so started service discovery light");
               param->dt_item->refresh_protocol_lite(param->sd->m_discovery_ip_version);
          }

     }else{
          HC_LOG_ERROR("unknown discover type");
     }

}

void service_discovery::testoutput_service_discovery(){
     HC_LOG_TRACE("");

     cout << "##-- service_discovery testoutput ";
     flush(cout);

     char buffer[INET6_ADDRSTRLEN];
     vector<sd_if> vif = getSD_if();
     cout << "[#" << vif.size() << "] --##" << endl;
     for(unsigned int i=0; i< vif.size();i++){
          sd_if f = vif[i];

          cout << "##-- " << f.get_ifname() << " (IfIndex=" << f.get_ifindex() << ") --##" << endl;

          //convert f.get_ifaddr() to cstring
          if(inet_ntop(f.get_addr_family(), (void*)&f.get_ifaddr().__ss_align ,buffer,sizeof(buffer)) == NULL){
               cout << "ERROR!! failed to convert ipAddr" << endl;
               continue;
          }

          cout << "\tipAddr: " << buffer << endl;

          if(f.get_mcast_ver() != SD_UNKNOWN_PROTO){
               if(f.get_addr_family() == AF_INET){
                    if(f.get_mcast_ver() != SD_IGMPvX_VERION){
                         cout << "\tprotocol: IGMPv" << f.get_mcast_ver() << endl;
                    }else{
                         cout << "\tprotocol: IGMPvX" << endl;
                    }
               }else if(f.get_addr_family() == AF_INET6){
                    if(f.get_mcast_ver() != SD_IGMPvX_VERION){
                         cout << "\tprotocol: MLDv" << f.get_mcast_ver() << endl;
                    }else{
                         cout << "\tprotocol: MLDvX" << endl;
                    }
               }else{
                    HC_LOG_ERROR("wrong address family");
                    cout << "\tprotocol: ??" << endl;
               }
          }else{
               cout << "\tprotocol: --" << endl;
          }

          //convert f.get_parent() to cstring
          if(inet_ntop(f.get_addr_family(), (void*)&f.get_parent().__ss_align ,buffer,sizeof(buffer)) == NULL){
               cout << "ERROR!! failed to convert ipAddr" << endl;
               continue;
          }
          cout << "\tquerier: " << buffer<< endl;

     }
}

//###############
//##-- sd_if --##
//###############
sd_if::sd_if(){
     HC_LOG_TRACE("");
}

string sd_if::get_ifname(){
     HC_LOG_TRACE("");

     return m_ifname;
}

int sd_if::get_ifindex(){
     HC_LOG_TRACE("");

     return m_ifindex;
}

struct sockaddr_storage sd_if::get_ifaddr(){
     HC_LOG_TRACE("");

     return m_ifaddr;
}

int sd_if::get_addr_family(){
     HC_LOG_TRACE("");

     return m_addr_family;
}

int sd_if::get_mcast_ver(){
     HC_LOG_TRACE("");

     return m_mcast_ver;
}

struct sockaddr_storage sd_if::get_parent(){
     HC_LOG_TRACE("");

     return m_parent;
}

vector<struct sockaddr_storage> sd_if::get_neighbors(){
     HC_LOG_TRACE("");

     return m_neighbors;
}
