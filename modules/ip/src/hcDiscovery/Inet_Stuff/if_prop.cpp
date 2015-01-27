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
#include "hcDiscovery/Inet_Stuff/if_prop.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include <sys/ioctl.h>
#include <fstream>
#include <ifaddrs.h>

using namespace std;

const char* getS(int status) { //get Status
     HC_LOG_TRACE("");

     static string strue = "true";
     static string sfalse = "false";
     if (status) {
          return strue.c_str();
     } else {
          return sfalse.c_str();
     }
}

void if_prop::printIfInfo() {
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          cout << "printIfInfo:sck invalid" << endl;
          return;
     }
     cout << "--Network Interfaces--" << endl;
     cout << "found " << m_nInterfaces << " Interface[s]" << endl << endl;

     char* addr;

     /* Iterate through the list of interfaces. */
     for (unsigned int i = 0; i < m_nInterfaces; i++) {
          struct ifreq *item = &m_ifr[i];
          printf("%s\n", item->ifr_ifrn.ifrn_name);
          //print IPv4-addr
          addr = inet_ntoa(((struct sockaddr_in *) &item->ifr_addr)->sin_addr);
          printf("\tIP:\t\t%s\n", addr);
          //print IPv6 addr

          //print Mac
          if (getHwAddr(item)) { //load hardware address
               addr = (char*) &(item->ifr_ifru.ifru_hwaddr.sa_data);

               printf("addr Famalie %d\n", item->ifr_ifru.ifru_hwaddr.sa_family);

               printf("\tMAC:\t\t");
               for (int j = 0; j < IFHWADDRLEN; j++) {
                    printf("%2.2x ", (unsigned char) addr[j]);
               }
               printf("\n");
          }

          //print Falgs auswerten
          if (getFlags(item)) { //load flags
               printf("\tup:\t\t%s\n", getS(item->ifr_flags & IFF_UP));
               printf("\trunning:\t%s\n", getS(item->ifr_flags & IFF_RUNNING));
               printf("\tloopback:\t%s\n", getS(item->ifr_flags & IFF_LOOPBACK));
               printf("\tall multi:\t%s\n", getS(item->ifr_flags & IFF_ALLMULTI));
               printf("\tmulticast:\t%s\n", getS(item->ifr_flags & IFF_MULTICAST));
          }

          printf("\n");
     }
}

void if_prop::print_Address_list(){
     HC_LOG_TRACE("");

     //--getifaddrs access--
     struct ifaddrs* ifa=NULL,*ifEntry=NULL;
     void* addPtr = NULL;
     int rc = 0;
     char addressBuffer[INET6_ADDRSTRLEN];

     rc = getifaddrs(&ifa);
     if (rc==0) {
          for(ifEntry=ifa; ifEntry!=NULL; ifEntry=ifEntry->ifa_next) {
               if(ifEntry->ifa_addr->sa_data == NULL) {
                    cout << "no Data" << endl;
                    continue;
               }
               if(ifEntry->ifa_addr->sa_family==AF_INET) {
                    addPtr = &((struct sockaddr_in *)ifEntry->ifa_addr)->sin_addr;
               } else if(ifEntry->ifa_addr->sa_family==AF_INET6) {
                    addPtr = &((struct sockaddr_in6 *)ifEntry->ifa_addr)->sin6_addr;
               } else {
                    //It isn't IPv4 or IPv6
                    cout << "not AF_INET or AF_INET6" << endl;
                    continue;
               }

               const char *a = inet_ntop(ifEntry->ifa_addr->sa_family,
                                         addPtr,
                                         addressBuffer,
                                         sizeof(addressBuffer));
               if(a != NULL) {
                    cout << "ifname: " << ifEntry->ifa_name << endl;
                    cout << "\tip-addr: " << a << endl;
               }
          }
     }
     freeifaddrs(ifa);
}

if_prop::if_prop():
          m_sck(-1), m_ifr(0), m_nInterfaces(0),m_own_socket(false), m_ifa(0)
{
     HC_LOG_TRACE("");
}

if_prop::~if_prop() {
     HC_LOG_TRACE("");

     if (is_sck_valid() && m_own_socket) {
          close(m_sck);
     }

     if(is_ifa_valid()){
          freeifaddrs(m_ifa);
     }
}

bool if_prop::init_IfInfo() {
     HC_LOG_TRACE("");

     // Get a socket handle
     int sck = socket(AF_INET, SOCK_DGRAM, 0);
     if (sck < 0) {
          HC_LOG_ERROR("failed to create Socket! Error: " << strerror(errno));
          return false; // failed
     }

     if(init_IfInfo(sck)){
          m_own_socket = true;
          return true;
     }else{
          return false;
     }
}

bool if_prop::init_IfInfo(int sck){
     HC_LOG_TRACE("");

     if(sck <= 0){
          return false;
     }

     m_sck = sck;

     // Query available interfaces
     m_ifc.ifc_len = sizeof(m_buf);
     m_ifc.ifc_buf = m_buf;

     return true;
}

bool if_prop::refresh_network_interfaces() {
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     // resize len
     m_ifc.ifc_len = sizeof(m_buf);

     if (ioctl(m_sck, SIOCGIFCONF, &m_ifc) < 0) {
          HC_LOG_ERROR("failed to get Interface config! Error: " << strerror(errno));
          close(m_sck);
          m_sck = -1;
          return false;
     }

     m_ifr = m_ifc.ifc_req;
     m_nInterfaces = m_ifc.ifc_len / sizeof(struct ifreq);
     HC_LOG_DEBUG("interface count: " << m_nInterfaces);

     //--getifaddrs access--
     if(is_ifa_valid()){
          freeifaddrs(m_ifa);
     }

     if(getifaddrs(&m_ifa) < 0){
          HC_LOG_ERROR("getifaddrs failed! Error: " << strerror(errno) );
          return false;
     }

     return true;
}


bool if_prop::getAddr(ifreq *item) { //load flags
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFADDR, item) == -1) {
          HC_LOG_ERROR("failed to get Addr! Error: " << strerror(errno));
          return false;
     }
     return true;
}

bool if_prop::getDstAddr(ifreq *item){
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFDSTADDR, item) == -1) {
          HC_LOG_ERROR("failed to get Dst Addr! Error: " << strerror(errno));
          return false;
     }
     return true;
}

bool if_prop::getFlags(ifreq *item) { //load flags
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFFLAGS, item) == -1) {
          HC_LOG_ERROR("failed to get flags! Error: " << strerror(errno));
          return false;
     }
     return true;
}

bool if_prop::setFlags(ifreq *item) { //load flags
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCSIFFLAGS, item) == -1) {
          HC_LOG_ERROR("failed to get flags! Error: " << strerror(errno));
          return false;
     }
     return true;
}

bool if_prop::getHwAddr(ifreq *item) { //load hardware address
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFHWADDR, item) == -1) {
          HC_LOG_ERROR("failed to get hardware address! Error: " << strerror(errno));
          return false;
     }
     return true;
}

bool if_prop::getMask(ifreq* item){
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFNETMASK, item) == -1) {
          HC_LOG_ERROR("failed to get mask! Error: " << strerror(errno));
          return false;
     }

     return true;
}

bool if_prop::getBroadcast(ifreq* item){
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFBRDADDR, item) == -1) {
          HC_LOG_ERROR("failed to get broadcast address! Error: " << strerror(errno));
          return false;
     }

     return true;
}

bool if_prop::getMTU(ifreq* item){
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return false;
     }

     if (ioctl(m_sck, SIOCGIFMTU, item) == -1) {
          HC_LOG_ERROR("failed to get mtu! Error: " << strerror(errno));
          return false;
     }

     return true;
}


unsigned int if_prop::getInterfaceCount() {
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return -1;
     }

     HC_LOG_DEBUG("interface count: " << m_nInterfaces);
     return m_nInterfaces;
}

struct ifreq* if_prop::getInterfaceItem(unsigned int index) {
     HC_LOG_TRACE("");

     //--ioctl access--
     if (!is_sck_valid()) {
          HC_LOG_ERROR("sck invalid");
          return NULL;
     }

     if (index >= m_nInterfaces) {
          HC_LOG_ERROR("wrong index: " << index);
          return NULL;
     }

     return &m_ifr[index];
}

struct ifreq* if_prop::getInterfaceItem(string if_name){
          HC_LOG_TRACE("");

          //--ioctl access--
          if (!is_sck_valid()) {
               HC_LOG_ERROR("sck invalid");
               return NULL;
          }

          struct ifreq* item=NULL;
          bool found_if=false;
          for(unsigned int i=0; i< getInterfaceCount(); i++){
               item = getInterfaceItem(i);
               if(if_name.compare(item->ifr_name) == 0){
                    found_if=true;
                    break;
               }
          }

          if(!found_if || !item){
               HC_LOG_ERROR("if_name: " << if_name << " not found!");
               return NULL;
          }else{
               return item;
          }
}

string if_prop::convertMAC_Addr(ifreq* item){
     HC_LOG_TRACE("");

     //--general--
     char* addr;
     ostringstream str;
     char buffer[4];
     addr = (char*) &(item->ifr_ifru.ifru_hwaddr.sa_data);
     for (int j = 0; j < IFHWADDRLEN-1; j++) {
          snprintf(buffer,sizeof(buffer),"%2.2x:", (unsigned char) addr[j]);
          str << buffer;
     }
     snprintf(buffer,sizeof(buffer),"%2.2x", (unsigned char) addr[IFHWADDRLEN-1]);
     str << buffer;
     return str.str();
}

vector<struct in6_addr*> if_prop::getIPv6Addr(string& ifName){
     HC_LOG_TRACE("ifName: " << ifName);

     //--ioctl access--
     vector<struct in6_addr*> addV;

     if (!is_ifa_valid()) {
          HC_LOG_ERROR("ifa invalid");
          return addV;
     }

     struct ifaddrs* ifEntry=NULL;
     for(ifEntry=m_ifa; ifEntry!=NULL; ifEntry=ifEntry->ifa_next) {
          if(ifEntry->ifa_addr != NULL &&
             ifEntry->ifa_addr->sa_data != NULL &&
             ifEntry->ifa_addr->sa_family==AF_INET6 &&
             strcmp(ifName.c_str(), ifEntry->ifa_name)==0){
               addV.push_back(&((struct sockaddr_in6 *)ifEntry->ifa_addr)->sin6_addr);
          }
     }

     return addV;
}
