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

#ifndef MC_TABLES_HPP
#define MC_TABLES_HPP

#include "hcDiscovery/Inet_Stuff/addr_storage.hpp"

#include <netinet/in.h>
#include <map>
#include <vector>
#include <iostream>
#ifdef __APPLE__
	#include <netinet/ip_mroute.h>
	#include <netinet6/ip6_mroute.h>
#else
	#include <linux/mroute.h>
	#include <linux/mroute6.h>
#endif
using namespace std;

#define MAX_N_LINE_LENGTH 200
#define JOINED_GROUP_PATH_V6 "/proc/net/igmp6"
#define JOINED_GROUP_PATH_V4 "/proc/net/igmp"
#define VIF_PATH_V4 "/proc/net/ip_mr_vif"
#define VIF_PATH_V6 "/proc/net/ip6_mr_vif"
#define MR_CACHE_PATH_V4 "/proc/net/ip_mr_cache"
#define MR_CACHE_PATH_V6 "/proc/net/ip6_mr_cache"
#define SNMP6_PATH "/proc/net/dev_snmp6"

typedef vector<addr_storage> MGroup_value;
typedef map<string, MGroup_value > MGroup_map;
typedef pair<string, MGroup_value > MGroup_pair;

typedef struct snmp6 SNMP6_value;
typedef map<string, SNMP6_value > SNMP6_map;
typedef pair<string, SNMP6_value > SNMP6_pair;

struct mr_vif{
     int addr_family;
     int vifi; //virutuell interface index
     string ifname;
     int bytesIn;
     int pktsIn;
     int bytesOut;
     int pktsOut;
     int flags;
     int lcl_index; //use for AF_INET
     addr_storage lcl_addr; //only for AF_INET
     addr_storage remote; //only for AF_INET
};

struct mr_cache{
     int addr_family;
     addr_storage group;
     addr_storage origin;
     int i_if;
     int pkts;
     int bytes;
     int wrong;
     vector<int> o_if;
};

struct snmp6 {
     int Icmp6InGroupMembQueries;
     int Icmp6InGroupMembResponses;
     int Icmp6OutGroupMembQueries;
     int Icmp6OutGroupMembResponses;

     int Icmp6InMLDv2Reports;
     int Icmp6OutMLDv2Reports;
};

struct igmp_dev{
     int index; //inteface index
     string if_name; //interface name
     int g_count; //count joined groups on this interface
     int querier_version; //querier version (default V3)
     vector<struct igmp_group> groups;
};

struct igmp_group{
     addr_storage group; //joined group
     int users; //number of user on this pc joined this group
     bool timer_run;
     int timer; //current respons time
     bool reporter; //i am the reporter of this group only V2
};

class mc_tables
{
private:
     int m_addr_family;
     //--filesystem access--
     MGroup_map m_mgroup_map;

     vector<struct mr_vif> m_mr_vif;
     vector<struct mr_cache> m_mr_cache;

     SNMP6_map m_snmp6_map;

     vector<struct igmp_dev> m_igmp_table;

     //--general--
     string int2str(int x);
     addr_storage hexCharAddr_To_ipFormart(string& ipAddr, int addrFamily);
     int str2int(string x);
     int hexChar_To_int(char x);

     //trim fist and last white_spaces

public:
     static void trim(char* str, unsigned int size);
     mc_tables();

     /**
     * @brief set address family
     */
     void init_tables(int addrFamily);

     //#######################
     //##-- joined groups --##
     //#######################
     /**
     * @brief refresh all devices and their joined groups
     */
     bool refresh_joined_groups();

     /**
     * @return the number of joined interfa2ces for a spezific network interface
     */
     unsigned int get_joined_groups_count(string& ifName);

     /**
     * @return the number of joined interfaces for a spezific network interface
     */
     addr_storage get_joined_group(string& ifName, unsigned int index);

     /**
     * @brief print all available devices and their joined groups
     */
     void print_all_joined_groups();

     //###############
     //##-- vifs -- ##
     //###############
     /**
      * @brief refresh the virutell interfaces
      */
     bool refresh_vifs();

     /**
      * @return the number of configured virtual interfaces
      */
     unsigned int get_vifs_count();

     /**
      * @return struct mr_vif
      */
     struct mr_vif get_vif(unsigned int index);

     /**
      * @brief print virtual interface info
      */
     void print_vif_info(struct mr_vif& t);

     /**
      * @brief print all virtual interface infos
      */
     void print_all_vif_infos();

     //################
     //##-- routes --##
     //################
     /**
      * @brief refresh the multicast routes
      */
     bool refresh_routes();

     /**
      * @return the number of configured mutlicast routes
      */
     unsigned int get_routes_count();

     /**
      * @return struct mr_cache
      */
     struct mr_cache get_route(unsigned int index);

     /**
      * @brief print a multicast route
      */
     void print_route_info(struct mr_cache& t);

     /**
      * @brief print all multicast route infos
      */
     void print_all_route_infos();

     //###############
     //##-- snmp6 --##
     //###############
     /**
      * @brief refresh the snmp6
      */
     bool refresh_snmp6();

     /**
      * @brief get snmp6 infos for a specific interface or an empty snmp6 struct
      */
     struct snmp6 get_snmp6(string if_name);

     /**
      * @return empty snmp6 struct
      */
     struct snmp6 get_snmp6_empty_struct();

     /**
      * @return all available interfaces watched by snmp6
      */
     vector<string> get_snmp6_all_interfaces();

     /**
      * @brief print snmp6 infos
      */
     void print_snmp6_infos(const struct snmp6* s);

     /**
      * @brief print all snmp6 infos
      */
     void print_all_snmp6_infos();

     //####################
     //##-- igmp table --##
     //####################
     /**
      * @brief refresh igmp table
      */
     bool refresh_igmp_table();

     /**
      * @return empty igmp dev struct
      */
     struct igmp_dev get_igmp_table_empty_dev();

     /**
      * @brief get igmp infos for a specific interface
      * @return the struct igmp_dev or on error an empty_igmp_dev struct
      */
     struct igmp_dev get_igmp_table_dev(string if_name);

     /**
      * @return the igmp table
      */
     vector<struct igmp_dev> get_igmp_table_all();

     /**
      * @return all available interfaces seen in the igmp table
      */
     vector<string> get_igmp_table_all_interfaces();

     /**
      * @brief print igmp infos for a specific interface
      */
     void print_igmp_table_dev(const struct igmp_dev* s);

     /**
      * @brief print the igmp table
      */
     void print_all_igmp_table();

     //################
     //##-- tests -- ##
     //################
     /**
     * @brief simple test outputs
     */
     static void test_joined_groups(int addrFamily);
     static void test_vifs(int addrFamily);
     static void test_mr_cache(int addrFamily);
     static void test_snmp6();
     static void test_igmp_table();

};

#endif // MC_TABLES_HPP
