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


#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include <cstring>

#include "hamcast/hamcast_logging.h"
#include "discovery/basic_dis.hpp"

//#################################
//####-- enum dis_proto_type --####
//#################################
string protoResolver(dis_proto_type i){
    switch(i){
    case INIT_PROTO: return "INIT_PROTO";
    case IGMPvX: return "IGMPvX";
    case IGMPv1: return "IGMPv1";
    case IGMPv2: return "IGMPv2";
    case IGMPv3: return "IGMPv3";
    case MLDvX: return "MLDvX";
    case MLDv1: return "MLDv1";
    case MLDv2: return "MLDv2";
    case PIMv1: return "PIMv1";
    case PIMv2: return "PIMv2";
    case NTPv1: return "NTPv1";
    case NTPv2: return "NTPv2";

    default: return "ERROR";
    }
}

//###############################
//####-- DiscoveryProtocol --####
//###############################
dis_proto::dis_proto(dis_proto_type type, struct sockaddr_storage hostAddr) :
    m_type(type), m_hostAddr(hostAddr), m_rawData_size(0) {
    HC_LOG_TRACE("");
}

dis_proto::dis_proto() :
    m_type(INIT_PROTO), m_rawData_size(0){
    HC_LOG_TRACE("");
    memset(&m_hostAddr,0,sizeof(m_hostAddr));
}

void dis_proto::setType(dis_proto_type type) {
    HC_LOG_TRACE("");
    this->m_type = type;
}

void dis_proto::setHostAddr(struct sockaddr_storage& hostAddr) {
    HC_LOG_TRACE("");
    this->m_hostAddr = hostAddr;
}

void dis_proto::savePacketRawData(const u_char* rawData, int size){
    HC_LOG_TRACE("");

    this->m_rawData_size = size;
    boost::shared_ptr<u_char> tmp(new u_char[size],ArrayDeleter<u_char>());
    m_rawData = tmp;
    memcpy(m_rawData.get(), rawData, size);
}

bool dis_proto::hasRawData(){
    HC_LOG_TRACE("");
    return m_rawData_size > 0;
}

u_char* dis_proto::getRawData(){
    HC_LOG_TRACE("");
    return m_rawData.get();
}

int dis_proto::getRawData_size(){
    HC_LOG_TRACE("");
    return m_rawData_size;
}

dis_proto_type dis_proto::getType() {
    HC_LOG_TRACE("");
    return m_type;
}

int dis_proto::getAddrFamily(){
    HC_LOG_TRACE("");
    return m_hostAddr.ss_family;
}

string dis_proto::getHostAddr() {
    HC_LOG_TRACE("");
    char buffer[INET6_ADDRSTRLEN];

    //reinterpret_cast<sockaddr_in*>(&sif.m_ifaddr)->sin_addr = reinterpret_cast<struct sockaddr_in*>(&dtItem->m_item->ifr_addr)->sin_addr;
    int af = m_hostAddr.ss_family;
    if (af == AF_INET) {
        if(inet_ntop(AF_INET,(void*)&(((struct sockaddr_in*)(&m_hostAddr))->sin_addr),buffer, sizeof(buffer)) == NULL){
            HC_LOG_ERROR("failed to convert src ip");
            return string();
        }
    }
    else {
        if(inet_ntop(AF_INET6,(void*)&(((struct sockaddr_in6*)(&m_hostAddr))->sin6_addr),buffer, sizeof(buffer)) == NULL){
            HC_LOG_ERROR("failed to convert src ip");
            return string();
        }
    }
    return string(buffer);
}

dis_proto::~dis_proto(){
    HC_LOG_TRACE("");
}

//#############################
//####-- Basic_Discovery --####
//#############################
basic_dis::basic_dis() :
    m_protocolType(INIT_PROTO),
    m_addrFamily(-1),
    m_saveRawData(false),
    m_hasEthernetFrame(false),
    m_filtertype(INIT_FILTER),
    m_timeOut(-1),
    m_protocolFound(false),
    m_startbarrier(NULL),
    m_p_TimeOut(NULL),
    m_p_PcapLoop(NULL)
{
    HC_LOG_TRACE("");
}

basic_dis::~basic_dis(){
    HC_LOG_TRACE("");
    this->closeSniffer();
}

void basic_dis::shutdown(){
    HC_LOG_TRACE("");
}

void basic_dis::pcapLoop_Callback(u_char *arg, const struct pcap_pkthdr *header, const u_char *packet) {
    HC_LOG_TRACE("");
    basic_dis* dis= (basic_dis*)arg;

    const u_char* ipvXHdr;
    bool filterRequest;

    if(dis->m_hasEthernetFrame){
        ipvXHdr = (const u_char*) (packet + SIZE_ETHERNET);
    }else{
        ipvXHdr = packet;
    }

    int packetSize = 0;

    if(header->caplen < header->len){
        packetSize = header->len;
    }else{
        packetSize = header->caplen;
    }

    //checks the paket with definte filter funktion
    if(dis->m_filtertype==ACTIVE){
        filterRequest=(dis->ActiveFilter)(ipvXHdr);
    }else if(dis->m_filtertype==PASSIVE){
        filterRequest=(dis->PassiveFilter)(ipvXHdr);
    }else{
        filterRequest=false;
    }

    if (filterRequest) {
        struct sockaddr_storage src_addr;

        dis->m_pcapS.stop_multi_receive();

        if(dis->getAddrFamily() == AF_INET){
            src_addr.ss_family = dis->getAddrFamily();

            reinterpret_cast<sockaddr_in*>(&src_addr)->sin_addr = ((struct ip*) ipvXHdr)->ip_src;
            //*(struct in_addr*)(&src_addr.__ss_align) = ((struct ip*) ipvXHdr)->ip_src;
        }else if(dis->getAddrFamily() == AF_INET6){
            src_addr.ss_family= dis->getAddrFamily();
            reinterpret_cast<sockaddr_in6*>(&src_addr)->sin6_addr = ((struct ip6_hdr*) ipvXHdr)->ip6_src;
            //*(struct in6_addr*)(&src_addr.__ss_align) = ((struct ip6_hdr*) ipvXHdr)->ip6_src;
        }else{
            HC_LOG_ERROR("wrong addrFamily");
            return;
        }

        dis->m_discoveredProtocol.setHostAddr(src_addr);
        dis->m_discoveredProtocol.setType(dis->m_protocolType);
        dis->m_protocolFound = true;

        if(dis->m_saveRawData){
            dis->m_discoveredProtocol.savePacketRawData(ipvXHdr, packetSize - SIZE_ETHERNET);
        }
    }
}

int basic_dis::getAddrFamily(){
    HC_LOG_TRACE("");
    return m_addrFamily;
}

void basic_dis::setProtocolType(dis_proto_type type){
    HC_LOG_TRACE("");
    this->m_protocolType=type;
}

bool basic_dis::initSniffer(ifreq *item, if_prop* ifInfo, unsigned int timeOut, string* pcapFilter, filter_type filterType, int maxPacketLen, int addrFamily, bool saveRawData) {
    HC_LOG_TRACE("");

    this->closeSniffer(); //maybe someone sniff twice with the same objekt

    this->m_protocolFound = false;

    this->m_addrFamily = addrFamily;

    this->m_saveRawData = saveRawData;

    if(ifInfo->getFlags(item)){
        this->m_hasEthernetFrame = (item->ifr_flags & IFF_POINTOPOINT) == 0;
    }else{
        return false;
    }



    this->m_timeOut = timeOut;
    this->m_filtertype = filterType;
    if(!this->m_pcapS.init_pcapSniffer(item->ifr_name, maxPacketLen)){
        return false;
    }

    if(!this->m_pcapS.setfilter(pcapFilter)){
        return false;
    }

    this->m_startbarrier = new boost::barrier(2); //thread_TimeOut and thread_PcapLoop

    return true;
}

void basic_dis::closeSniffer() {
    HC_LOG_TRACE("");

    delete m_p_TimeOut;
    delete m_p_PcapLoop;
    delete m_startbarrier;

    this->m_pcapS.close_pcapSniffer();

    m_p_TimeOut= NULL;
    m_p_PcapLoop = NULL;
    m_startbarrier = NULL;
}

void basic_dis::startSniffer() {
    HC_LOG_TRACE("");

    m_p_PcapLoop = new boost::thread(basic_dis::thread_PcapLoop, this);
    m_p_TimeOut = new boost::thread(basic_dis::thread_TimeOut,this);
}

void basic_dis::thread_TimeOut(void *arg) {
    HC_LOG_TRACE("");

    basic_dis* dis= (basic_dis*)arg;

    dis->m_startbarrier->wait();

    if(!dis->m_p_PcapLoop->timed_join(boost::posix_time::seconds(dis->m_timeOut))){ ////on timeOut
        dis->m_pcapS.stop_multi_receive(); //stop thread_PcapLoop
        dis->m_p_PcapLoop->join();
    }

    dis->shutdown();
}

void basic_dis::thread_PcapLoop(void *arg) {
    HC_LOG_TRACE("");

    basic_dis* dis= (basic_dis*) arg;

    dis->m_startbarrier->wait();

    dis->m_pcapS.receive_multi_Pcap_Paket(ENDLESS, pcapLoop_Callback, (u_char*)arg);
}

void basic_dis::joinSniffer() {
    HC_LOG_TRACE("");
    if(this->m_p_TimeOut!=NULL){
        this->m_p_TimeOut->join();
    }

}

void basic_dis::stopSearching(){
    HC_LOG_TRACE("");
    this->m_pcapS.stop_multi_receive();
}

bool basic_dis::foundProtocol() {
    HC_LOG_TRACE("");
    return m_protocolFound;
}

dis_proto basic_dis::getfoundProtocol() {
    HC_LOG_TRACE("");
    return m_discoveredProtocol;
}
