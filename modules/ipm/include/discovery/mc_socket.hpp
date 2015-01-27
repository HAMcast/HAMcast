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

#ifndef MCSOCKET_H_
#define MCSOCKET_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <time.h>
#include <string>

///@author Sebastian Woelke
///@brief socket for multicast applications

/*
 * ##--Multicast Addresses--##
 * 224.0.0.0 - 224.255.255.255 ==> für Routingprotokolle reserviert
 * 239.0.0.0 - 239.255.255.255 ==> für scoping reserviert
 * 225.x.x.x bis 238.x.x.x     ==> frei verfügbar
 */

//IPv4 addresses that are reserved for IP multicasting (http://en.wikipedia.org/wiki/Multicast_address)
#define IPV4_ALL_HOST_ADDR 					"224.0.0.1" 	//The All Hosts multicast group that contains all systems on the same network segment
#define IPV4_ALL_IGMP_ROUTERS_ADDR                "224.0.0.2"	//The All Routers multicast group that contains all routers on the same network segment
#define IPV4_ALL_SPF_ROUTER_ADDR                  "224.0.0.5"	//The Open Shortest Path First (OSPF) AllSPFRouters address. Used to send Hello packets to all OSPF routers on a network segment
#define IPV4_ALL_D_Routers_ADDR                   "224.0.0.6"	//The OSPF AllDRouters address. Used to send OSPF routing information to OSPF designated routers on a network segment
#define IPV4_RIPV2_ADDR						"224.0.0.9"	//The RIP version 2 group address. Used to send routing information using the RIP protocol to all RIP v2-aware routers on a network segment
#define IPV4_EIGRP_ADDR						"224.0.0.10"	//EIGRP group address. Used to send EIGRP routing information to all EIGRP routers on a network segment
#define IPV4_PIMv2_ADDR						"224.0.0.13"	//PIM Version 2 (Protocol Independent Multicast)
#define IPV4_VRR_ADDR						"224.0.0.18"	//Virtual Router Redundancy Protocol
#define IPV4_IS_IS_OVER_IP_19_ADDR                "224.0.0.19"	//IS-IS over IP
#define IPV4_IS_IS_OVER_IP_20_ADDR                "224.0.0.20"	//IS-IS over IP
#define IPV4_IS_IS_OVER_IP_21_ADDR                "224.0.0.21"	//IS-IS over IP
#define IPV4_IGMPV3_ADDR                          "224.0.0.22"	//IGMP Version 3 (Internet Group Management Protocol)
#define IPV4_HOT_STANDBY_ROUTERV2_ADDR            "224.0.0.102"	//Hot Standby Router Protocol Version 2
#define IPV4_MCAST_DNS_ADDR					"224.0.0.251"	//Multicast DNS address
#define IPV4_LINK_LOCAL_MCAST_NAME_RES_ADDR       "224.0.0.252"	//Link-local Multicast Name Resolution address
#define IPV4_NTP_ADDR						"224.0.1.1"	//Network Time Protocol address
#define IPV4_CISCO_AUTO_RP_ANNOUNCE_ADDR          "224.0.1.39"	//Cisco Auto-RP-Announce address
#define IPV4_CISCO_AUTO_RP_DISCOVERY_ADDR         "224.0.1.40"	//Cisco Auto-RP-Discovery address
#define IPV4_H_323_GETEKEEPER_DISC_ADDR           "224.0.1.41"	//H.323 Gatekeeper discovery address

//IPv6 addresses that are reserved for IP multicasting (http://www.iana.org/assignments/ipv6-multicast-addresses/)
#define IPV6_ALL_NODES_ADDR                       "ff02::1"	      //All nodes on the local network segment (equivalent to the IPv4 link-local broadcast address, 169.254.255.255)
#define IPV6_ALL_LINK_LOCAL_ROUTER                "ff02::2"	      //All routers on the link local network segment
#define IPV6_ALL_SITE_LOCAL_ROUTER                "ff05::2"       //All routers on the site local network segment [RFC4291]
#define IPV6_ALL_MLDv2_CAPABLE_ROUTERS            "ff02::16"      //All MLDv2-capable routers [RFC3810]
#define IPV6_ALL_PIM_ROUTERS                      "ff02::d"       //All PIM Routers


#define MC_SCOKET_IF_CHOOSE_INIT -1

std::string ipAddrResolver(std::string ipAddr);


//Multicast Socket
class mc_socket {
protected:
     int m_sock;// (Socket-Deskriptor)
     int m_addrFamily; //AF_INET | AF_INET6
     int m_ifIndex;
     bool m_own_socket;

     //contain important resource
     mc_socket(const mc_socket &copy);
public:
     mc_socket();

     /**
      * @brief close datagram socket automaticly
      */
     virtual ~mc_socket();

     /**
      * @brief create IPv4 datagram socket
      */
     virtual bool create_UDP_IPv4_Socket();

     /**
      * @brief create IPv6 datagram socket
      */
     virtual bool create_UDP_IPv6_Socket();

     /**
      * @brief set to an extern socket
      */
     bool set_own_socket(int socket, int addr_family);

     /**
      * @return address family (AF_INET | AF_INET6)
      */
     int get_addr_family();

     /**
      * @brief bind IPv4 or IPv6 socket on a spezific port
      */
     bool bind_UDP_Socket(int port);

     /**
      * @brief enable or disable loopback for sending multicast packets
      */
     bool setLoopBack(bool enable);

     ///**
     // * @brief set the socket option for receive all mutlicast packet only used for ipv4
     // */
     //bool set_recv_all_mc(const char* if_name);

     /**
      * @brief send a string
      * @param char* addr: IPv4 / IPv6 (create_UDP_IPv4_Socket / create_UDP_IPv6_Socket) address in cleartext
      */
     bool send_Packet(const char* addr, int port, std::string data);

     /**
      * @brief send a Packet to addr
      * @param char* addr: IPv4 / IPv6 (create_UDP_IPv4_Socket / create_UDP_IPv6_Socket) address in cleartext
      */
     bool send_Packet(const char* addr, int port, const unsigned char* data, unsigned int data_size);

     /**
      * @brief receive a datagram
      * @param unsigned char* buf: read N bytes into BUF from socket
      *        int sizeOfBuf: size of buf
      *        int &sizeOfInfo: filled with the effective packet length less then sizeOfBuf
      */
     bool receive_Packet(unsigned char* buf, int sizeOfBuf, int &sizeOfInfo);

     /**
      * @brief set a receive timeout
      */
     bool set_receive_timeout(long msec);

     /**
      * @brief choose a spezific network interface
      * @param char* interface: in cleartext (IPv4 or IPv6)
      *        char* interface: IPv4 ==> spezific network interface address , IPv6 ==> interface name
      */
     bool chooseIf(const char* interface);

     /**
      * @brief join a multicast group on a spezific network interface
      * @param char* addr: group to joind in cleartext (IPv4 or IPv6)
      *        char* interface: IPv4 ==> spezific network interface address , IPv6 ==> interface name
      */
     bool joinGroup(const char* addr, const char* interface);

     /**
      * @brief leave a multicast group on a spezific network interface
      * @param char* addr: group to leave in cleartext (IPv4 or IPv6)
      *        char* interface: IPv4 ==> spezific network interface address , IPv6 ==> interface name (such as "eth0")
      */
     bool leaveGroup(const char* addr, const char* interface);

     /**
      * @brief check for valid socket deskriptor
      */
     bool is_udp_valid() {
          return m_sock > 0;
     }

};

#endif /* MCSOCKET_H_ */
