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

#ifndef DISCOVERYTANK_H_
#define DISCOVERYTANK_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string>
#include <vector>

#include "discovery/if_prop.hpp"
#include "discovery/basic_dis.hpp"
#include "discovery/mc_tables.hpp"

using std::vector;
using std::string;

///@author Sebastian Woelke
///@brief collect information about network environment by the help of @class if_info and several discovery tools

class dis_tank_item;
class dis_proto;

enum discovery_ip_version {
     IP4, IP6, IPALL
};

class dis_tank {
private:
     bool m_initialised;
     if_prop m_ifInfo;
     mc_tables m_tables_v4;
     mc_tables m_tables_v6;
     vector<dis_tank_item*> m_dtItem;

     void cleanTank();
     bool basicTankRefresh();

     //GOF singleton
     dis_tank();
     dis_tank(const dis_tank&);
     dis_tank& operator=(const dis_tank&);
public:
     static dis_tank* getInstance();

     /**
      * @brief clean tank automaticly
      */
     virtual ~dis_tank();

     /**
      * @brief initialize discovery tank
      */
     bool init_DiscoveryTank();

     /**
      * @brief refresh discovery tank ???
      */
     bool refreshTankFast();

     /**
      * @brief refresh discovery tank ???
      */
     bool refreshTankFriendly();

     /**
      * @return number of available network interfaces
      */
     unsigned int getIfCount(); //number of available network interfaces

     /**
      * @return a network interface
      */
     dis_tank_item* getIf(unsigned int index);

     /**
      * @return a network interface
      */
     dis_tank_item* getIf(const string &ifName);

     /**
      * @brief refresh the joined groups
      */
     bool refreshJoinedGroups();

     bool is_initialised() {
          return m_initialised;
     }
};

class dis_tank_item {
private:
     if_prop* m_ifInfo;
     struct ifreq *m_item;

     mc_tables* m_tables_v4;
     mc_tables* m_tables_v6;
     vector<dis_proto*> m_dpItem;

     void cleanProtocols();
     void addProtocol(dis_proto* discoveryProtocol);

     friend class service_discovery;
public:

     dis_tank_item(if_prop* ifInfo,struct ifreq *item, mc_tables* m_tables_v4, mc_tables* m_tables_v6);
     virtual ~dis_tank_item();

     string getName();
     string getIPv4_Addr();
     string getMask();
     string getBroadcast();
     vector<string> getIPv6_Addr();
     //string getMAC_Addr();
     bool isUP();
     bool isRunning();
     bool isLoopback();
     bool isPointToPoint();
     bool isAllMulti();
     bool hasMulticast();
     int getMTU();
     int getNetworkSpeed();
     int getIfIndex();

     /**
      * @brief refresh available network protocols with out needed of root privileges
      */
     bool refresh_protocol_lite(discovery_ip_version version);

     /**
      * @brief refresh available network protocls the fastest way
      */
     bool refreshProtocolFast(discovery_ip_version version);

     /**
      * @brief refresh available network protocols the friendly way
      */
     bool refreshProtocolFriendly(discovery_ip_version version);

     /**
      * @brief return the number of available protocols
      */
     unsigned int getProtocolCount();

     /**
      * @return infos about a network protocols
      */
     dis_proto* getProtocol(unsigned int index);

     /**
      * @return the number of joined groups
      */
     unsigned int getJoinedGroupCount();

     /**
      * @return joined multicast group
      */
     string getJoinedGroup(unsigned int index);
};

#endif /* DISCOVERYTANK_H_ */
