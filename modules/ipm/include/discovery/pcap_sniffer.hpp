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

#ifndef PCAPSNIFFER_H_
#define PCAPSNIFFER_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pcap.h>
#include <string>

///@author Sebastian Woelke
///@brief wrapper for libcap

#define LOGMSG 1 // sollen status nachrichten ausgegeben werden
#define SIZE_ETHERNET 14 // laenge des erthnet headers
#define ENDLESS -1 //number of packets to sniff

class pcap_sniffer {
private:
     bool m_init_done;
     bool m_filterAllocated;
     int m_packet_len; //default snap length (maximum bytes per packet to capture)
     struct bpf_program m_fp; /* compiled filter program (expression) */
     bpf_u_int32 m_mask; /* subnet mask */
     bpf_u_int32 m_net; /* ip */
     char m_errbuf[PCAP_ERRBUF_SIZE]; /* error buffer */
     pcap_t *m_handle; /* packet capture handle */
public:
     pcap_sniffer();

     /**
      * @brief close pcap sniffer and free core filter
      */
     ~pcap_sniffer();

     /**
      * @brief initialize pcap sniffer
      * @param char* dev: select a network interface (such as "eth0")
      *        int packet_len: default snap length (maximum bytes per packet to capture)
      */
     bool init_pcapSniffer(char* dev, int packet_len);

     /**
      * @brief compile filter expression
      * @param string* filter_exp: see @link http://www.manpagez.com/man/7/pcap-filter/
      */
     bool setfilter(std::string* filter_exp);

     /**
      * @brief receive a single network packet include ethernet header
      */
     bool receive_single_Pcap_Packet(unsigned char* buf, unsigned int &sizeOfInfo); //buf size = packet_len

     /**
      * @brief receive network packets until an event has triggered
      *
      * if a network packet received receive_multi_Pcap_Paket call the function callBackF
      *
      * events: stop_multi_receive()
      *         number of received packets greater then num_packets
      *
      * @param int num_packets: number of maxium received network packets, ENDLESS (-1) means no packet limit
      *        pcap_handler callBackF: callback function
      *                                    defined as void function(u_char *user, const struct pcap_pkthdr *header, const u_char *packet)
      *
      *        u_char *arg: parameter for the callback function
      */
     bool receive_multi_Pcap_Paket(int num_packets, pcap_handler callBackF, u_char *arg);

     /**
      * @brief stop receive_multi_Pcap_Paket()
      */
     void stop_multi_receive();

     /**
      * @brief dealocate all pcap resources
      */
     void close_pcapSniffer();
};

#endif /* PCAPSNIFFER_H_ */
