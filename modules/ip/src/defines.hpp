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

/**
  * @author Fabian Hollfer <hamcast (at) fholler.de>
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  *
  * @brief IPv4/IPv6 module defines
  */

/**
 *  @def IPMODULE_RBUFLEN Length of recv buffer
 *  @def IPMODULE_MAXREADS Number of successive reads on a recv socket
 *  @def IPV6 is set when the  Code is compiled as IPv6 module
 *  @def AI_FAMILIY, AF_INET6 when IPV6 is set, else AF_INET
 *  @def IN_ADDR, in6_addr when IPV6 is set, else in_addr
 *  @def SOCKADDR_IN, sockaddr_in6 when IPV6 is set, else sockaddr_in
 *  @def ADDRLEN, INET6_ADDRSTRLEN when IPV6 is set, else INET_ADDRSTRLEN
 */
#ifndef DEFINES_HPP
#define DEFINES_HPP

// General defines
#define IPMODULE_RBUFLEN    2048
#define IPMODULE_MAXREADS   10

#ifndef IPV6

#define AI_FAMILY AF_INET
#define IN_ADDR in_addr
#define SOCKADDR_IN sockaddr_in
#define ADDRLEN INET_ADDRSTRLEN

#else

#define AI_FAMILY AF_INET6
#define IN_ADDR in6_addr
#define SOCKADDR_IN sockaddr_in6
#define ADDRLEN INET6_ADDRSTRLEN

#endif
#endif
