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

#ifndef RAWSOCKET_H_
#define RAWSOCKET_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string>

///@author Sebastian Woelke
///@brief wrapper for raw-socket

#define LOGMSG 1 // sollen status nachrichten ausgegeben werden

class raw_socket {
private:
     int m_raw_sock;// raw_Socketnummer
public:
     raw_socket();

     /**
      * @brief close raw socket automaticly
      */
     virtual ~raw_socket();

     /**
      * @brief create IPv4 raw socket
      */
     bool create_Raw_IPv4_Socket(int ipProto);

     /**
      * @brief bind raw sock
      */
     bool bind_Raw_Socket();

     /**
      * @brief enable or disable loopback for sending multicast packets
      */
     bool setLoopBack(bool enabled);

     /**
      * @brief receive a network packet
      * @param unsigned char* buf: read N bytes into BUF from socket
      *        int sizeOfBuf: size of buf
      *        int &sizeOfInfo: filled with the effective packet length less then sizeOfBuf
      *        string ifAddr: choose an spezific network interface
      */
     bool receive_Raw_Packet(unsigned char* buf, int sizeOfBuf, int &sizeOfInfo, std::string ifAddr);

     /**
      * @brief send a raw packet
      */
     bool send_Raw_Packet(unsigned char* buf, int sizeOfBuf,  int port, std::string& ip);

     /**
      * @brief setNoIP_Hdr make sure that the kernel knows the header is included in the data, and doesn't insert its own header into the packet before our data
      */
     bool setNoIP_Hdr();

     /**
      * @brief check for valid socket deskriptor
      */
     bool is_raw_valid() {
          return m_raw_sock > 0;
     }
};

#endif /* RAWSOCKET_H_ */

