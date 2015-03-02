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

#include <map>
#include <stdint.h>
#include <string>
#include <netinet/in.h>
#include <netdb.h>

#include "ip_instance.hpp"
#include "ip_socket.hpp"
#include "ip_exceptions.hpp"

#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_logging.h"
#include "hamcast/uri.hpp"

using namespace ipm;

using std::string;
using std::map;
using std::pair;
using std::set;
using std::vector;
using hamcast::uri;

/*** PRIVATE ***/

void ip_instance::notify_receive_loop()
{
    HC_LOG_TRACE("");
    boost::uint32_t pipe_msg = s_queue_event;
    if (write(write_handle(), &pipe_msg, sizeof(pipe_msg)) < 0)
        HC_LOG_ERROR ("Cannot write to pipe, on notify.");
}

void ip_instance::init()
{
    HC_LOG_TRACE("");
    if (pipe(m_pipe) != 0)
    {
        char* error_cstr = strerror(errno);
        std::string error_str = "pipe(): ";
        error_str += error_cstr;
        free(error_cstr);
        throw std::logic_error(error_str);
    }
    HC_LOG_DEBUG("Start receive loop.");
    m_loop = boost::thread(ip_instance::receive_loop_helper, this);
}

/*** PUBLIC ***/
ip_instance::ip_instance(hc_log_fun_t log_fun, hc_event_callback_t ecb,
                         hc_recv_callback_t rcb, hc_atomic_msg_size_callback_t mcb) :
        m_log_fun(log_fun), m_event_cb(ecb), m_recv_cb(rcb), m_msg_cb(mcb),
        m_if_index(0)
{
    HC_LOG_TRACE("");
    m_sendsock = new ip_socket();
    init();
}

ip_instance::ip_instance(hc_log_fun_t log_fun, hc_event_callback_t ecb,
                         hc_recv_callback_t rcb, hc_atomic_msg_size_callback_t mcb,
                         const unsigned int ifindex, const std::string& ifname,
                         sockaddr_storage ifaddr) :
        m_log_fun(log_fun), m_event_cb(ecb), m_recv_cb(rcb), m_msg_cb(mcb),
        m_if_addr(ifaddr), m_if_index(ifindex), m_if_name(ifname)
{
    HC_LOG_TRACE("");
    m_sendsock = new ip_socket(m_if_addr, m_if_index, m_if_name);
    init();
}

void ip_instance::kill_receive_loop()
{
    HC_LOG_TRACE("");
    boost::uint32_t pipe_msg = s_quit_event;
    if (write(write_handle(), &pipe_msg, sizeof(pipe_msg)) < 0)
        HC_LOG_ERROR("Cannot write to pipe, on exit.");
    m_loop.join();
}

void ip_instance::receive_loop()
{
    hc_set_log_fun(m_log_fun);
    HC_LOG_TRACE("ip instance receive loop");
    const int maxreads = 10;
    const int recvbuf_len = 65000;
    int maxfd=0;
    fd_set readset;
    fd_set readset_org;
    maxfd = read_handle();
    FD_ZERO(&readset);
    FD_ZERO(&readset_org);
    FD_SET(read_handle(), &readset_org);
    bool init = false;

    std::auto_ptr<ip_instance_msg> msg;
    std::map<uri, ip_socket*> l_sockets;
    std::map<uri, ip_socket*>::iterator sit;
    std::map<int, uri> l_uris;
    std::map<int, uri>::iterator uit;
    for (;;)
    {
        memcpy(&readset, &readset_org, sizeof(readset_org));
        if (select(maxfd + 1, &readset, 0, 0, 0) < 0) {
            // must not happen
            HC_LOG_FATAL("select() call failed");
            return;
        }
        if (FD_ISSET(read_handle(), &readset))
        {
            boost::uint32_t pipe_msg;
            if (read(read_handle(), &pipe_msg, sizeof(pipe_msg)) <0) {
                HC_LOG_ERROR ("Cannot read from pipe.");
                continue;
            }
            switch (pipe_msg)
            {
                case s_queue_event:
                {
                    do
                    {
                        msg.reset(m_queue.try_pop());
                    }
                    while (msg.get() == 0);
                    switch(msg->operation) {
                    case iio_join: {
                            uri grp_uri (msg->uri_arg);
                            HC_LOG_DEBUG("Join group: " << grp_uri.str());
                            if (!grp_uri.empty()) { // uri valid
                                sit = l_sockets.find(grp_uri);
                                if (sit == l_sockets.end()) {
                                    ip_socket *nsock = new ip_socket(m_if_addr, m_if_index, m_if_name);
                                    try {
                                        nsock->join(grp_uri);
                                        HC_LOG_DEBUG(" - group joined");
                                        nsock->bind(grp_uri);
                                        HC_LOG_DEBUG(" - socket bind");
                                        l_sockets.insert(std::pair<uri, ip_socket*>(grp_uri,nsock));
                                        HC_LOG_DEBUG(" - add socket to list");
                                        m_groups.insert(grp_uri);
                                        HC_LOG_DEBUG(" - add group to list");
                                        init = true;
                                        HC_LOG_DEBUG(" - set init true for select");
                                    }
                                    catch (ipm_setsockopt_exception& e) {
                                        std::cerr << "setsockopt join error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (ipm_mcast_operation_exception& e) {
                                        std::cerr << "multicast join operation error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (ipm_bind_exception& e) {
                                        std::cerr << "Bind error:";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (ipm_exception& e) {
                                        std::cerr << "IP module error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (std::exception& e) {
                                        std::cerr << "Unknown error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                }
                                else {
                                    HC_LOG_DEBUG ("Group already joined.");
                                }
                            }
                            else {
                                HC_LOG_DEBUG ("Group name empty or invalid.");
                            }
                            break;
                        }
                    case iio_leave: {
                            uri grp_uri (msg->uri_arg);
                            if (!grp_uri.empty()) { // uri valid
                                sit = l_sockets.find(grp_uri);
                                if (sit != l_sockets.end()) {
                                    try {
                                        sit->second->leave(grp_uri);
                                        delete (sit->second);   // delete object, not handled by erase below
                                        l_sockets.erase(sit);
                                        m_groups.erase(grp_uri);
                                        init = true;
                                    }
                                    catch (ipm_setsockopt_exception& e) {
                                        std::cerr << "setsockopt leave error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (ipm_mcast_operation_exception& e) {
                                        std::cerr << "multicast leave operation error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (ipm_exception& e) {
                                        std::cerr << "IP module error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                    catch (std::exception& e) {
                                        std::cerr << "Unknown error: ";
                                        std::cerr << e.what() << std::endl;
                                    }
                                }
                                else {
                                    HC_LOG_DEBUG ("Group not found.");
                                }
                            }
                            else {
                                HC_LOG_DEBUG ("Group name empty or invalid.");
                            }
                            break;
                        }
                    default:
                        HC_LOG_DEBUG ("Unknown operation type.");
                    }
                    break;
                }
                case s_quit_event:
                {
                    return;
                }
            }
        }
        HC_LOG_DEBUG ("Run socket receive loop");
        for (int i=0; i < maxfd + 1; ++i) {
            if (i == read_handle())
                continue;
            else {
                if (FD_ISSET(i, &readset)) {
                    HC_LOG_DEBUG (" - read from socket " << i);
                    for(int j=0; j < maxreads; ++j) {
                        uit = l_uris.find(i);
                        if (uit == l_uris.end()) {
                            HC_LOG_DEBUG ("Unable to map sockfd to uri.");
                            continue;
                        }
                        uri& group = uit->second;
                        sit = l_sockets.find(group);
                        if (sit == l_sockets.end()) {
                            HC_LOG_DEBUG ("Unable to map uri to ip_socket.");
                            continue;
                        }
                        ip_socket *recvsock = sit->second;
                        char recvbuf[recvbuf_len];
                        int ret = 0;
                        if ((ret = recvsock->recv_async(recvbuf, recvbuf_len))>0) {
                            HC_LOG_DEBUG ("Received msg for group: " << group.str());
                            m_recv_cb(m_handle, recvbuf, ret, new uri(group), NULL);
                        }
                        else {
                            HC_LOG_DEBUG ("Recv failure, no data available on socket.");
                            //recvsock->init ();
                            j = maxreads;
                        }
                    }
                }
            }
        }

        if (init) {
            HC_LOG_DEBUG ("Run init for select.");
            FD_ZERO(&readset_org);
            maxfd = read_handle();
            FD_SET (read_handle(), &readset_org);
            for (sit = l_sockets.begin(); sit != l_sockets.end(); ++sit) {
                int sockfd = sit->second->get_sockfd();
                l_uris.insert(std::pair<int, uri>(sockfd, sit->first));
                FD_SET (sockfd, &readset_org);
                if (sockfd > maxfd)
                    maxfd = sockfd;
            }
            HC_LOG_DEBUG ("Max sock id: " << maxfd << "." );
            init = false;
        }
    }
}

int ip_instance::join(const uri& group_uri)
{
    HC_LOG_TRACE("");
    m_queue.unsafe_push(new ip_instance_msg(iio_join, group_uri));
    HC_MEMORY_BARRIER();
    notify_receive_loop();
    return HC_SUCCESS;
}

int ip_instance::leave(const uri& group_uri)
{
    HC_LOG_TRACE("");
    m_queue.unsafe_push(new ip_instance_msg(iio_leave, group_uri));
    HC_MEMORY_BARRIER();
    notify_receive_loop();
    return HC_SUCCESS;
}

int ip_instance::send(const uri& group_uri, const void* buf, int len, unsigned char ttl)
{
    HC_LOG_TRACE("");
    int numbytes = 0;
    // check destination group
    if(group_uri.empty()){
        return HC_INVALID_URI;
    }

    // set multicast ttl of socket
    if (ttl < 1) {
        HC_LOG_WARN("Invalid TTL value: " << ttl << ". Use default 1.");
        ttl = DEFAULT_MULTICAST_TTL;
    }
    if (m_sendsock->get_multicast_ttl () != ttl) {
        m_sendsock->set_multicast_ttl (ttl);
    }

    try {
        struct addrinfo hints, *grp;
        int ret;

        memset (&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        if ((ret = getaddrinfo (group_uri.group().c_str(), group_uri.port().c_str(), &hints, &grp)) != 0) {
            HC_LOG_WARN ("sendto, getaddrinfo: " << gai_strerror (ret));
            throw ipm_address_exception ("sendto, getaddrinfo");
        }
        struct sockaddr_storage addr;
        memcpy (&addr, grp->ai_addr, grp->ai_addrlen);
        numbytes = m_sendsock->sendto(&addr, buf, len);
        freeaddrinfo(grp);
    }
    catch (ipm_exception &e) {
        HC_LOG_ERROR("Exception in IP module: " << e.what());
        return HC_UNKNOWN_ERROR;
    }
    return numbytes;
}

uri ip_instance::map(const uri& group_uri)
{
    HC_LOG_TRACE("");
    uri result;
    /*
     * FIXME: need to check if uri contains valid IP,
     * otherwise return NULL or query some other mapping service
     */
    if (!group_uri.empty() && (group_uri.ham_namespace() == "ip")) {
        HC_LOG_DEBUG ("URI valid, return mapping (copy).");
        result = uri (group_uri);
    }
    else {
        HC_LOG_DEBUG("Invalid URI, return NULL mapping.");
    }
    return result;
}

void ip_instance::neighbor_set(vector<uri>& result)
{
    HC_LOG_TRACE("");
    set<uri>::iterator it;
    set<uri> t_neighbors = m_neighbors;
    for (it = t_neighbors.begin(); it != t_neighbors.end(); ++it) {
        uri u = (*it);
        std::stringstream ss;
        ss << u.ham_scheme() << ":";
        ss << u.ham_namespace() << ":";
        ss << u.group() << ":";
        if (!u.instantiation().empty()) {
            ss << "@" << u.instantiation();
        }
        u = uri (ss.str());
        if (!u.empty()) {
            result.push_back(u);
        }
    }
}

void ip_instance::group_set(std::vector<std::pair<hamcast::uri, int> >& result)
{
    HC_LOG_TRACE("");
    set<uri>::iterator it;
    set<uri> t_groups = m_groups;
    for (it = t_groups.begin(); it != t_groups.end(); ++it) {
        uri u = (*it);
        std::stringstream ss;
        ss << u.ham_scheme() << ":";
        ss << u.ham_namespace() << ":";
        ss << u.group() << ":";
        if (!u.instantiation().empty()) {
            ss << "@" << u.instantiation();
        }
        u = uri (ss.str());
        if (!u.empty()) {
            result.push_back(pair<uri, int>(u, HC_LISTENER_STATE));
        }
    }
}

void ip_instance::children_set (vector<uri>&, const uri&)
{
    HC_LOG_TRACE("");
}

void ip_instance::parent_set(vector<uri> &result, const uri&)
{
    HC_LOG_TRACE("");
    if (!m_querier.empty()) {
        result.push_back(m_querier);
    }
}

int ip_instance::designated_host(const uri&)
{
    HC_LOG_TRACE("");
    return false;
}
