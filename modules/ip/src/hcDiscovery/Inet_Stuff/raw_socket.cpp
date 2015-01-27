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
#include <string>
#include "raw_socket.hpp"
#include <iostream>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>


using namespace std;

raw_socket::raw_socket() :
          m_raw_sock(0) {
}
//IPPROTO_IP | IPPROTO_ICMP | IPPROTO_TCP | IPPROTO_UDP
bool raw_socket::create_Raw_IPv4_Socket(int ipProto) {
     HC_LOG_TRACE("");

     m_raw_sock = socket(AF_INET, SOCK_RAW, ipProto);
     if (m_raw_sock < 0) {
          HC_LOG_ERROR("failed to create! Error: " << strerror(errno));
          return false; // failed
     } else {
          return true;
     }
}

bool raw_socket::bind_Raw_Socket() {
     HC_LOG_TRACE("");
     if (!is_raw_valid()) {
          if (LOGMSG)
               cout << "bind_Raw_Socket:raw_socket invalid" << endl;
          return false;
     }
     sockaddr_in m_addr;
     int rc;

     m_addr.sin_family = AF_INET;
     m_addr.sin_addr.s_addr = INADDR_ANY;
     m_addr.sin_port = htons(0); // 0 für alle ports

     rc = bind(m_raw_sock, (struct sockaddr *) &m_addr, sizeof(m_addr));
     if (rc == -1) {
          if (LOGMSG)
               cout << "bind_Raw_Socket: failed to bind! Error: " << strerror(errno) << endl;
          return false;
     } else {
          return true;
     }
}

bool raw_socket::setLoopBack(bool enabled) {
     HC_LOG_TRACE("");
     if (!is_raw_valid()) {
          if (LOGMSG)
               cout << "receive_Raw_Packet:raw_socket invalid" << endl;
          return false;
     }

     int rc;

     u_char loop;
     if (enabled == true) {
          loop = 1;
     } else {
          loop = 0;
     }
     rc = setsockopt(m_raw_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

     if (rc == -1) {
          if (LOGMSG)
               cout << "setLoopBack:failed to setLoopBack(on/off)! Error: " << strerror(errno) << endl;
          return false;
     } else {
          return true;
     }
}

bool raw_socket::receive_Raw_Packet(unsigned char* buf, int sizeOfBuf, int &sizeOfInfo, string ifaddr) {
     HC_LOG_TRACE("");

     if (!is_raw_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int rc;
     unsigned int fromlen;
     struct sockaddr_in from;
     struct hostent *he;
     if ((he = gethostbyname(ifaddr.c_str())) == NULL) {
          HC_LOG_ERROR("failed to convert Interface (gethostbyname)");
     }

     from.sin_port = htons(0);
     from.sin_addr = *((struct in_addr *) he->h_addr);

     fromlen = sizeof(from);

     //rc = recv(raw_sock, buf, sizeOfBuf, 0);

     rc = recvfrom(m_raw_sock, buf, sizeOfBuf, 0, (struct sockaddr *) &from, &fromlen);

     sizeOfInfo = rc;
     if (rc == -1) {
          HC_LOG_ERROR("failed to receive! Error: " << strerror(errno));
          return false; //failed to  send
     } else {
          return true;
     }

}

bool raw_socket::send_Raw_Packet(unsigned char* buf, int sizeOfBuf, int port, string& ip){
     HC_LOG_TRACE("");

     if (!is_raw_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     struct sockaddr_in sin;
     sin.sin_family = AF_INET;
     sin.sin_port = htons (port);
     sin.sin_addr.s_addr = inet_addr (ip.c_str());

     if (sendto(m_raw_sock, buf, sizeOfBuf, 0,(struct sockaddr *) &sin, sizeof (sin)) < 0){
          HC_LOG_ERROR("failed to send row packet! Error: " << strerror(errno));
          return false;
     } else{
          return true;
     }

}

bool raw_socket::setNoIP_Hdr(){
     HC_LOG_TRACE("");

     if (!is_raw_valid()) {
          HC_LOG_ERROR("raw_socket invalid");
          return false;
     }

     int one = 1;
     if (setsockopt (m_raw_sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof (one)) < 0){
          HC_LOG_ERROR("failed to set no ip header! Error: " << strerror(errno));
     }

     return true;
}

raw_socket::~raw_socket() {
     HC_LOG_TRACE("");
     if (is_raw_valid()) {
          close(m_raw_sock);
     }
}

/*
//this function generates header checksums
unsigned short csum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}
*/
