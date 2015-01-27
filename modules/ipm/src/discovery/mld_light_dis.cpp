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
#include <cstring>
#include <sys/time.h>
#include <boost/thread.hpp>

#include "hamcast/hamcast_logging.h"
#include "discovery/mld_light_dis.hpp"

mld_light_dis::mld_light_dis():
     m_item(0), m_ifInfo(0), m_worker_thread(0), m_running(false)

{
     HC_LOG_TRACE("");

}

mld_light_dis::~mld_light_dis(){
     HC_LOG_TRACE("");

     delete m_worker_thread;
}

bool mld_light_dis::sniffPassiv(ifreq *item, if_prop* ifInfo){
     HC_LOG_TRACE("");

     m_found_protocol = false;

     m_item = item;
     m_ifInfo =  ifInfo;

     m_if_name = item->ifr_name;

     m_t6.init_tables(AF_INET6);
     if(!m_t6.refresh_snmp6()) return false;

     m_snmp_infos_old = m_t6.get_snmp6(m_if_name);
     //snmp6 interface available
     if(m_snmp_infos_old.Icmp6InGroupMembQueries == m_t6.get_snmp6_empty_struct().Icmp6InGroupMembQueries){
          return false;
     }

     m_running = true;
     m_worker_thread = new boost::thread(mld_light_dis::thread_worker,this);

     return true;
}

void mld_light_dis::thread_worker(void *arg){
     HC_LOG_TRACE("");

     mld_light_dis* ldis= (mld_light_dis*) arg;
     struct timeval start_time;
     struct timeval current_time;
     gettimeofday(&start_time, NULL);
     gettimeofday(&current_time,NULL);

     HC_LOG_DEBUG("wait " << MLD_LIGHT_PASSIV_TIMEOUT << " sec on interface: " << ldis->m_if_name);
     while(ldis->m_running && start_time.tv_sec + MLD_LIGHT_PASSIV_TIMEOUT >= current_time.tv_sec ){
          sleep(MLD_LIGHT_EXIT_TIME);

          if(!ldis->m_t6.refresh_snmp6()) return;
          struct snmp6 snmp_infos_new = ldis->m_t6.get_snmp6(ldis->m_if_name);

          //snmp6 interface available
          if(snmp_infos_new.Icmp6InGroupMembQueries == ldis->m_t6.get_snmp6_empty_struct().Icmp6InGroupMembQueries){
               HC_LOG_ERROR("interface: " << ldis->m_if_name << " not available in the snmp6 tables");
               return;
          }

          if(ldis->m_snmp_infos_old.Icmp6InGroupMembQueries < snmp_infos_new.Icmp6InGroupMembQueries){
               ldis->m_running = false;

               ldis->m_discoveredProtocol.setType(MLDvX);

               struct sockaddr_storage tmp_addr;
               memset(&tmp_addr,0,sizeof(tmp_addr));
               tmp_addr.ss_family = AF_INET6;
               ldis->m_discoveredProtocol.setHostAddr(tmp_addr);

               ldis->m_found_protocol = true;
          }else{
               gettimeofday(&current_time,NULL);
          }
     }
     HC_LOG_DEBUG("after waiting");

}

bool mld_light_dis::sniffActive(ifreq *item, if_prop* ifInfo){
     HC_LOG_TRACE("");
     HC_LOG_ERROR("not implemented");

     return false;
}

void mld_light_dis::joinSniffer(){
     HC_LOG_TRACE("");

     if(this->m_worker_thread!=NULL){
          this->m_worker_thread->join();
     }
}

void mld_light_dis::stopSearching(){
     HC_LOG_TRACE("");

     m_running = false;
}

bool mld_light_dis::foundProtocol(){
     HC_LOG_TRACE("");

     return m_found_protocol;
}

dis_proto mld_light_dis::getfoundProtocol(){
     HC_LOG_TRACE("");

     return m_discoveredProtocol;
}
