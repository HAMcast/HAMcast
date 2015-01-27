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

#ifndef IF_PROP_H_
#define IF_PROP_H_

#include <net/if.h>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
using namespace std;

///@author Sebastian Woelke
///@brief detect network interface properties

#define MAX_N_IF 30 //number of maximum saved and handled network interfaces
#define SIZEOFBUFF (MAX_N_IF*sizeof(struct ifreq))

//#define MAX_N_LINE_LENGTH 100
//#define JOINED_GROUP_PATH_V6 "/proc/net/igmp6"
//#define JOINED_GROUP_PATH_V4 "/proc/net/igmp"

//typedef vector<string> MGroup_value;
//typedef map<string, MGroup_value > MGroup_map;
//typedef pair<string, MGroup_value > MGroup_pair;


class if_prop {
private:
     //--ioctl access--
     int m_sck;
     struct ifconf m_ifc;
     struct ifreq *m_ifr; //interface request (interfaces properties)
     char m_buf[SIZEOFBUFF]; //ifr--->buf
     unsigned int m_nInterfaces; //number of available interfaces
     bool m_own_socket;
     //--getifaddrs access--
     struct ifaddrs* m_ifa;

     //--filesystem access--
     //MGroup_map m_mgroup_map;

     //--general--
     string hexCharAddr_To_ipFormart(string& ipAddr, int addrFamily);

public:
     if_prop();

     /**
      * @brief close datagram socket automaticly
      */
     ~if_prop();

     /**
      * @brief set a socket
      * @return true on success
      */
     bool init_IfInfo(int socket);
     /**
      * @brief create datagram socket
      * @return true on success
      */
     bool init_IfInfo();

     /**
      * @brief return network interface address
      * @return true on success
      */
     bool getAddr(ifreq *item);

     /**
      * @brief return network interface destination address (remote p2p address)
      * @return true on success
      */
     bool getDstAddr(ifreq *item);

     /**
      * @brief return network interface flags
      * @return true on success
      */
     bool getFlags(ifreq* item);

     /**
      * @brief return network interface flags
      * @return true on success
      */
     bool setFlags(ifreq* item);

     /**
      * @brief return hardware address
      * @return true on success
      */
     bool getHwAddr(ifreq* item);

     /**
      * @brief return subnet mask
      * @return true on success
      */
     bool getMask(ifreq* item);

     /**
      * @brief return broadcast address
      * @return true on success
      */
     bool getBroadcast(ifreq* item);

     /**
      * @brief return maximum transmission unit
      * @return true on success
      */
     bool getMTU(ifreq* item);

     /**
      * @return number of network interfaces
      */
     unsigned int getInterfaceCount();

     /**
      * @return struct ifreq* for a spezific network interface and NULL on error
      */
     struct ifreq* getInterfaceItem(unsigned int index);

     /**
      * @return struct ifreq* for a spezific network interface and NULL on error
      */
     struct ifreq* getInterfaceItem(string if_name);

     /**
      * @brief refresh all information of all interfaces
      * @param return true on success
      */
     bool refresh_network_interfaces();

     /**
      * @return all IPv6 addresses of interface ifname
      */
     vector<struct in6_addr*> getIPv6Addr(string& ifName);

     /**
      * @brief print all available network interface informations
      */
     void printIfInfo();

     /**
      * @brief print an interface address list
      */
     void print_Address_list();

     /**
      * @return mac address in cleartext
      */
     string convertMAC_Addr(ifreq* item);

     /**
      * @brief check for valid socket deskriptor
      */
     bool is_sck_valid() {
          return m_sck > 0;
     }

     bool is_ifa_valid(){
          return m_ifa > 0;
     }

};

#endif /* IF_PROP_H_ */
