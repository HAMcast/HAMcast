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

#ifndef BASIC_DISCOVERY_H_
#define BASIC_DISCOVERY_H_

#include "hcDiscovery/Inet_Stuff/if_prop.hpp"
#include "hcDiscovery/Inet_Stuff/pcap_sniffer.hpp"

#include <tr1/memory>
#include <string>
#include <netinet/ip.h>
#include <netinet/ip6.h>

#include <boost/thread.hpp>
#include <boost/thread/pthread/mutex.hpp>
#include <boost/thread/barrier.hpp>

using namespace std;
//using namespace std::tr1;

///@author Sebastian Woelke
///@brief generic discovery protocol engine

/**
 * @brief possible protocol type
 */
enum dis_proto_type {
     INIT_PROTO, IGMPvX, IGMPv1, IGMPv2, IGMPv3, MLDvX, MLDv1, MLDv2, PIMv1, PIMv2, NTPv1, NTPv2, DVMRP
          };

string protoResolver(dis_proto_type i);

/**
 * @brief filter type for aktive protocol discovery and passive protocol discovery
 */
enum filter_type{
     ACTIVE, PASSIVE, INIT_FILTER
          };

/**
 * @brief Functor for delete an array in a smartpointer
 */
template<typename T>
struct ArrayDeleter{
     void operator ()(T* ptr){
          delete [] ptr;
     }
};

/**
 * @brief hold information about a network protocol
 */
class dis_proto {
private:
     dis_proto_type m_type;
     struct sockaddr_storage m_hostAddr;
     std::tr1::shared_ptr<u_char> m_rawData;
     int m_rawData_size;

     friend class service_discovery;
public:
     dis_proto(dis_proto_type typ, struct sockaddr_storage hostAddr);
     dis_proto();
     ~dis_proto();

     /**
      * @brief set protocol type
      */
     void setType(dis_proto_type type);

     /**
      * @brief set host address
      */
     void setHostAddr(struct sockaddr_storage& hostAddr);

	 /**
      * @return host address as string
      */
     string getHostAddr();
	 
     /**
      * @brief save raw packet data
      */
     void savePacketRawData(const u_char* rawData, int size);

     /**
      * @brief has saved raw packet data
      */
     bool hasRawData();

     /**
      * @brief return packet raw data
      */
     u_char* getRawData();

     /**
      * @brief return packet raw data size
      */
     int getRawData_size();

     /**
      * @brief return address family (AF_INET or AF_INET6)
      */
     int getAddrFamily();

     /**
      * @return enum dis_proto_type
      */
     dis_proto_type getType();
};

class basic_dis_interface {
public:
     /**
      * @brief non blocking function to discover a network protocol on an spezific network interface
      * @param ifreq *item: network interface to sniff on
      *        if_prop* ifInfo: for network interface properties request
      */
     virtual bool sniffPassiv(ifreq *item, if_prop* ifInfo)=0;

     /**
      * @brief non blocking function to discover a network protocol on an spezific network interface
      * @param ifreq *item: network interface to sniff on
      *        if_prop* ifInfo: for network interface properties request
      */
     virtual bool sniffActive(ifreq *item, if_prop* ifInfo)=0;

     /**
      * @brief blocked the calling thread until the pcap sniffer has terminated
      */
     virtual void joinSniffer()=0;

     /**
      * @brief terminat pcap sniffer
      */
     virtual void stopSearching()=0;

     /**
      * @brief return true when a spezific network protocol is found
      */
     virtual bool foundProtocol()=0;

     /**
      * @return founded protocol
      */
     virtual dis_proto getfoundProtocol()=0;

};



class basic_dis: public basic_dis_interface {
private:
     dis_proto_type m_protocolType;
     int m_addrFamily; //AF_INET or AF_INET6
     bool m_saveRawData;
     bool m_hasEthernetFrame;

     filter_type m_filtertype; //Pcap Filtertype (ACTIVE | PASSIVE)

     pcap_sniffer m_pcapS; //Picap sniffer instance

     unsigned int m_timeOut; //time in second

     dis_proto m_discoveredProtocol; //on Success return a class protocol inforamtion else NULL
     bool m_protocolFound;

     boost::barrier* m_startbarrier; //started thread_TimeOut and thread_PcapLoop synchronised

     boost::thread* m_p_TimeOut; //TimeOut Thread
     boost::thread* m_p_PcapLoop; //TimeOut Thread

     static void thread_TimeOut(void *arg); //stopp pcapLoop after defined time
     static void thread_PcapLoop(void *arg);

     void closeSniffer();
protected:

     /**
      * @brief standard callback function from pcaplibrary
      *
      * sniff all nonfilterd packets
      * this packets will checked by an extern filter function for protocol spezific packets
      *
      * @if is a protocol spezific packet found
      *         then it stop the pcap sniffer and create a @class dis_proto with additional information
      *
      * @param u_char *arg contain an instance of @class basic_dis
      *        struct pcap_pkthdr *header is not used
      *        u_char *packet contain the sniffed network packet
      */
     static void pcapLoop_Callback(u_char *arg, const struct pcap_pkthdr *header, const u_char *packet);

     /**
      * @return set address family
      */
     int getAddrFamily();

     /**
      * @return set protocol type to look for
      */
     void setProtocolType(dis_proto_type type);

     /**
      * @brief initializes pcap sniffer and timeout thread
      *
      * @param ifreq *item: network interface to sniff on
      *        CPPTime timeOut: stops the pcap sniffer after this relative time
      *        string* pcapFilter: pcap core filter expression
      *        filter_type filterType: ACTIVE or PASSIVE
      *        int maxPacketLen: max allocated length of a to sniffed packet
      *        int addrFamily: AF_INET or AF_INET
      *        bool saveRawData: save raw packet data to dis_proto
      */
     bool initSniffer(ifreq *item, if_prop* ifInfo, unsigned int timeOut, string* pcapFilter, filter_type filterType, int maxPacketLen, int addrFamily, bool saveRawData);

     /**
      * @brief start pcap sniffer
      */
     void startSniffer();

     /**
      * @brief check the parameter ipvXHdr for spezific protocol details
      * @param u_char* ipvXHdr: contain an IPv4 or an IPv6 packet
      */
     virtual bool ActiveFilter(const u_char* data)=0;

     /**
      * @brief check the parameter ipvXHdr for spezific protocol details
      * @param u_char* ipvXHdr: contain an IPv4 or an IPv6 packet
      */
     virtual bool PassiveFilter(const u_char* data)=0;

     /**
      * @brief launched after stoped sniffing
      */
     virtual void shutdown();
public:
     basic_dis();
     virtual ~basic_dis();

     /**
      * @brief non blocking function to discover a network protocol on an spezific network interface
      * @param ifreq *item: network interface to sniff on
      *        if_prop* ifInfo: for network interface properties request
      */
     //virtual bool sniffPassiv(ifreq *item, if_prop* ifInfo)=0;

     /**
      * @brief non blocking function to discover a network protocol on an spezific network interface
      * @param ifreq *item: network interface to sniff on
      *        if_prop* ifInfo: for network interface properties request
      */
     //virtual bool sniffActive(ifreq *item, if_prop* ifInfo)=0;

     /**
      * @brief blocked the calling thread until the pcap sniffer has terminated
      */
     void joinSniffer();

     /**
      * @brief terminat pcap sniffer
      */
     void stopSearching();

     /**
      * @brief return true when a spezific network protocol is found
      */
     bool foundProtocol();

     /**
      * @return founded protocol
      */
     dis_proto getfoundProtocol();

};

#endif /* BASIC_DISCOVERY_H_ */
