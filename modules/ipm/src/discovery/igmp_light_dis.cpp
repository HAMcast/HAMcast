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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <cstring>

#include "hamcast/hamcast_logging.h"
#include "discovery/igmp_light_dis.hpp"
#include "discovery/mc_socket.hpp"

igmp_light_dis::igmp_light_dis():
     m_item(0), m_ifInfo(0), m_worker_thread(0), m_running(false)
{
     HC_LOG_TRACE("");

}

igmp_light_dis::~igmp_light_dis(){
     HC_LOG_TRACE("");

     delete m_worker_thread;
}

bool igmp_light_dis::sniffPassiv(ifreq *item, if_prop* ifInfo){
     HC_LOG_TRACE("");

     m_found_protocol = false;

     m_item = item;
     m_ifInfo =  ifInfo;

     m_if_name = item->ifr_name;

     m_running = true;
     m_worker_thread = new boost::thread(igmp_light_dis::thread_worker,this);

     return true;
}

void igmp_light_dis::thread_worker(void *arg){
     HC_LOG_TRACE("");

     igmp_light_dis* ldis= (igmp_light_dis*) arg;

     int status = 0; // 0 = init, 1 = timer stoped, 2 = querier found
     struct igmp_dev dev;
     mc_socket sock;
     mc_tables t4;
     addr_storage if_addr;
     addr_storage test_addr = addr_storage(IGMP_LIGHT_TEST_GROUP);

     struct timeval start_time;
     struct timeval current_time;

     t4.init_tables(AF_INET);
     if(!t4.refresh_igmp_table()) return;// hat es funktioniert????? und die anderen funktionen

     //igmp interface available, validate interface
     dev= t4.get_igmp_table_dev(ldis->m_if_name);
     if(dev.if_name.compare(ldis->m_if_name) != 0) {
          HC_LOG_ERROR("interface: " << ldis->m_if_name << " not available in the igmp tables");
          return;
     }

     if(dev.querier_version !=IGMPV2_QUERIER_VERSISON) {
          HC_LOG_DEBUG("interface " << ldis->m_if_name << " has wrong igmp version: " << dev.querier_version);
          return;
     }

     HC_LOG_DEBUG("join an igmp testgroup on interface: " << ldis->m_if_name);
     //join an igmp testgroup on every interface
     if(!sock.create_UDP_IPv4_Socket()) return;
     if(!ldis->m_ifInfo->getAddr(ldis->m_item)) return;
     if_addr = ldis->m_item->ifr_addr;
     if(!sock.chooseIf(if_addr.to_string().c_str())) return;
     if(!sock.joinGroup(IGMP_LIGHT_TEST_GROUP,if_addr.to_string().c_str())) return;

     //wait until the timer has been stoped after joining
     while(status==0 && ldis->m_running){
          sleep(MC_TV_UNSOLICITED_REPORT_INTERVAL/3);
          if(!t4.refresh_igmp_table()) return;

          dev= t4.get_igmp_table_dev(ldis->m_if_name);
          if(dev.if_name.compare(ldis->m_if_name) != 0 || dev.querier_version !=IGMPV2_QUERIER_VERSISON) {
               return;
          }

          if(!ldis->timer_running(dev,test_addr)){
               HC_LOG_DEBUG("correct initialized: " << ldis->m_if_name);
               status=1;
          }
     }

     HC_LOG_DEBUG("check all " << IGMP_LIGHT_UPDATE_INTEVAL/1000 << "msec if the timer runs");
     HC_LOG_DEBUG("wait " << IGMP_LIGHT_PASSIV_TIMEOUT << " sec on interface: " << ldis->m_if_name);
     //check all 0.4 sec if the timer runs
     gettimeofday(&start_time, NULL);
     gettimeofday(&current_time,NULL);
     while(current_time.tv_sec < (start_time.tv_sec +IGMP_LIGHT_PASSIV_TIMEOUT) && ldis->m_running){

          usleep(IGMP_LIGHT_UPDATE_INTEVAL);
          if(!t4.refresh_igmp_table()) return;

          dev = t4.get_igmp_table_dev(ldis->m_if_name);
          if(dev.index != t4.get_igmp_table_empty_dev().index){
               if(dev.querier_version == IGMPV2_QUERIER_VERSISON && ldis->timer_running(dev,test_addr)){
                    HC_LOG_DEBUG("IGMPv2 Querier found on interface: " << ldis->m_if_name);
                    status=2;
                    ldis->m_running = false;
               }
          }

          gettimeofday(&current_time,NULL);
     }
     HC_LOG_DEBUG("after waiting");

     if(status == 2){
          HC_LOG_DEBUG("generate output");
          ldis->m_discoveredProtocol.setType(IGMPv2);

          struct sockaddr_storage tmp_addr;
          memset(&tmp_addr,0,sizeof(tmp_addr));
          tmp_addr.ss_family= AF_INET;
          ldis->m_discoveredProtocol.setHostAddr(tmp_addr);

          ldis->m_found_protocol = true;
     }
}

bool igmp_light_dis::timer_running(struct igmp_dev& dev, addr_storage& addr){
     HC_LOG_TRACE("");

     for(unsigned int i=0; i< dev.groups.size(); i++){
          struct igmp_group g= dev.groups[i];
          if(g.group == addr && g.timer_run){
               return true;
          }
     }

     return false;
}

bool igmp_light_dis::sniffActive(ifreq *item, if_prop* ifInfo){
     HC_LOG_TRACE("");
     HC_LOG_ERROR("not implemented");

     return false;
}

void igmp_light_dis::joinSniffer(){
     HC_LOG_TRACE("");

     if(this->m_worker_thread!=NULL){
          this->m_worker_thread->join();
     }
}

void igmp_light_dis::stopSearching(){
     HC_LOG_TRACE("");

     m_running = false;
}

bool igmp_light_dis::foundProtocol(){
     HC_LOG_TRACE("");

     return m_found_protocol;
}

dis_proto igmp_light_dis::getfoundProtocol(){
     HC_LOG_TRACE("");

     return m_discoveredProtocol;
}
