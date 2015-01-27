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

#include <errno.h>
#include <fstream>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "defines.hpp"
#include "ip_utils.hpp"
#include "ipmodule_exceptions.hpp"
#include "utils.hpp"

using boost::cmatch;
using boost::regex;

using std::ifstream;
using std::pair;
using std::string;
using std::stringstream;
using std::vector;

namespace ip_module
{

struct ::addrinfo* get_addrinfo(const string& host, const string& port)
{
    struct ::addrinfo* res;
    struct ::addrinfo hints;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AI_FAMILY;
    hints.ai_socktype = SOCK_DGRAM;

    const int rtval = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if(rtval != 0)
        throw(name_resolve_exception("getaddrinfo error:" + string(gai_strerror(rtval)) + ", tried to resolve: host: "+host+", port:"+port));
    return res;
}

vector<pair<string, string> > addrinfo_to_string(const struct ::addrinfo* addr)
{
    vector<pair<string, string> > result;
    const struct addrinfo* ptr = addr;
    while(ptr != NULL)
    {
        pair<string, string> rtval = sockaddr_to_numeric_string(ptr->ai_addr, ptr->ai_addrlen);
        if(!rtval.first.empty() || !rtval.second.empty())
            result.push_back(rtval);
        ptr = ptr->ai_next;
    }
    return result;
}

pair<string, string> sockaddr_to_numeric_string(const struct sockaddr* addr, const socklen_t& addr_size)
{
    char host[128];
    char port[16];
    int rtval = getnameinfo(addr, addr_size, host, sizeof(host), port, sizeof(port), (NI_NUMERICHOST || NI_NUMERICSERV));
    if(rtval != 0)
        throw(name_resolve_exception("getnameinfo error:" + string(gai_strerror(rtval))));
    return pair<string, string>(host, port);
}

vector<pair<string, string> > numeric_resolve(const string& host, const string& port)
{
    struct addrinfo* addrinfo = get_addrinfo(host, port);
    vector<pair<std::string, std::string> > result = addrinfo_to_string(addrinfo);
    freeaddrinfo(addrinfo);
    return result;
}

string get_if_name(const string ip_addr)
{
    HC_LOG_TRACE("");

    if(ip_addr == "0.0.0.0" || ip_addr == "::")
        return "any";

    struct ifaddrs *ifap;
    getifaddrs(&ifap);

    struct ifaddrs *ptr = ifap;
    string result;

    //convert searched addr to binary form
    struct IN_ADDR bin_addr;
    if(inet_pton(AI_FAMILY, ip_addr.c_str(), &bin_addr) != 1)
    {
        throw (invalid_address_exception("inet_pton error: "+errno_to_string(errno)));
    }

    while(ptr != NULL)
    {
        if (ptr->ifa_addr->sa_family == AI_FAMILY){
            char addr[ADDRLEN];
#ifdef IPV6
            if(memcmp(bin_addr.s6_addr, (reinterpret_cast<struct sockaddr_in6*>(ptr->ifa_addr)->sin6_addr.s6_addr), 16) == 0)
#else
            if(bin_addr.s_addr == reinterpret_cast<struct sockaddr_in*>(ptr->ifa_addr)->sin_addr.s_addr)
#endif
            {
                result = ptr->ifa_name;
                break; // interface found, stop searching
            }
         }
         ptr = ptr->ifa_next;
    }
    freeifaddrs(ifap);
    return result;
    }
    
/*
string get_if_name(const string ip_addr)
{
    //using getifaddrs() would be better, but discovered the function after implemention of the ioctl variant :(
    if(ip_addr == "0.0.0.0" || ip_addr == "::")
        return "any";
    struct ifconf ifconf;
    const int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int ifreq_len = 10; //number of interfaces to read
    int last_ifreq_len = ifreq_len;
    scoped_ptr<struct ifreq> ifreqs;
    
    while(ifreq_len == last_ifreq_len) //check if we got ifreq_len interface informations, if yes increment ifreq space + refetch interface informations
    {
        last_ifreq_len = ifreq_len;

        //reallocate ifreq struct
        ifreq_len *= 2; //double ifreq size
        ifreqs.reset(reinterpret_cast<struct ifreq*>(realloc(ifreqs.get(), ifreq_len * sizeof(struct ifreq))));
        if(ifreqs.get() == NULL) //realloc failed
        {
            close(sockfd);
            throw realloc_exception(-1);
        }
        ifconf.ifc_buf = (char *) ifreqs.get();
        ifconf.ifc_len = ifreq_len * sizeof(struct ifreq);


        //fetch interface informations
        const int rtval = ioctl(sockfd, SIOCGIFCONF, &ifconf);
        if(rtval <0 || ifconf.ifc_len == 0)
        {
            close(sockfd);
            throw(no_interfaces_found_exception(errno));
        }
        ifreq_len = ifconf.ifc_len / sizeof(ifreq);
    }
    close(sockfd);

    //convert searched addr to binary form
    struct in_addr bin_addr;
    if(inet_pton(AF_INET, ip_addr.c_str(), &bin_addr) != 1){
        throw (invalid_address_exception(errno));
    }

    for(int i=0;i<ifreq_len;i++){
        struct ifreq rq = ifreqs.get()[i];
        char addr[INET_ADDRSTRLEN];//ipv6: INET6_ADDRSTRLEN
        struct sockaddr_in* sockaddr = (struct sockaddr_in*)&(rq.ifr_addr);
        if(inet_ntop(AF_INET, &(sockaddr->sin_addr.s_addr), addr, sizeof(addr)) == NULL){
            throw(invalid_address_exception(errno));
        }
                
        if(bin_addr.s_addr == sockaddr->sin_addr.s_addr){
            return string(rq.ifr_name);
        }
    }
    return string("");
}
*/


string ifindex_to_name(const unsigned int& ifindex) 
{
    HC_LOG_TRACE("");
    if(ifindex == 0)
        return "any";
    char ifname[IFNAMSIZ];
    if_indextoname(ifindex, ifname);
    if(ifname == NULL)
        throw(interface_not_exist_exception("interface doesn't exist"));
    return string(ifname);
}


int ifname_to_index(const string& ifname)
{
    HC_LOG_TRACE("");
    if(ifname == "any")
        return 0;
    int ifindex  = if_nametoindex(ifname.c_str());
    if(ifindex == 0)
        throw(interface_not_exist_exception(errno_to_string(errno)));
    return ifindex;
}
/*
void ifname_to_addr(const int ifindex, struct SOCKADDR_IN* sockaddr, int sockfd)
{
    struct ifreq ifr;
    int rtval;
    if((rtval = ioctl(sockfd, SIOCGIFADDR ,&ifr)) == -1)
        throw(ioctl_exception(rtval));

    *sockaddr = *(reinterpret_cast<struct SOCKADDR_IN*>(&ifr.ifr_addr));
}
*/


#ifdef IPV6
void host_to_sockaddr_storage(const string& host, const int& port, struct sockaddr_storage* sockaddr_)
{
    HC_LOG_TRACE("");
    struct sockaddr_in6* sockaddr = reinterpret_cast<struct sockaddr_in6*>(sockaddr_);
    sockaddr->sin6_family = AF_INET6;
    sockaddr->sin6_port = htons(port);
    sockaddr->sin6_flowinfo = 0;
    sockaddr->sin6_scope_id = 0;

    if(host == "0")
    {
        sockaddr->sin6_addr = in6addr_any;
    }
    else
    {
        if(inet_pton(AF_INET6, host.c_str(), &(sockaddr->sin6_addr)) != 1)
        {
            throw (invalid_address_exception("inet_pton error: "+errno_to_string(errno)));
        }
    }

}
#else
void host_to_sockaddr_storage(const string& host, const int& port, struct sockaddr_storage* sockaddr_)
{
    HC_LOG_TRACE("arguments: host: " + host+", port: " << port);
    struct sockaddr_in* sockaddr  = reinterpret_cast<struct sockaddr_in*>(sockaddr_);
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);

    if(host == "0")
    {
        sockaddr->sin_addr.s_addr = htons(INADDR_ANY);
    }
    else
    {
        if(inet_pton(AF_INET, host.c_str(), &(sockaddr->sin_addr)) != 1)
        {
            throw (invalid_address_exception("inet_pton error: "+errno_to_string(errno)));
        }
    }
}
#endif

void uri_to_sockaddr_storage(const hc_uri_t& uri, struct sockaddr_storage* group, struct sockaddr_storage* src)
{
    HC_LOG_TRACE("");
    if(group != NULL)
    {
        host_to_sockaddr_storage(uri.host().c_str(), uri.port_as_int(), group);
    }

    if((src != NULL) && (!uri.user_information().empty()))
    {
        host_to_sockaddr_storage(uri.user_information().c_str(), 0, src);
    }
}

string get_default_gw(const std::string& if_name)
{
    HC_LOG_TRACE("");
#ifdef IPV6
    return "";
#else
    ifstream fd("/proc/net/route", ifstream::in);
    if(!fd)
        throw io_exception("can't open /proc/net/route");

    bool found = false;
    string line;

    while(!found && getline(fd, line)){
        if(line.find(if_name+"\t00000000", 0) != string::npos){
            found = true;
        }
    }
    fd.close();

    if(!found)
        throw io_exception("no default gw found in /proc/net/route for interface: "+if_name);

    cmatch match;
    regex expr(if_name+"\t00000000\t([0-9A-F]{8}).+");
    regex_match(line.c_str(), match, expr);
    if(match.size() != 2)
        throw io_exception("/proc/net/route has unexpected format");

    vector<string> gw_hex;
    gw_hex.push_back(match[1].str().substr(0,2));
    gw_hex.push_back(match[1].str().substr(2,2));
    gw_hex.push_back(match[1].str().substr(4,2));
    gw_hex.push_back(match[1].str().substr(6,2));

    stringstream gw; 
    for(int i=3;i>=0;i--)
    {
        int x;
        std::istringstream iss(gw_hex[i]);
        iss >> std::hex >>  x;  
        gw << x;
        if(i > 0)
            gw << ".";
    }

    return gw.str();
#endif
}

} // namespace ip_module
