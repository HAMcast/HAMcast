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

#ifndef SERVICE_DISCOVERY_HPP
#define SERVICE_DISCOVERY_HPP

#include <string>
#include <netinet/in.h>
#include <vector>

#include <hcDiscovery/dis_tank.hpp>

using namespace std;

class sd_if;


#define SD_UNKNOWN_PROTO -1

#define SD_IGMPvX_VERION 0
#define SD_IGMPv1_VERSION 1
#define SD_IGMPv2_VERSION 2
#define SD_IGMPv3_VERSION 3

#define SD_MLDvX_VERSION 0
#define SD_MLDv1_VERSION 1
#define SD_MLDv2_VERSION 2


enum discovery_type {
     OFF, LIGHT, FULL
};

class service_discovery
{
private:
     discovery_type m_discovery_type;
     discovery_ip_version m_discovery_ip_version;
     dis_tank* m_dis_tank;

     static void thread_worker(void *arg);
public:
     service_discovery(discovery_type type, discovery_ip_version version);

     /**
     * @return all running, no loopback, multicast capable interfaces
     */
     vector<sd_if> getSD_if();

     /**
     * @brief refresh service discovery
     */
     void refreshSD();

     /**
     * @brief a little test output
     */
     void testoutput_service_discovery();
};

class sd_if{
private:
     string m_ifname;
     int m_ifindex;
     struct sockaddr_storage m_ifaddr;
     int m_addr_family;
     int m_mcast_ver;
     struct sockaddr_storage m_parent;
     vector<struct sockaddr_storage> m_neighbors;

     friend class service_discovery;
public:
     sd_if();

     /**
      * @return interface name e.g. "eth0" or "wlan0"
      */
     string get_ifname();

     /**
      * @return interface index
      */
     int get_ifindex();

     /**
      * @return primary ipv4/6 interface address
      */
     struct sockaddr_storage get_ifaddr();

     /**
      * @return AF_INET or AF_INET6
      */
     int get_addr_family();

     /**
      * @brief igmp (ipv4) or mld (ipv6) version
      * @return 1, 2, 3 = mld/igmp version || 0 = unknown version but protocol found  || -1 = no protocol found, see defines
      */
     int get_mcast_ver();

     /**
      * @return querier
      */
     struct sockaddr_storage get_parent();

     /**
      * @return neighbor set
      */
     vector<struct sockaddr_storage> get_neighbors();
};

struct worker_thread_param{
     service_discovery* sd;
     dis_tank_item* dt_item;
};

#endif // SERVICE_DISCOVERY_HPP
