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

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "ip_socket.hpp"
#include "ip_exceptions.hpp"
#include "ip_module.hpp"

#include "hamcast/hamcast_logging.h"
#include "hamcast/hamcast.hpp"


using namespace ipm;

/*** PRIVATE STUFF ***/

int ip_socket::init()
{
    HC_LOG_TRACE("");

    int af = m_if_addr.ss_family;

    if ((m_sockfd = ::socket(af, SOCK_DGRAM, 0)) == -1) {
        HC_LOG_ERROR (" init create socket, errno "<< errno);
        return -1;
    }

    // enable address reuse
    int one = 1;
    if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
    {
        HC_LOG_ERROR ("setting SO_REUSEADDR on socket failed");
    }

    // set outgoing interface, if specified
    if (m_if_index > 0) {
        if (af == AF_INET) {
            struct sockaddr_in* sin4 = reinterpret_cast<sockaddr_in*>(&m_if_addr);
            HC_LOG_DEBUG ("Set outgoing IPv4 multicast interface to: " << m_if_name << ".");
            if (setsockopt( m_sockfd, IPPROTO_IP, IP_MULTICAST_IF, &(sin4->sin_addr), sizeof( struct in_addr ) ))
                HC_LOG_ERROR ("Failed to set outgoing IPv4 multicast interface: " << strerror(errno));
        }
        else if (af == AF_INET6) {
            HC_LOG_DEBUG ("Set outgoing IPv6 multicast interface to: " << m_if_name << ".");
            if (setsockopt (m_sockfd, IPPROTO_IP, IPV6_MULTICAST_IF, &m_if_index, sizeof(m_if_index)))
                HC_LOG_ERROR ("Failed to set outgoing IPv6 multicast interface: " << strerror(errno));
        }
        else {
            HC_LOG_DEBUG ("Using default interface for multicast.");
        }

    }
    else {
        HC_LOG_DEBUG ("Using default interface for multicast.");
    }
    m_ttl = __get_multicast_ttl();
    return 0;
}

int ip_socket::init(const int af)
{
    HC_LOG_TRACE("");
    HC_REQUIRE (af == AF_INET || af == AF_INET6);

    if ((m_sockfd = ::socket(af, SOCK_DGRAM, 0)) == -1) {
        HC_LOG_ERROR ("Create Socket failed!");
        return -1;
    }

    // set outgoing interface for multicast traffic
    if ((af == AF_INET) && (m_if_index > 0)) {
        struct sockaddr_in* sin4 = NULL;
        struct ifreq IfReq;

        memset( &IfReq, 0, sizeof( IfReq ) );
        strncpy( IfReq.ifr_name, m_if_name.c_str(), sizeof( IfReq.ifr_name ) );

        if ( ioctl ( m_sockfd, SIOCGIFADDR, &IfReq ) < 0 )
            HC_LOG_ERROR ("IOCTL failure, ifname: " << m_if_name << ", error: " << strerror(errno));

        switch ( IfReq.ifr_addr.sa_family ) {
            case AF_INET:
                sin4 = (struct sockaddr_in *) &IfReq.ifr_addr;
            break;

            default:
                HC_LOG_DEBUG ("Invalid address family " << IfReq.ifr_addr.sa_family);
        }
        HC_LOG_DEBUG ("Set outgoing IPv4 multicast if ...");
        if (setsockopt( m_sockfd, IPPROTO_IP, IP_MULTICAST_IF, &sin4->sin_addr, sizeof( struct in_addr ) ))
            HC_LOG_ERROR ("Failed to set outgoing IPv4 multicast interface: " << strerror(errno));
    }
    else if ( (af == AF_INET6) && (m_if_index > 0)) {
        HC_LOG_DEBUG ("Set outgoing IPv6 multicast if ...");
        if (setsockopt (m_sockfd, IPPROTO_IP, IPV6_MULTICAST_IF, &m_if_index, sizeof(m_if_index)))
            HC_LOG_ERROR ("Failed to set outgoing IPv6 multicast interface: " << strerror(errno));
    }
    else {
        HC_LOG_DEBUG ("Unable to set outgoing multicast interface, no IP version information.");
    }
    m_ttl = __get_multicast_ttl();
    return 0;
}

unsigned int ip_socket::__get_multicast_ttl()
{
    HC_LOG_TRACE("");
    int ttl = 1;
    int ret = -1;
    socklen_t ttl_size = sizeof(ttl);

    if (m_if_addr.ss_family == AF_INET) {
        HC_LOG_DEBUG ("Get IPv4 TTL");
        ret = getsockopt(m_sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, &ttl_size);
    }
    else if (m_if_addr.ss_family == AF_INET6) {
        HC_LOG_DEBUG("Get IPv6 TTL");
        ret = getsockopt(m_sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, &ttl_size);
    }
    else {
        HC_LOG_DEBUG ("No IP address family/version information!");
    }
    if (ret < 0) {
        HC_LOG_ERROR (" Get TTL error: " << gai_strerror (ret));
    }
    HC_LOG_DEBUG(" TTL: " << ttl);
    return ttl;
}

int ip_socket::mcast_join (const struct sockaddr *grp, socklen_t grplen)
{
    HC_LOG_TRACE("");
#ifdef MCAST_JOIN_GROUP
    struct group_req req;
    if (m_if_index > 0) {
        req.gr_interface = m_if_index;
    } else if (!m_if_name.empty()) {
        if ( (req.gr_interface = if_nametoindex (m_if_name.c_str())) == 0 ) {
            errno = ENXIO;
            return (-1);
        }
    } else {
        req.gr_interface = 0;
    }
    if (grplen > sizeof (req.gr_group)) {
        errno = EINVAL;
        return (-1);
    }
    memcpy (&req.gr_group, grp, grplen);

    int family = -1;
    switch (grp->sa_family) {
        case AF_INET:
            family = IPPROTO_IP;
            break;
        case AF_INET6:
            family = IPPROTO_IPV6;
            break;
        default:
            family = -1;
        return (-1);
    }

    return (setsockopt (m_sockfd, family, MCAST_JOIN_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
        case AF_INET: {
            struct ip_mreq mreq;
            struct ifreq ifreq;

            memcpy (&mreq.imr_multiaddr,
                   &((const sockaddr_in *)grp)->sin_addr,
                   sizeof (struct in_addr));
            if (ifindex > 0) {
                if (if_indextoname (m_if_index, ireq.ifr_name) == NULL) {
                    errno = ENXIO;
                    return (-1);
                }
                goto doioctl;
            }
            else if (!m_if_name.empty()) {
                strncpy (ifreq.ifr_name, m_if_name.c_str(), IFNAMSIZ);
doioctl:
                if (ioctl (sockfd, SIOCGIFADDR, &ifreq) < 0)
                    return (-1);
                memcpy (&mreq.imr_interface,
                        &((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr,
                        sizeof(struct in_addr));
            }
            else {
                mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            }
            return (setsockopt (m_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                &mreq, sizeof (mreq)));
        }
        case AF_INET6: {
            struct ipv6_mreq mreq6;
            memcpy (&mreq6.ipv6mr_multiaddr,
                    &((const struct sockaddr_in6 *)grp)->sin6_addr,
                    sizeof (struct in6_addr));
            if (ifindex > 0) {
                mreq6.ipv6mr_interface = m_if_index;
            }
            else if (ifname != NULL) {
                if ((mreq6.ipv6mr_interface = if_nametoindex(m_if_name.c_str())) == 0) {
                    errno = ENXIO;
                    return (-1);
                }
            }
            else {
                mreq6.ipv6mr_interface = 0;
            }
            return (setsockopt (m_sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                                &mreq6, sizeof (mreq6)));
        }
        default: {
            errno = EAFNOSUPPORT;
            return (-1);
        }
    }
#endif
}

int ip_socket::mcast_join_source_group (const struct sockaddr *src, socklen_t srclen,
                                        const struct sockaddr *grp, socklen_t grplen)
{
    HC_LOG_TRACE("");
#ifdef MCAST_JOIN_SOURCE_GROUP
    struct group_source_req req;
    if (m_if_index > 0) {
        req.gsr_interface = m_if_index;
    } else if (!m_if_name.empty()) {
        if ( (req.gsr_interface = if_nametoindex(m_if_name.c_str())) == 0) {
            errno = ENXIO;  /* if name not found */
            return (-1);
        }
    } else
        req.gsr_interface = 0;
    if (grplen > sizeof(req.gsr_group) ||
            srclen > sizeof(req.gsr_source)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);

    int family = -1;
    switch (grp->sa_family) {
        case AF_INET:
            family = IPPROTO_IP;
            break;
        case AF_INET6:
            family = IPPROTO_IPV6;
            break;
        default:
            family = -1;
            return (-1);
    }

    return (setsockopt(m_sockfd, family, MCAST_JOIN_SOURCE_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
#ifdef IP_ADD_SOURCE_MEMBERSHIP
    case AF_INET: {
        struct ip_mreq_source   mreq;
        struct ifreq            ifreq;

        memcpy(&mreq.imr_multiaddr,
               &((struct sockaddr_in *) grp)->sin_addr,
               sizeof(struct in_addr));
        memcpy(&mreq.imr_sourceaddr,
               &((struct sockaddr_in *) src)->sin_addr,
               sizeof(struct in_addr));

        if (m_if_index > 0) {
            if (if_indextoname(m_if_index, ifreq.ifr_name) == NULL) {
                errno = ENXIO;  /* i/f index not found */
                return(-1);
            }
            goto doioctl;
        } else if (!m_if_name.empty()) {
            strncpy(ifreq.ifr_name, m_if_name.c_str(), IFNAMSIZ);
doioctl:
            if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
                return(-1);
            memcpy(&mreq.imr_interface,
                   &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr,
                   sizeof(struct in_addr));
        } else
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return(setsockopt(m_sockfd, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
                          &mreq, sizeof(mreq)));
    }
#endif
    case AF_INET6: /* IPv6 source-specific API is
                      MCAST_JOIN_SOURCE_GROUP */
    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

int ip_socket::mcast_leave (const struct sockaddr *grp, socklen_t grplen)
{
    HC_LOG_TRACE("");
#ifdef MCAST_LEAVE_GROUP
    struct group_req req;
    if (m_if_index > 0) {
        req.gr_interface = m_if_index;
    } else if (!m_if_name.empty()) {
        if ( (req.gr_interface = if_nametoindex (m_if_name.c_str())) == 0 ) {
            errno = ENXIO;
            return (-1);
        }
    } else {
        req.gr_interface = 0;
    }
    if (grplen > sizeof (req.gr_group)) {
        errno = EINVAL;
        return (-1);
    }
    memcpy (&req.gr_group, grp, grplen);

    int family = -1;
    switch (grp->sa_family) {
        case AF_INET:
            family = IPPROTO_IP;
            break;
        case AF_INET6:
            family = IPPROTO_IPV6;
            break;
        default:
            family = -1;
        return (-1);
    }

    return (setsockopt(m_sockfd, family, MCAST_LEAVE_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
    case AF_INET: {
        struct ip_mreq      mreq;

        memcpy(&mreq.imr_multiaddr,
               &((const struct sockaddr_in *) grp)->sin_addr,
               sizeof(struct in_addr));
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        return (setsockopt (m_sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                            &mreq, sizeof(mreq)));
    }

#ifndef IPV6_LEAVE_GROUP    /* APIv0 compatibility */
#define IPV6_LEAVE_GROUP    IPV6_DROP_MEMBERSHIP
#endif
    case AF_INET6: {
        struct ipv6_mreq    mreq6;

        memcpy(&mreq6.ipv6mr_multiaddr,
               &((const struct sockaddr_in6 *) grp)->sin6_addr,
               sizeof(struct in6_addr));
        mreq6.ipv6mr_interface = 0;
        return (setsockopt (m_sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
                            &mreq6, sizeof(mreq6)));
    }

    default:
        errno = EAFNOSUPPORT;
        return (-1);
    }
#endif
}

int ip_socket::mcast_leave_source_group (const struct sockaddr *src, socklen_t srclen,
                                         const struct sockaddr *grp, socklen_t grplen)
{
    HC_LOG_TRACE("");
#ifdef MCAST_LEAVE_SOURCE_GROUP
    struct group_source_req req;
    req.gsr_interface = 0;
    if (grplen > sizeof(req.gsr_group) ||
            srclen > sizeof(req.gsr_source)) {
        errno = EINVAL;
        return (-1);
    }
    memcpy(&req.gsr_group, grp, grplen);
    memcpy(&req.gsr_source, src, srclen);

    int family = -1;
    switch (grp->sa_family) {
        case AF_INET:
            family = IPPROTO_IP;
            break;
        case AF_INET6:
            family = IPPROTO_IPV6;
            break;
        default:
            family = -1;
            return (-1);
    }

    return (setsockopt(m_sockfd, family, MCAST_LEAVE_SOURCE_GROUP, &req, sizeof(req)));
#else
    switch (grp->sa_family) {
#ifdef IP_DROP_SOURCE_MEMBERSHIP
    case AF_INET: {
        struct ip_mreq_source   mreq;

        memcpy(&mreq.imr_multiaddr,
               &((struct sockaddr_in *) grp)->sin_addr,
               sizeof(struct in_addr));
        memcpy(&mreq.imr_sourceaddr,
               &((struct sockaddr_in *) src)->sin_addr,
               sizeof(struct in_addr));
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);

        return(setsockopt(m_sockfd, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP,
                          &mreq, sizeof(mreq)));
    }
#endif
    case AF_INET6: /* IPv6 source-specific API is
                      MCAST_LEAVE_SOURCE_GROUP */
    default:
        errno = EAFNOSUPPORT;
        return(-1);
    }
#endif
}

/*** PUBLIC STUFF ***/

ip_socket::ip_socket()
{
    HC_LOG_TRACE ("");
    int ret;
    //set defaults for IPv6
    m_if_index = 0;
    m_if_name = "unknown";
    m_if_addr.ss_family = AF_INET6;
    //m_if_addr.ss_len = sizeof(struct sockaddr_in6); // MacOS (UNIX, BSD?) only
    struct sockaddr_in6 *sa6 = reinterpret_cast<sockaddr_in6*>(&m_if_addr);
    sa6->sin6_addr = in6addr_any; // use my IPv6 address
    if ((ret = init ()) == -1) {
        HC_LOG_WARN ("Unable to create IPv6 send socket, try IPv4 now!");
        // set defaults for IPv4
        m_if_addr.ss_family = AF_INET;
        struct sockaddr_in *sa = reinterpret_cast<sockaddr_in*>(&m_if_addr);
        sa->sin_addr.s_addr = INADDR_ANY;  // use my IPv4 address
        if ((ret = init ()) == -1) {
            HC_LOG_ERROR("Unable to create Ipv4 send socket!");
            throw ipm_socket_create_exception("Create socket failure.");
        }
    }
}

ip_socket::ip_socket(struct sockaddr_storage& ifaddr, const int& ifindex, const std::string& ifname)
{
    HC_LOG_TRACE ("");
    m_if_addr = ifaddr;
    m_if_index = ifindex;
    m_if_name = ifname;
    // init socket
    init();
}


ip_socket::ip_socket(const hamcast::uri &grp_uri, const int& ifindex, const std::string& ifname)
{
    HC_LOG_TRACE ("");
    m_if_index = ifindex;
    m_if_name = ifname;
    struct addrinfo hints, *grp;
    int ret;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    // resolve uri to address struct
    if ((ret = getaddrinfo (grp_uri.host().c_str(), grp_uri.port().c_str(), &hints, &grp)) != 0) {
        HC_LOG_WARN ("getaddrinfo: " << gai_strerror (ret));
        throw ipm_address_exception ("getaddrinfo");
    }

    memcpy (&m_if_addr, grp->ai_addr, sizeof (m_if_addr));
    freeaddrinfo (grp);
    // init socket
    init();
}

void ip_socket::join (const hamcast::uri &grp_uri)
{
    HC_LOG_TRACE("");
    struct addrinfo hints, *grp, *src;
    int ret;
    // resolve source to addresss struct
    if (!grp_uri.user_information().empty()) {
        memset (&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((ret = getaddrinfo (grp_uri.user_information().c_str(), "0", &hints, &src)) != 0) {
            HC_LOG_WARN ("getaddrinfo: " << gai_strerror (ret));
            src = NULL;
        }
    }
    else {
        src = NULL;
    }

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    // resolve group to address struct
    if ((ret = getaddrinfo (grp_uri.host().c_str(), grp_uri.port().c_str(), &hints, &grp)) != 0) {
        HC_LOG_WARN ("getaddrinfo: " << gai_strerror (ret));
        return;
    }
    // join SSM or ASM group respectively
    if (src != NULL) {// SSM
        if((ret = mcast_join_source_group (src->ai_addr, src->ai_addrlen, grp->ai_addr, grp->ai_addrlen)) == -1) {
            HC_LOG_ERROR (" ssm join, return: " << ret << " errno: " << errno);
            return;
        }
    }
    else {// ASM
        if ((ret = mcast_join (grp->ai_addr, grp->ai_addrlen)) == -1) {
            HC_LOG_ERROR (" asm join, return: " << ret << " errno: " << errno);
            return;
        }
    }
    // free memory
    if (src)
        freeaddrinfo (src);
    freeaddrinfo (grp);
}

void ip_socket::leave (const hamcast::uri &grp_uri)
{
    HC_LOG_TRACE("");
    struct addrinfo hints, *grp, *src;
    int ret;
    // resolve source to address struct
    if (!grp_uri.user_information().empty()) {
        memset (&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((ret = getaddrinfo (grp_uri.user_information().c_str(), "0", &hints, &src)) != 0) {
            HC_LOG_WARN (" getaddrinfo: " << gai_strerror (ret));
            src = NULL;
        }
    }
    else {
        src = NULL;
    }

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    // resolve group to address struct
    if ((ret = getaddrinfo (grp_uri.host().c_str(), grp_uri.port().c_str(), &hints, &grp)) != 0) {
        HC_LOG_WARN (" getaddrinfo: " << gai_strerror (ret));
        return;
    }

    // leave SSM or ASM group respectively
    if (src != NULL) {// SSM
        if ((ret = mcast_leave_source_group (src->ai_addr, src->ai_addrlen, grp->ai_addr, grp->ai_addrlen)) == -1) {
            HC_LOG_ERROR (" ssm leave, return: " << ret << " errno: " << errno);
            return;
        }
    }
    else {// ASM
        if ((ret = mcast_leave (grp->ai_addr, grp->ai_addrlen)) == -1) {
            HC_LOG_ERROR (" asm leave, return: " << ret << " errno: " << errno);
            return;
        }
    }
    // free memory
    if (src)
        freeaddrinfo(src);
    freeaddrinfo(grp);
}

int ip_socket::recv_async(void* buf, const int &len) 
{
    HC_LOG_TRACE("");
    int rtval = recv(m_sockfd, buf, len, MSG_DONTWAIT);
    if(rtval < 0) {
        switch (errno) {
        case EAGAIN:
            HC_LOG_INFO(" EAGAIN, socket blocking or timeout");
            break;
        case EBADF:
            HC_LOG_ERROR(" EBADF bad file descriptor");
            break;
        case ECONNRESET:
            HC_LOG_ERROR(" ECONNRESET connection closed");
        case EFAULT:
            HC_LOG_ERROR(" EFAULT invalid receive buffer");
            break;
        case EINTR:
            HC_LOG_ERROR(" EINTR signal interrupt");
            break;
        case ENOBUFS:
            HC_LOG_ERROR(" ENOBUFS cannot alloc buffers");
            break;
        case ENOTSOCK:
            HC_LOG_ERROR(" ENOTSOCK not a socket");
            break;
        case ETIMEDOUT:
            HC_LOG_ERROR(" ETIMEDOUT timeout");
            break;
        default:
            HC_LOG_ERROR(" UNKNOWN ERROR: " << errno);
        }
    }
    return rtval;
}

int ip_socket::sendto(const struct sockaddr_storage* addr, const void *buf, const int buf_len)
{
    HC_LOG_TRACE("");
    int rtval = ::sendto(m_sockfd, buf, buf_len, 0, reinterpret_cast<const struct sockaddr*>(addr), sizeof(struct sockaddr));
    if(rtval == -1) {
        HC_LOG_ERROR (" sendto, errno: " << errno);
    }
    return rtval;
}

void ip_socket::set_multicast_ttl(const unsigned int& ttl)
{
    HC_LOG_TRACE("");
    m_ttl = ttl;
    int ret = 0;
    if (m_if_addr.ss_family == AF_INET) {
        HC_LOG_DEBUG ("Set IPv4 TTL.");
        ret = setsockopt(m_sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    }
    else if (m_if_addr.ss_family == AF_INET6) {
        HC_LOG_DEBUG ("Set IPv6 TTL.");
        ret = setsockopt(m_sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl));
    }
    else {
        HC_LOG_ERROR (" No IP address family/version informatio!");
        ret = -1;
    }
    if (ret < 0) {
        HC_LOG_ERROR (" set TTL, return: " << ret << "errno: " << errno
                                           << " socket: " << m_sockfd);
        m_ttl = DEFAULT_MULTICAST_TTL;
    }
}

void ip_socket::bind(const hamcast::uri& grp_uri)
{
    HC_LOG_TRACE("");
    struct addrinfo hints, *grp;
    int ret;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((ret = getaddrinfo (grp_uri.host().c_str(), grp_uri.port().c_str(), &hints, &grp)) != 0) {
        HC_LOG_ERROR ("getaddrinfo: " << gai_strerror (ret));
        return;
    }
    if (::bind(m_sockfd, grp->ai_addr, grp->ai_addrlen) == -1) {
        HC_LOG_ERROR (" bind, errno: " << errno);
        return;
    }
}
