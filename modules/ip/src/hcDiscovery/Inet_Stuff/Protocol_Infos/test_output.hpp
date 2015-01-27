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

#ifndef TESTOUTPUT_H_
#define TESTOUTPUT_H_

///@author Sebastian Woelke
///@brief Debug output for internet packets

class test_output {
private:
public:

     /**
      * @brief print the packet buf in wireshark style
      */
     static void printBuf(const unsigned char * buf, unsigned int size);

     /**
      * @brief print IPv4 header in cleartext
      */
     static void printPacket_IPv4_Infos(const unsigned char * buf, unsigned int size);

     /**
      * @brief print UDP header in cleartext
      * @param buf musst contain IPv4 header
      */
     static void printPacket_IPv4_UdpInfos(const unsigned char * buf);

     /**
      * @brief print IGMP header in cleartext
      * @param buf musst contain IPv4 header
      */
     static void printPaket_IPv4_IgmpInfos(const unsigned char * buf);

     /**
      * @brief print IPv6 header in cleartext and nextheader infos
      */
     static void printPacket_IPv6_Infos(const unsigned char * buf);

     /**
      * @brief print IPv6 header in cleartext and nextheader infos
      */
     static void printPacket_IPv6_IcmpInfos(const unsigned char * buf);

};
#endif /* TESTOUTPUT_H_ */
