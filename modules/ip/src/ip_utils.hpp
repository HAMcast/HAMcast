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

#ifndef IP_UTILS_HPP
#define IP_UTILS_HPP

#include <netdb.h>
#include <string>
#include <vector>

#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"


/**
  * @author Fabian Hollfer <hamcast (at) fholler.de>
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  *
  * @brief IPv4/IPv6 module utility and helper functions
  */
namespace ip_module
{

/**
 * @brief resolves given host and port to their numeric addresses and numerical port number
 * @returns vector with pair<string host, string port> sets, one pair for one getaddr(..) result
 * @throws name_resolve_exception
 */
std::vector<std::pair<std::string, std::string> > numeric_resolve(const std::string& host, const std::string& port);

/**
 * @brief converts the addrinfo parameter to an vector of pair<string host, string port>
 * @returns vector with pair<string host, string port> sets, one pair for one getaddr(..) result
 */
std::vector<std::pair<std::string, std::string> > addrinfo_to_string(const struct ::addrinfo* addr);

/**
 * @brief returns an pair with the numeric host and port for the given sockaddr struct
 * @param addr_size size of addr
 * @return pair<string hostname, string portnr>
 * @throws name_resolve_exception on getnameinfo(..) error
 */
std::pair<std::string, std::string> sockaddr_to_numeric_string(const struct sockaddr* addr, const socklen_t& addr_size);

/**
 * @brief resolves given host and portnames
 * @returns result from the getaddrinfo call, after using the return value must be freed with freeaddrinfo(res)
 * @throws name_resolve_exception on getaddrinfo(..) error
 */
struct ::addrinfo* get_addrinfo(const std::string& host, const std::string& port);

/**
 * @Brief fills the supplied sockaddr_storage struct with the  numerical host and port
 * @throws invalid_address_exception if host can't be converted with inet_pton to an binary address
 */
void host_to_sockaddr_storage(const std::string& host, const int& port, struct sockaddr_storage* sockaddr_);

/**
 * @Brief fills the supplied sockaddr_storage structs with the given URI
 * @param src Source part of an SSM URI, willbe set to NULL if URI don't contains an user_information part, port will always be set to 0
 * @param group group address of an multicast URI
 * @param uri address parts must be numerical, no host-/portnames are beeing resolved
 * @throws invalid_address_exception if host part of the URI can't be converted with inet_pton to an binary address
 */
void uri_to_sockaddr_storage(const hc_uri_t& uri, struct sockaddr_storage* group, struct sockaddr_storage* src);

/**
 * @Brief converts interface name ifname to their interface number
 * @param ifname if ifname is "any" 0 will be returned
 * @throws interface_not_exist_exception if no interface index for the ifname argument can be resolved
 */
int ifname_to_index(const std::string& ifname);

/**
 * @brief returns the interface name that has ip_addr assigned
 * @returns any if ip_addr is "0.0.0.0" or "::"
 * @throws no_interfaces_found_exception will be thrown if ioctl return 0 interfaces
 * @throws realloc_exception will be thrown if realloc fails
 * @throws invalid_address_exception if ip_addr can't be converted with inet_pton to an binary address
 */
std::string get_if_name(const std::string ip_addr);

/**
 * @brief returns the interface index for an given ip_address, wrapper for ifname_to_index(get_if_name(..))
 * @throws no_interfaces_found_exception will be thrown if ioctl return 0 interfaces
 * @throws realloc_exception will be thrown if realloc fails
 * @throws invalid_address_exception if ip_addr can't be converted with inet_pton to an binary address
 * @throws interface_not_exist_exception if no interface with the given index could be found
 */
inline int ip_to_if_index(const std::string ip_addr)
{
    HC_LOG_TRACE("");
    return ifname_to_index((get_if_name(ip_addr)));
}

/**
 * @brief returns the interface name for the given interface index
 * @returns any if 0 was given as ifindex
 * @throws interface_not_exist_exception if no interface with the given index could be found
 */
std::string ifindex_to_name(const unsigned int& ifindex);

/**
 * @brief returns the default gw for the given interface
 * @returns "" if no default gw is set, else it return the IP-address of the gateway
 * @throws io_exception if /proc/net/route isn't readable
 * @Todo implementation for IPV6 is missing
 */
std::string get_default_gw(const std::string& ip_addr);

} // namespace
#endif
