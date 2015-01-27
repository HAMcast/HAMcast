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

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

#include "tunnel.hpp"
#include "tunnel_message.hpp"

using namespace tunnel_module;
using hamcast::uri;
using std::set;
using std::string;
using std::vector;
using std::pair;
using std::map;

tunnel::tunnel (hc_log_fun_t lf, hc_event_callback_t ecb,
                hc_recv_callback_t rcb, hc_atomic_msg_size_callback_t mcb)
{
    m_log_fun = lf;
    m_recv_cb = rcb;
    m_event_cb = ecb;
    m_msize_cb = mcb;

    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_DGRAM;
    hints.ai_flags      = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, TUNNEL_DEFAULT_PORT, &hints, &servinfo)) != 0) {
        HC_LOG_ERROR ("tunnel: getaddrinfo: " << gai_strerror(rv));
        return;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((m_recvsock = ::socket (p->ai_family, SOCK_DGRAM, 0)) == -1) {
            HC_LOG_ERROR ("tunnel: socket.");
            continue;
        }
        HC_LOG_DEBUG( "tunnel: socket open" );
        if (::bind (m_recvsock, p->ai_addr, p->ai_addrlen) == -1) {
            close (m_recvsock);
            HC_LOG_ERROR("tunnel: bind.");
            continue;
        }
        HC_LOG_DEBUG( "tunnel: socket bind" );
        memcpy(&m_local_addr,p->ai_addr, p->ai_addrlen);
        break;
    }

    if (p == NULL) {
        HC_LOG_ERROR ("tunnel: failed to bind socket.");
        return;
    }
    freeaddrinfo(servinfo);
    init();
}

tunnel::tunnel (hc_log_fun_t lf, hc_event_callback_t ecb,
                hc_recv_callback_t rcb, hc_atomic_msg_size_callback_t mcb,
        const std::string& la, const std::string& lp,
        const std::string& ra, const std::string& rp)
{
    m_log_fun = lf;
    m_recv_cb = rcb;
    m_event_cb = ecb;
    m_msize_cb = mcb;

    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_DGRAM;
    hints.ai_flags      = AI_PASSIVE;

    if (!la.empty() && !lp.empty()) {
        if ((rv = getaddrinfo(la.c_str(), lp.c_str(), &hints, &servinfo)) != 0) {
            HC_LOG_ERROR ("tunnel: getaddrinfo: " << gai_strerror(rv));
            return;
        }
    }
    else if (la.empty() && !lp.empty()) {
        if ((rv = getaddrinfo(NULL, lp.c_str(), &hints, &servinfo)) != 0) {
            HC_LOG_ERROR ("tunnel: getaddrinfo: " << gai_strerror(rv));
            return;
        }
    }
    else {
        if ((rv = getaddrinfo(NULL, TUNNEL_DEFAULT_PORT, &hints, &servinfo)) != 0) {
            HC_LOG_ERROR ("tunnel: getaddrinfo: " << gai_strerror(rv));
            return;
        }
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((m_recvsock = ::socket (p->ai_family, SOCK_DGRAM, 0)) == -1) {
            HC_LOG_ERROR ("tunnel: socket.");
            continue;
        }
        HC_LOG_DEBUG( "tunnel: socket open" );
        if (::bind (m_recvsock, p->ai_addr, p->ai_addrlen) == -1) {
            close (m_recvsock);
            HC_LOG_ERROR("tunnel: bind.");
            continue;
        }
        HC_LOG_DEBUG( "tunnel: socket bind" );
        memcpy(&m_local_addr,p->ai_addr, p->ai_addrlen);
        break;
    }

    if (p == NULL) {
        HC_LOG_ERROR ("tunnel: failed to bind socket.");
        return;
    }
    freeaddrinfo(servinfo);

    if (!ra.empty() && !rp.empty()) {
        if ((rv = getaddrinfo(ra.c_str(), rp.c_str(), &hints, &servinfo)) != 0) {
            HC_LOG_ERROR ("tunnel: getaddrinfo: " << gai_strerror(rv));
            return;
        }
        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((m_sendsock = ::socket (p->ai_family, SOCK_DGRAM, 0)) == -1) {
                HC_LOG_ERROR ("tunnel: socket.");
                continue;
            }
            memcpy(&m_remote_addr,p->ai_addr, p->ai_addrlen);
            break;
        }
        if (p == NULL) {
            HC_LOG_ERROR ("tunnel: failed to bind socket.");
            return;
        }
        freeaddrinfo(servinfo);
    }
    else {
        m_sendsock = ::socket (AF_INET, SOCK_DGRAM, 0);
    }
    init();
}

void tunnel::init()
{
    HC_LOG_TRACE("");
    m_open = true;
    m_loop = boost::thread(tunnel::receive_loop_helper, this);
}

void tunnel::receive_loop()
{
    hc_set_log_fun(m_log_fun);
    HC_LOG_TRACE("");
    size_t buflen = 65 * 1024;
    char recv_buffer[buflen];
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof(their_addr);
    while (true) {
        int ret = 0;
        if ((ret = ::recvfrom(m_recvsock, recv_buffer, buflen-1, 0,
                              (struct sockaddr*)&their_addr, &addr_len)) < 0) {
            HC_LOG_ERROR ("tunnel receive");
        }
        else {
            tunnel_message recv_msg (recv_buffer, ret);
            switch (recv_msg.type()) {
            case TUN_MSG_INVALID: {
                HC_LOG_DEBUG (" - received INVALID msg: dropping");
                break;
            }
            case TUN_MSG_DATA: {
                HC_LOG_DEBUG (" - received DATA msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                m_recv_cb (m_handle, recv_msg.payload(), recv_msg.payload_size(), new uri(recv_msg.group_uri()), NULL);
                break;
            }
            case TUN_MSG_JOIN: {
                HC_LOG_DEBUG (" - received JOIN msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                boost::mutex::scoped_lock scopedLock(m_local_groups_mutex);
                if ((m_local_groups.count (recv_msg.group_uri()) == 0) && !(recv_msg.group_uri()).empty()) {
                    boost::mutex::scoped_lock scopedLock(m_remote_groups_mutex);
                    std::map<uri, set<SA> >::iterator it = m_remote_groups.find(recv_msg.group_uri());
                    if (it != m_remote_groups.end()) {
                        set<SA>& children = it->second;
                        children.insert(their_addr);
                    }
                    else {
                        set<SA> tmp;
                        tmp.insert(their_addr);
                        m_remote_groups.insert(pair<uri, set<SA> >(recv_msg.group_uri(), tmp));
                    }
                    try {
                        tunnel_message join_ack (TUN_MSG_JOIN_ACK, recv_msg.group_uri());
                        sendto (join_ack, their_addr);
                    }
                    catch(...){
                        HC_LOG_ERROR("Send join ack failed, group uri: " << recv_msg.group_uri ());
                    }
                }
                break;
            }
            case TUN_MSG_JOIN_ACK: {
                HC_LOG_DEBUG (" - received JOIN ACK msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                boost::mutex::scoped_lock scopedLock(m_tmp_groups_mutex);
                if (m_tmp_groups.count(recv_msg.group_uri()) != 0) {
                    boost::mutex::scoped_lock scopedLock(m_local_groups_mutex);
                    m_local_groups.insert(recv_msg.group_uri());
                    m_tmp_groups.erase(recv_msg.group_uri());
                }
                break;
            }
            case TUN_MSG_JOIN_ERROR: {
                HC_LOG_DEBUG (" - received JOIN ERROR msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                break;
            }
            case TUN_MSG_LEAVE: {
                HC_LOG_DEBUG (" - received LEAVE msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                {
                    boost::mutex::scoped_lock scopedLock(m_remote_groups_mutex);
                    m_remote_groups.erase(recv_msg.group_uri());
                }
                tunnel_message leave_ack (TUN_MSG_LEAVE_ACK, recv_msg.group_uri());
                sendto (leave_ack, their_addr);
                break;
            }
            case TUN_MSG_LEAVE_ACK: {
                HC_LOG_DEBUG (" - received LEAVE ACK msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                break;
            }
            case TUN_MSG_LEAVE_ERROR: {
                HC_LOG_DEBUG (" - received LEAVE ERROR msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                break;
            }
            case TUN_MSG_QUERY: {
                HC_LOG_DEBUG (" - received QUERY msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                break;
            }
            case TUN_MSG_REPORT: {
                HC_LOG_DEBUG (" - received REPORT msg: " << recv_msg.group_uri().str() << " msg SIZE: " << recv_msg.size());
                break;
            }
            default: {
                HC_LOG_ERROR ("Received invalid message, SIZE: " << ret);
                break;
            }
            }
        }
    }
}

int tunnel::sendto(const tunnel_message& msg, SA dst)
{
    HC_LOG_TRACE("");
    int ret = 0;
    if ((ret = ::sendto(m_sendsock, reinterpret_cast<const void*>(msg.buffer()), msg.size(), 0, reinterpret_cast<struct sockaddr*>(&dst), sizeof(struct sockaddr_in))) < 0) {
        HC_LOG_ERROR (" sendto return: " << ret << " errno: " << errno);
    }
    return ret;
}

int tunnel::send (const uri& group_uri, const void *buf, size_t len, size_t)
{
    HC_LOG_TRACE ("");
    int ret = 0;
    try {
        uri tmp;
        if (!group_uri.port ().empty ()) {
            std::stringstream ss;
            ss << group_uri.scheme () + "://";
            if (!group_uri.user_information ().empty ()) {
                ss << group_uri.user_information () << "@";
            }
            ss << group_uri.host();
            tmp = uri (ss.str ());
        }
        else {
            tmp = group_uri;
        }
        boost::mutex::scoped_lock scopedLock(m_remote_groups_mutex);
        std::map<uri, set<SA> >::iterator it = m_remote_groups.find(tmp);
        if (it != m_remote_groups.end()) {
            tunnel_message data_msg (TUN_MSG_DATA, group_uri, static_cast<const char*>(buf), len);
            set<SA> children = it->second;
            set<SA>::iterator cit;
            for (cit = children.begin(); cit != children.end(); ++cit) {
                ret = sendto (data_msg, *cit);
            }
        }
        else {
            HC_LOG_DEBUG ("tunnel send: group not joined by tunnel endpoint.");
        }
    }
    catch (...) {
        HC_LOG_ERROR ("Send error!");
        return HC_UNKNOWN_ERROR;
    }
    return ret;
}

int tunnel::join (const uri& group_uri)
{
    HC_LOG_TRACE ("");
    m_local_groups.insert (group_uri);
    return HC_SUCCESS;
    
    try {
        tunnel_message join_msg (TUN_MSG_JOIN, group_uri);
        if ((m_remote_addr.ss_family == AF_INET) || (m_remote_addr.ss_family == AF_INET6)) {
            if (sendto (join_msg, m_remote_addr) < 0) {
                throw 42;
            }
            boost::mutex::scoped_lock scopedLock(m_tmp_groups_mutex);
            m_tmp_groups.insert(group_uri);
        }
        else {
            boost::mutex::scoped_lock scopedLock(m_local_groups_mutex);
            m_local_groups.insert (group_uri);
        }
    }
    catch (...) {
        return HC_UNKNOWN_ERROR;
    }
    return HC_SUCCESS;
}

int tunnel::leave (const uri& group_uri)
{
    HC_LOG_TRACE ("");
    try {
        tunnel_message leave_msg (TUN_MSG_LEAVE, group_uri);
        if ((m_remote_addr.ss_family == AF_INET) || (m_remote_addr.ss_family == AF_INET6)) {
            if (sendto (leave_msg, m_remote_addr) < 0) {
                throw 42;
            }
        }
        boost::mutex::scoped_lock scopedLock(m_local_groups_mutex);
        m_local_groups.erase(group_uri);
    }
    catch (...) {
        return HC_UNKNOWN_ERROR;
    }
    return HC_SUCCESS;
}

uri tunnel::map (const uri& group_uri)
{
    return uri(group_uri);
}

void tunnel::parent_set(vector<uri>& result, const uri& group_uri)
{
    HC_LOG_TRACE("");
    //std::cout << "Parentset: " << group_uri.str () << std::endl;
    char addr_str[INET6_ADDRSTRLEN];
    if (m_remote_addr.ss_family == AF_INET) {
        if (inet_ntop (AF_INET, &(((struct sockaddr_in*)&m_remote_addr)->sin_addr), addr_str, sizeof(addr_str)) == NULL) {
            HC_LOG_ERROR ("Failed to convert address to string.");
        }
        else {
            std::stringstream ss;
            ss << "tun://" << addr_str << ":";
            ss << ntohs(((struct sockaddr_in*)&m_remote_addr)->sin_port);
            //std::cout << "Parent: " << ss.str () << std::endl;
            result.push_back(uri(ss.str ()));
        }
    }
    else if (m_remote_addr.ss_family == AF_INET6) {
        if (inet_ntop (AF_INET6, &(((struct sockaddr_in6*)&m_remote_addr)->sin6_addr), addr_str, sizeof(addr_str)) == NULL) {
            HC_LOG_ERROR ("Failed to convert address to string.");
        }
        else {
            std::stringstream ss;
            ss << "tun://" << addr_str << ":";
            ss << ntohs(((struct sockaddr_in6*)&m_remote_addr)->sin6_port);
            result.push_back(uri(ss.str ()));
        }
    }
}

void tunnel::children_set(vector<uri>& result, const uri& group_uri)
{
    HC_LOG_TRACE("");
    //std::cout << "Childrenset: " << group_uri.str () << std::endl;
    boost::mutex::scoped_lock scopedLock(m_remote_groups_mutex);
    std::map<uri,set<SA> >::iterator it = m_remote_groups.find(group_uri);
    if (it != m_remote_groups.end()) {
        std::cout << "group: " << it->first.str() << std::endl;
        set<SA>& children = it->second;
        set<SA>::iterator cit;
        for (cit = children.begin(); cit != children.end(); ++cit) {
            SA t_addr = *cit;
            char addr_str[INET6_ADDRSTRLEN];
            if (t_addr.ss_family == AF_INET) {
                if (inet_ntop (AF_INET, &(((struct sockaddr_in*)&t_addr)->sin_addr), addr_str, sizeof(addr_str)) == NULL) {
                    HC_LOG_ERROR ("Failed to convert address to string.");
                }
                else {
                    std::stringstream ss;
                    ss << "tun://" << addr_str << ":";
                    ss << ntohs(((struct sockaddr_in*)&m_remote_addr)->sin_port);
                    //std::cout << "Child: " << ss.str () << std::endl;
                    result.push_back(uri(ss.str ()));
                }
            }
            else if (m_remote_addr.ss_family == AF_INET6) {
                if (inet_ntop (AF_INET6, &(((struct sockaddr_in6*)&t_addr)->sin6_addr), addr_str, sizeof(addr_str)) == NULL) {
                    HC_LOG_ERROR ("Failed to convert address to string.");
                }
                else {
                    std::stringstream ss;
                    ss << "tun://" << addr_str << ":";
                    ss << ntohs(((struct sockaddr_in6*)&m_remote_addr)->sin6_port);
                    result.push_back(uri(ss.str ()));
                }
            }
        }
    }
}

bool tunnel::designated_host (const uri&)
{
    HC_LOG_TRACE("");
    return false;
}

void tunnel::neighbor_set (vector<uri>& result)
{
    HC_LOG_TRACE("");
    char addr_str[INET6_ADDRSTRLEN];
    if (m_remote_addr.ss_family == AF_INET) {
        if (inet_ntop (AF_INET, &(((struct sockaddr_in*)&m_remote_addr)->sin_addr), addr_str, sizeof(addr_str)) == NULL) {
            HC_LOG_ERROR ("Failed to convert address to string.");
        }
        else {
            std::stringstream ss;
            ss << "remote://" << addr_str << ":";
            ss << ntohs(((struct sockaddr_in*)&m_remote_addr)->sin_port);
            result.push_back(uri(ss.str ()));
        }
    }
    else if (m_remote_addr.ss_family == AF_INET6) {
        if (inet_ntop (AF_INET6, &(((struct sockaddr_in6*)&m_remote_addr)->sin6_addr), addr_str, sizeof(addr_str)) == NULL) {
            HC_LOG_ERROR ("Failed to convert address to string.");
        }
        else {
            std::stringstream ss;
            ss << "remote://" << addr_str << ":";
            ss << ntohs(((struct sockaddr_in6*)&m_remote_addr)->sin6_port);
            result.push_back(uri(ss.str ()));
        }
    }
}

void tunnel::group_set(vector<pair<uri,int> >& result)
{
    HC_LOG_TRACE ("");
    //std::cout << "Groupset" << std::endl;
    set<uri>::iterator it;
    m_local_groups_mutex.lock();
    for (it = m_local_groups.begin(); it != m_local_groups.end(); ++it) {
        uri u = *it;
        if (!u.port().empty()) {
            std::stringstream ss;
            ss << u.scheme() + "://";
            if (!u.user_information().empty()) {
                ss << u.user_information() << "@";
            }
            ss << u.host();
            u = uri(ss.str());
        }
        //std::cout << "Local Group: " << u.str () << std::endl;
        result.push_back(pair<uri,int>(u, HC_LISTENER_STATE));
    }
    m_local_groups_mutex.unlock();
    std::map<uri, set<SA> >::iterator rit;
    m_remote_groups_mutex.lock();
    for (rit = m_remote_groups.begin(); rit != m_remote_groups.end(); ++rit) {
        uri u = rit->first;
        if (!u.port().empty()) {
            std::stringstream ss;
            ss << u.scheme() + "://";
            if (!u.user_information().empty()) {
                ss << u.user_information() << "@";
            }
            ss << u.host();
            u = uri(ss.str());
        }
        //std::cout << "Remote Group: " << u.str () << std::endl;
        result.push_back(pair<uri,int>(u, HC_SENDER_STATE));
    }
    m_remote_groups_mutex.unlock();
}

void tunnel::shutdown()
{
    HC_LOG_TRACE ("Shutdown tunnel ...");
    // tear down all connections
    close (m_sendsock);
    close (m_recvsock);
}

std::string tunnel::get_ifaddr ()
{
    char ifa_str[INET6_ADDRSTRLEN];
    std::stringstream ss;
    if (m_local_addr.ss_family == AF_INET) {
        inet_ntop (AF_INET, &((struct sockaddr_in*)&m_local_addr)->sin_addr, ifa_str, sizeof(ifa_str));
        ss << ifa_str << ":" << ((struct sockaddr_in*)&m_local_addr)->sin_port;
    }
    else if (m_local_addr.ss_family == AF_INET6) {
        inet_ntop (AF_INET6, &((struct sockaddr_in6*)&m_local_addr)->sin6_addr, ifa_str, sizeof(ifa_str));
        ss << ifa_str << ":" << ((struct sockaddr_in6*)&m_local_addr)->sin6_port;
    }
    return ss.str ();
}

/* XXX: DEBUG FUNCTIONS FOR TESTING ONLY */

void tunnel::__add_remote_listener(uri &group_uri, SA &node)
{
    HC_LOG_TRACE("");
    boost::mutex::scoped_lock scopedLock(m_remote_groups_mutex);
    std::map<uri, set<SA> >::iterator it = m_remote_groups.find(group_uri);
    if (it != m_remote_groups.end()) {
        //std::cout << "Group exists, add child" << std::endl;
        set<SA>& children = it->second;
        children.insert(node);
    }
    else {
        //std::cout << "Group new, add child" << std::endl;
        set<SA> tmp;
        tmp.insert(node);
        m_remote_groups.insert(pair<uri, set<SA> >(group_uri, tmp));
    }
}
