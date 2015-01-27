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

#include "hcDiscovery/Inet_Stuff/Protocol_Infos/test_output.hpp"
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <netinet/igmp.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>



//#include <iostream>
//using namespace std;

void test_output::printBuf(const unsigned char * buf, unsigned int size) {

     for (unsigned int i = 0; i < size; i += 16) {
          for (unsigned int j = i; j < 16 + i && j < size; j++) {

               if (j % 8 == 0 && j % 16 != 0 && j != 0) {
                    printf(" ");
               }

               if (buf[j] == 0) {
                    printf("00 ");
               } else if (buf[j] < 16 && buf[j] > 0) {
                    printf("0%X ", buf[j]);
               } else {
                    printf("%X ", buf[j]);
               }
          }

          printf("   ");

          for (unsigned int j = i; j < 16 + i && j < size; j++) {

               if (j % 8 == 0 && j % 16 != 0 && j != 0) {
                    printf(" ");
               }

               if (buf[j] == 0) {
                    printf(".");
               } else {
                    printf("%c", buf[j]);
               }
          }

          printf("\n");
     }
     printf("\n");
}

void test_output::printPacket_IPv4_Infos(const unsigned char* buf, unsigned int size) {
     if (sizeof(struct ip) <= size) {
          struct ip* iphdr;
          struct in_addr addr;
          iphdr = (struct ip*) buf;
          printf("--IP-Header-- \n");
          printf("version: %d\n", iphdr->ip_v);
          printf("ip Hdr len: %d\n", iphdr->ip_hl*4);
          printf("tos: %d\n", iphdr->ip_tos);
          printf("total Length: %d\n", ntohs(iphdr->ip_len));
          printf("identification: %d\n", ntohs(iphdr->ip_id));
          printf("don't fragment: %d\n", (ntohs(iphdr->ip_off) & IP_DF)>0);
          printf("more fragments: %d\n", (ntohs(iphdr->ip_off) & IP_MF)>0);
          printf("fragment offset: %d\n", ntohs(iphdr->ip_off) & IP_OFFMASK);
          printf("time To live: %i\n", iphdr->ip_ttl);
          printf("protocol: %d (0x%X)\n", iphdr->ip_p,iphdr->ip_p);
          addr = iphdr->ip_src;
          printf("Source: %s\n", inet_ntoa(addr));
          addr = iphdr->ip_dst;
          printf("Destination: %s\n", inet_ntoa(addr));
     }else{
          printf("Packet zu klein: %d\n", size);
     }
}

void test_output::printPacket_IPv4_UdpInfos(const unsigned char* buf) {
     struct ip* iphdr;
     iphdr = (struct ip*) buf;

     struct udphdr *udp;
     udp = (struct udphdr*) (iphdr->ip_hl * 4 + buf);
     printf("--UDP-Header--\n");
     printf("Quellport: %d\n", ntohs(udp->source));
     printf("Zielport: %d\n", ntohs(udp->dest));
     printf("Länge der Nutzlast: %d Bytes\n", (int) (ntohs(udp->len) - sizeof(struct udphdr)));
}

void test_output::printPaket_IPv4_IgmpInfos(const unsigned char* buf) {
     struct ip* iphdr;
     iphdr = (struct ip*) buf;
     if(iphdr->ip_p == IPPROTO_IGMP){
          struct igmp *igmphdr;
          igmphdr = (struct igmp*) (iphdr->ip_hl * 4 + buf);
          printf("--Igmp-Header--\n");
          printf("Typ: 0x%X ", igmphdr->igmp_type);

          switch (igmphdr->igmp_type) {
          case 0x11:
               printf("<Membership Query>");
               break;
          case 0x16:
               printf("<Version 2 Membership Report>");
               break;
          case 0x17:
               printf("<Leave Group>");
               break;
          case 0x12:
               printf("<Version 1 Membership Report>");
               break;
          }
          printf("\n");

          printf("Max Resp Time: %d sec\n", igmphdr->igmp_code / 10);
          printf("Checksum: 0x%X\n", ntohs(igmphdr->igmp_cksum));
          printf("Group Address %s\n", inet_ntoa(igmphdr->igmp_group));
     }else{
          printf("non igmp packet!");
     }
}

void test_output::printPacket_IPv6_Infos(const unsigned char * buf){
     struct ip6_hdr* ipv6Hdr = (struct ip6_hdr*) buf;
     int n = ipv6Hdr->ip6_nxt;
     struct ip6_ext* nextHdr = (struct ip6_ext*)((char*)ipv6Hdr + sizeof(struct ip6_hdr));

     char ipBuffer[INET6_ADDRSTRLEN];

     printf("--IPv6 Header-- :: %p\n", ipv6Hdr);
     printf("Flow Label: \t%d\n", ipv6Hdr->ip6_flow);
     printf("Payload Length: \t%d\n", ipv6Hdr->ip6_plen);
     printf("Next Header: \t%d\n", ipv6Hdr->ip6_nxt);
     printf("Hop Limit: \t%d\n", ipv6Hdr->ip6_hops);

     if(inet_ntop(AF_INET6, &(ipv6Hdr->ip6_src),ipBuffer,sizeof(ipBuffer)) == NULL){
          printf("Error failed to convert src addr\n");
          return;
     }
     printf("Source Address: \t%s\n", ipBuffer);

     if(inet_ntop(AF_INET6, &(ipv6Hdr->ip6_dst),ipBuffer,sizeof(ipBuffer)) == NULL){
          printf("Error failed to convert dst addr\n");
          return;
     }
     printf("Destination Address: \t%s\n", ipBuffer);


     while(1){
          switch(n){
          case IPPROTO_HOPOPTS:
               printf("--Next Header <Hop-by-Hop options (%d)>-- :: %p\n", IPPROTO_HOPOPTS, nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_ROUTING:
               printf("--Next Header <Routing (%d)>-- :: %p\n",IPPROTO_ROUTING, nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_FRAGMENT:
               printf("--Next Header <Fragment (%d)>-- :: %p\n",IPPROTO_FRAGMENT, nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_DSTOPTS:
               printf("--Next Header <ip6_dest (%d)>-- :: %p\n",IPPROTO_DSTOPTS,nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        default:
               printf("--Next Header <Unknown>-- :: %p\n\n",nextHdr);
               return;
          }
     }

}

void test_output::printPacket_IPv6_IcmpInfos(const unsigned char * buf){
     struct ip6_hdr* ipv6Hdr = (struct ip6_hdr*) buf;
     int n = ipv6Hdr->ip6_nxt;
     struct ip6_ext* nextHdr = (struct ip6_ext*)((char*)ipv6Hdr + sizeof(struct ip6_hdr));

     printf("--IPv6 Header-- :: %p\n", ipv6Hdr);

     while(1){
          switch(n){
          case IPPROTO_HOPOPTS:
               printf("--Next Header <Hop-by-Hop options (%d)>-- :: %p\n", IPPROTO_HOPOPTS, nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_ROUTING:
               printf("--Next Header <Routing (%d)>-- :: %p\n",IPPROTO_ROUTING, nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_FRAGMENT:
               printf("--Next Header <Fragment (%d)>-- :: %p\n",IPPROTO_FRAGMENT, nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_DSTOPTS:
               printf("--Next Header <ip6_dest (%d)>-- :: %p\n",IPPROTO_DSTOPTS,nextHdr);
               printf("len:\t%d\n", nextHdr->ip6e_len);
               printf("nextHdr:\t%d\n",nextHdr->ip6e_nxt);
               n = nextHdr->ip6e_nxt;
               nextHdr = (struct ip6_ext*)((char*)nextHdr + (nextHdr->ip6e_len+1)*8 );
               break;
        case IPPROTO_ICMPV6:
               {
                    printf("--Next Header <ICMPv6 (%d)>-- :: %p\n",IPPROTO_ICMPV6,nextHdr);

                    struct icmp6_hdr* icmpHdr = (struct icmp6_hdr*) nextHdr;
                    const char* str=0;

                    switch(icmpHdr->icmp6_type){
                    case 130:
                         str = "Multicast Listener Query";
                         break;
                    case 131:
                         str = "Version 1 Multicast Listener Report";
                         break;
                    case 132:
                         str = "Multicast Listener Done";
                         break;
                    case 143:
                         str = "Version 2 Multicast Listener Report";
                         break;
                    default:
                         str = "Unknown";
                    }

                    printf("type:\t%d <%s>\n", icmpHdr->icmp6_type, str);
                    printf("code:\t%d\n", icmpHdr->icmp6_code);
                    return;
                    break;
               }
        default:
               printf("--Next Header <Unknown>-- :: %p\n\n",nextHdr);
               return;
          }

     }

}

