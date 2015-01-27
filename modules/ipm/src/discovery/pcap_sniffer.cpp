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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pcap.h>
#include <string>
#include <stdio.h>

#include "hamcast/hamcast_logging.h"
#include "discovery/pcap_sniffer.hpp"

using std::string;

pcap_sniffer::pcap_sniffer() :
          m_init_done(false), m_filterAllocated(false) {
     HC_LOG_TRACE("");
}

bool pcap_sniffer::init_pcapSniffer(char* dev, int p_len) {
     HC_LOG_TRACE("");

     m_packet_len = p_len;

     /* get network number and mask associated with capture device */
     if (pcap_lookupnet(dev, &m_net, &m_mask, m_errbuf) == -1) {
          HC_LOG_ERROR("failed to get netmask for device " << dev << ": " << m_errbuf);

          m_net = 0;
          m_mask = 0;

          return false;
     }

     /* open capture device */
     m_handle = pcap_open_live(dev, m_packet_len, 1, 1000, m_errbuf);
     if (m_handle == NULL) {
          HC_LOG_ERROR("failed to open device " << dev << ": " << m_errbuf);
          return false;
     }

     m_init_done = true;
     return true;
}

void pcap_sniffer::close_pcapSniffer(){
     HC_LOG_TRACE("");

     if (m_init_done) {
          if(m_filterAllocated){
               pcap_freecode(&m_fp);
          }
          pcap_close(m_handle);
     }

     m_init_done = false;
     m_filterAllocated = false;
}


pcap_sniffer::~pcap_sniffer() {
     HC_LOG_TRACE("");
     close_pcapSniffer();
}

bool pcap_sniffer::setfilter(string* filter_exp) {
     HC_LOG_TRACE("");

     /* compile the filter expression */
     if (pcap_compile(m_handle, &m_fp, filter_exp->c_str(), 0, m_net) == -1) {
          HC_LOG_ERROR("failed to parse filter " << filter_exp->c_str() << ": " << pcap_geterr(m_handle));
          return false;
     }else{
          m_filterAllocated = true;
     }

     /* apply the compiled filter */
     if (pcap_setfilter(m_handle, &m_fp) == -1) {
          HC_LOG_ERROR("failed to install filter " << filter_exp->c_str() << ": " << pcap_geterr(m_handle));
          return false;
     }
     return true;
}

bool pcap_sniffer::receive_single_Pcap_Packet(unsigned char* buf, unsigned int &sizeOfInfo) {
     HC_LOG_TRACE("");

     struct pcap_pkthdr h;
     const u_char *tmp = pcap_next(m_handle, &h);
     sizeOfInfo = h.len;
     for (unsigned int i = 0; i < h.len; i++) {
          buf[i] = tmp[i];
     }
     return true;
}

bool pcap_sniffer::receive_multi_Pcap_Paket(int num_packets, pcap_handler callBackF, u_char *arg) {
     HC_LOG_TRACE("");

     int tmp = pcap_loop(m_handle, num_packets, callBackF, arg);
     if (tmp == 0) {
          return true;
     } else if (tmp == -1) {
          HC_LOG_ERROR(" pcap_loop Error");
          return false;
     } else if (tmp == -2) {
          return true;
     } else {
          HC_LOG_ERROR("Error: " << tmp);
          return false;
     }
}

void pcap_sniffer::stop_multi_receive() {
     HC_LOG_TRACE("");
     pcap_breakloop(m_handle);
}
