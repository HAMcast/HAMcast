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
#include <boost/lexical_cast.hpp>

#include "hamcast/hamcast_logging.h"

#include "ip_utils.hpp"
#include "native_socket.hpp"
#include "utils.hpp"

#ifdef __APPLE__
#include <arpa/inet.h>
#endif

using boost::lexical_cast;
using std::string;
using namespace ip_module;

namespace ip_module
{

#ifdef IPV6
    native_socket::native_socket() : m_bind_ip("::"), m_ifindex(0)
#else
    native_socket::native_socket() : m_bind_ip("0.0.0.0"), m_ifindex(0)
#endif
    {
        HC_LOG_TRACE("");
        init();
    }

    //bind_ip: interface used for receiving packets
    //m_ifindex: will be used to choose interface to send packets
    native_socket::native_socket(const string& bind_ip) : m_bind_ip("0.0.0.0"), m_ifindex(0)
    {
        HC_LOG_TRACE("");
        HC_LOG_DEBUG("native socket custom contr: "+bind_ip);
        init();
    }

    void native_socket::init(){
        HC_LOG_TRACE("");
        HC_LOG_DEBUG("m_bind_ip: " + m_bind_ip + ", m_infindex: " + lexical_cast<string>(m_ifindex));

#ifdef IPV6
        m_socketfd = ::socket(AF_INET6, SOCK_DGRAM, 0);
#else
        m_socketfd = ::socket(AF_INET, SOCK_DGRAM, 0);
#endif
        if(m_socketfd == -1)
            throw socket_create_exception("opening socket failed: "+errno_to_string(errno));

        int one = 1;
        if (setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
        {
            HC_LOG_ERROR("setting SO_REUSEADDR on socket failed");
        }
        multicast_ttl = getsockopt_multicast_ttl();

        //set_outgoing_mcast_if(); can't be set atm with the hamcast_module api

    }

    native_socket::~native_socket()
    {
        HC_LOG_TRACE("");
        close(m_socketfd);
    }

    void native_socket::mcast_group_operation(const struct sockaddr_storage* addr, const int& optname)
    {
        HC_LOG_TRACE("");
#ifdef __linux
        struct group_req mgroup;

        mgroup.gr_interface = m_ifindex; //kernel choose interface
        mgroup.gr_group = *addr; 
#elif __APPLE__
        struct ip_mreq mgroup;
        mgroup.imr_interface.s_addr = htonl(INADDR_ANY); //IP address of local interface
#ifdef IPV6
        if(inet_pton(AF_INET6, m_bind_ip.c_str(), &(mgroup.imr_interface.s_addr)) != 1)
#else
        if(inet_pton(AF_INET, m_bind_ip.c_str(), &(mgroup.imr_interface.s_addr)) != 1)
#endif
            std::logic_error("inet_pton failed");  //throw(inet_pton_exception(errno));
#endif

#ifdef IPV6
        const int rtval = setsockopt(m_socketfd, IPPROTO_IPV6, optname, &mgroup, sizeof(mgroup));
#else
        const int rtval = setsockopt(m_socketfd, IPPROTO_IP, optname, &mgroup, sizeof(mgroup));
#endif
        if(rtval == -1)
        {
            throw setsockopt_exception("multicast join exception: " + errno_to_string(errno));
        }
    }

#ifdef __APPLE__
    //OS X doesnt implement IP_ADD_SOURCE_MEMBERSHIP
    void native_socket::ssm_group_operation(const struct sockaddr_storage* groupaddr, const sockaddr_storage*, const int& optname)
    {
        throw not_implemented_exception("ssm_group_operation");
    }
    void native_socket::join(const struct sockaddr_storage* group, const struct sockaddr_storage* ssm_src)
    {
        throw not_implemented_exception("join");
    }
    void native_socket::leave(const struct sockaddr_storage* group, const struct sockaddr_storage* ssm_src)
    {
        throw not_implemented_exception("leave");
    }
#else

    void native_socket::ssm_group_operation(const struct sockaddr_storage* groupaddr, const struct sockaddr_storage* ssm_src, const int& optname)
    {
        HC_LOG_TRACE("");
        struct group_source_req src;
        src.gsr_group = *groupaddr;
        src.gsr_interface = m_ifindex;
        src.gsr_source = *ssm_src;

#ifdef IPV6
        const int rtval = setsockopt(m_socketfd, IPPROTO_IPV6, optname, &src, sizeof(src));
#else
        const int rtval = setsockopt(m_socketfd, IPPROTO_IP, optname, &src, sizeof(src));
#endif
        if(rtval == -1)
        {
            throw setsockopt_exception("SSM join exception: "+errno_to_string(errno));
        }
    }

    void native_socket::join(const struct sockaddr_storage* group, const struct sockaddr_storage* ssm_src) 
    {
        HC_LOG_TRACE("");
        ssm_group_operation(group, ssm_src, MCAST_JOIN_SOURCE_GROUP);
    }

    void native_socket::leave(const struct sockaddr_storage* group, const struct sockaddr_storage* ssm_src) 
    {
        HC_LOG_TRACE("");
        ssm_group_operation(group, ssm_src, MCAST_LEAVE_SOURCE_GROUP);
    }
#endif

    void native_socket::join(const struct sockaddr_storage* group) 
    {
        HC_LOG_TRACE("");
#if defined(MCAST_JOIN_GROUP) && !defined(__APPLE__)
        mcast_group_operation(group, MCAST_JOIN_GROUP);
#else
        switch (group->ss_family) {
        case AF_INET:{
                struct ip_mreq mreq;
    #ifdef __APPLE__
                if(inet_pton(AF_INET, m_bind_ip.c_str(), &(mreq.imr_interface.s_addr)) != 1) {
                    mreq.imr_interface.s_addr = htonl(INADDR_ANY); //IP address of local interface
                    std::logic_error("inet_pton failed");  
                    //throw inet_pton_exception(errno);
                }
    #else
                struct ifreq ifreq;
                if (m_ifindex > 0) {
                    if (if_indextoname (m_ifindex, ifreq.ifr_name)==NULL)
                        throw interface_not_exist_exception ("if_indextoname error: " + errno_to_string(errno));

                    if (ioctl (m_socketfd, SIOCGIFADDR, &ifreq) < 0) {
                        throw ioctl_exception ();
                    }
                    memcpy (&mreq.imr_interface, &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));
                }
                else {
                    mreq.imr_interface.s_addr = htonl (INADDR_ANY);
                }
    #endif
                memcpy (&mreq.imr_multiaddr, &((const struct sockaddr_in *) group)->sin_addr, sizeof(struct in_addr));
                if (setsockopt (m_socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0)
                    throw setsockopt_exception("IPv4 Multicast join exception: " + errno_to_string(errno));
            }
            break;
    #ifdef IPV6
        case AF_INET6:{
                struct ipv6_mreq mreq6;
        #ifdef __APPLE__
                if(inet_pton(AF_INET6, m_bind_ip.c_str(), &(mreq6.ipv6mr_multiaddr)) != 1) {
                    mreq6.ipv6mr_multiaddr = in6addr_any; //IP address of local interface
                    std::logic_error("inet_pton failed");
                    //throw inet_pton_exception(errno);
                }
        #else
                if (m_ifindex > 0) {
                    mreq6.ipv6mr_interface = m_ifindex;
                }
                else 
                    mreq6.ipv6mr_interface = 0;
        #endif
                memcpy (&mreq6.ipv6mr_multiaddr, &((const struct sockaddr_in6 *) group)->sin6_addr, sizeof(struct in6_addr));
                if (setsockopt (m_socketfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) != 0)
                    throw setsockopt_exception("IPv6 Multicast join exception: " + errno_to_string(errno));
            }
            break;
    #endif
        default:{
                errno = EAFNOSUPPORT;
                throw wrong_address_family_exception ("Not supported: " + errno_to_string(errno));
            }
        }
#endif
    }

    void native_socket::leave(const struct sockaddr_storage* group) 
    {
        HC_LOG_TRACE("");
#ifdef MCAST_JOIN_GROUP
        mcast_group_operation(group, MCAST_LEAVE_GROUP);
#else
        switch (group->ss_family) {
        case AF_INET:{
                struct ip_mreq mreq;
    #ifdef __APPLE__
                if(inet_pton(AF_INET, m_bind_ip.c_str(), &(mreq.imr_interface.s_addr)) != 1) {
                    mreq.imr_interface.s_addr = htonl(INADDR_ANY); //IP address of local interface
                    std::logic_error("inet_pton failed");
                    //throw inet_pton_exception(errno);
                }
    #else
                struct ifreq ifreq;
                if (m_ifindex > 0) {
                    if (if_indextoname (m_ifindex, ifreq.ifr_name)==NULL)
                        throw interface_not_exist_exception ("if_indextoname error: " + errno_to_string(errno));

                    if (ioctl (m_socketfd, SIOCGIFADDR, &ifreq) < 0) {
                        throw ioctl_exception ();
                    }
                    // FIXME: first parameter should be mreq.imr_interface.s_addr ? Check for the others too
                    memcpy (&mreq.imr_interface, &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));
                }
                else {
                    mreq.imr_interface.s_addr = htonl (INADDR_ANY);
                }
    #endif
                memcpy (&mreq.imr_multiaddr, &((const struct sockaddr_in *) group)->sin_addr, sizeof(struct in_addr));
                if (setsockopt (m_socketfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) != 0)
                    throw setsockopt_exception("IPv4 Multicast leave exception: " + errno_to_string(errno));
            }
            break;
#ifdef IPV6
        case AF_INET6:{
                struct ipv6_mreq mreq6;
    #ifdef __APPLE__
                if(inet_pton(AF_INET6, m_bind_ip.c_str(), &(mreq6.ipv6mr_multiaddr)) != 1) {
                    mreq6.ipv6mr_multiaddr = in6addr_any; //IP address of local interface
                    std::logic_error("inet_pton failed");
                    //throw inet_pton_exception(errno);
                }
    #else
                if (m_ifindex > 0) {
                    mreq6.ipv6mr_interface = m_ifindex;
                }
                else 
                    mreq6.ipv6mr_interface = 0;
    #endif
                memcpy (&mreq6.ipv6mr_multiaddr, &((const struct sockaddr_in6 *) group)->sin6_addr, sizeof(struct in6_addr));
                if (setsockopt (m_socketfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6)) != 0)
                    throw setsockopt_exception("IPv6 Multicast leave exception: " + errno_to_string(errno));
            }
            break;
#endif
        default:{
                errno = EAFNOSUPPORT;
                throw wrong_address_family_exception ("Not supported: " + errno_to_string(errno));
            }
        }
#endif
    }


    int native_socket::recv_async(void* buf, const int &len) 
    {
        HC_LOG_TRACE("");
        int rtval = recv(m_socketfd, buf, len, MSG_DONTWAIT);
        if((rtval == -1) && errno == EAGAIN)
        {
            HC_LOG_INFO("EAGAIN");
            return -1;
        }
        if(((rtval == -1) || (rtval > len)))
        {
            throw recv_exception("recv returned: "+errno_to_string(errno));
        }
        return rtval;
    }

    int native_socket::sendto(const struct sockaddr_storage* addr, const void *buf, const int& buf_len)
    {
        HC_LOG_TRACE("");
        const int rtval = ::sendto(m_socketfd, buf, buf_len, 0, reinterpret_cast<const struct sockaddr*>(addr), sizeof(struct sockaddr));
        if(rtval == -1)
        {
            throw sendto_exception("sendto returned: "+errno_to_string(errno));
        }
        return rtval;
    }

    int native_socket::getsockopt_multicast_ttl()
    {
        HC_LOG_TRACE("");
        int ttl;
        socklen_t ttl_size = sizeof(ttl);

#ifdef IPV6
        if(getsockopt(m_socketfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, &ttl_size) == -1)
        {
            throw setsockopt_exception("getting TTL failed: "+errno_to_string(errno));
        }

#else
        if (getsockopt(m_socketfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, &ttl_size) == -1)
        {
            throw setsockopt_exception("setting TTL failed: "+errno_to_string(errno));
        }
#endif
        HC_LOG_DEBUG("TTL: " << ttl);
        return ttl;

    }

    void native_socket::set_multicast_ttl(const int ttl)
    {
        HC_LOG_TRACE("");
#ifdef IPV6
        if(setsockopt(m_socketfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) == -1)
#else
        if (setsockopt(m_socketfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1)
#endif
        {
            throw setsockopt_exception("setting TTL failed: "+errno_to_string(errno));
        }

        multicast_ttl = ttl;
    }


    void native_socket::set_outgoing_mcast_if()
    {
        HC_LOG_TRACE("");
        HC_LOG_DEBUG("setting interface for outgoing multicast packet to interface: " + ifindex_to_name(m_ifindex));
#ifdef IPV6
        const int rtval = setsockopt(m_socketfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &m_ifindex, sizeof(m_ifindex));
#else
        const int rtval = setsockopt(m_socketfd, IPPROTO_IP, IP_MULTICAST_IF, &m_ifindex, sizeof(m_ifindex));
#endif
        if(rtval == -1)
        {
            HC_LOG_ERROR("setting outgoing multicast interface failed");
        }
    }

    void native_socket::bind(const int& port)
    {
        HC_LOG_TRACE("");
        struct sockaddr_storage sockaddr;
        host_to_sockaddr_storage(m_bind_ip, port, &sockaddr);

        HC_LOG_DEBUG("bind to: " + m_bind_ip);
        if(::bind(m_socketfd, reinterpret_cast<const struct sockaddr*>(&sockaddr), sizeof(struct SOCKADDR_IN)) == -1)
        {
            throw bind_exception("bind returned: "+errno_to_string(errno));
        }
    }

} //namespace
