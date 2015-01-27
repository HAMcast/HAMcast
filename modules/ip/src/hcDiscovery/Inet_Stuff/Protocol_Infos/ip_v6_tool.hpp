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

#ifndef IPV6CHAIN_H
#define IPV6CHAIN_H

#include <sys/types.h>
#include <netinet/ip6.h>
#include <string>

#include <boost/asio/ip/address.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/address.hpp>
using namespace std;

///@author Sebastian Woelke
///@brief IPv6 helper

class ip_v6_tool {

public:
     /**
      * @brief track nextheaders from IPv6 header to a spezific protocol
      * @return pointer to the spezific protocol header or return null if the spezific protocol cannot be found
      */
     static u_char* getSpecificHdr(struct ip6_hdr* ipv6Hdr, int Protocol){
          int n= ipv6Hdr->ip6_nxt;
          struct ip6_ext* nextHdr= (struct ip6_ext*)((char*)ipv6Hdr + sizeof(struct ip6_hdr));

          while(1){

               if(n == Protocol){
                    return (u_char*)nextHdr;
                    // possible nextheader
               }else if(n == IPPROTO_HOPOPTS || n == IPPROTO_ROUTING ||
                        n == IPPROTO_FRAGMENT || n == IPPROTO_DSTOPTS){
                    n =  nextHdr->ip6e_nxt;
                    nextHdr =  (struct ip6_ext*)(((char*)nextHdr)+ (nextHdr->ip6e_len+1)*8);
               }else if(n == IPPROTO_NONE){
                    return NULL;
               }else{
                    return NULL;
               }
          }
     }

     /**
      * @brief compare IPv6 addresses
      */
     static bool equalAddr(struct in6_addr* a, struct in6_addr* b){
          for(unsigned int i=0; i<sizeof(a->__in6_u.__u6_addr8);i++){
               if(a->__in6_u.__u6_addr8[i] != b->__in6_u.__u6_addr8[i]){
                    return false;
               }
          }
          return true;
     }

     /**
      * @brief compare IPv4/IPv6 addresses
      */
     static bool equalAddr(string a, string b){
          boost::system::error_code ec;
          boost::asio::ip::address addr1;
          boost::asio::ip::address addr2;
          addr1 =  boost::asio::ip::address::from_string( a , ec );
          if ( ec ){
               return false;
          }
          addr2 =  boost::asio::ip::address::from_string( b , ec );
          if ( ec ){
               return false;
          }
          return addr1 == addr2;
     }




};

#endif // IPV6CHAIN_H
