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

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sstream>
#include <map>
#include <set>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

#include "hamcast/exception.hpp"
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_logging.h"
#include "hamcast/hamcast_module.h"

#include "tunnel.hpp"

using namespace tunnel_module;
using std::pair;
using std::map;
using std::set;
using std::string;
using hamcast::uri;

/*
 * MIDDLEWARE.INI EXAMPLE
 *
 * [tunnel_module]
 * file=<libtunnel>
 * tun0.localaddr=0.0.0.0
 * tun0.localport=8000
 * tun0.remoteaddr=1.2.3.4
 * tun0.remoteport=9000
 * tun1.remoteaddr=5.6.7.8
 * tun1.remoteport=9001
 * tun2.localaddr=192.168.0.2
 */

const size_t c_tun_default_mtu = 8*1024;
int s_tunnel_counter = 0;

boost::thread listen_thread;

inline tunnel* self(hc_module_instance_t instance)
{
    return reinterpret_cast<tunnel*>(instance);
}

void *get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

#if 0
void listen_loop (const string& port,
                  hc_log_fun_t log_fun,
                  struct hc_module_handle* mod_handle,
                  hc_new_instance_callback_t new_instance_cb,
                  hc_recv_callback_t recv_cb,
                  hc_event_callback_t event_cb,
                  hc_atomic_msg_size_callback_t msg_size_cb,
                  size_t msg_size)
{
    struct sockaddr_storage remote_addr, local_addr;
    socklen_t sin_size;
    struct addrinfo hints, *dstinfo, *p;
    int ret, sockfd, tunfd;
    int yes = 1;
    string tunnel_name = "rtun";
    size_t tun_mtu = c_tun_default_mtu;

    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // resolve uri to address struct
    if ((ret = getaddrinfo (NULL, port.c_str() , &hints, &dstinfo)) != 0) {
        HC_LOG_ERROR ("getaddrinfo: " << gai_strerror (ret));
        return;
    }

    for (p=dstinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            HC_LOG_ERROR ("listen_loop: socket");
            continue;
        }

        if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            HC_LOG_ERROR ("listen_loop: setsockopt");
            return;
        }

        if (bind (sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close (sockfd);
            HC_LOG_ERROR ("listen_loop: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        HC_LOG_ERROR ("listen_loop: failed to bind!");
        return;
    }

    freeaddrinfo(dstinfo);

    if (listen (sockfd, 10) == -1) {
        HC_LOG_ERROR ("listen_loop: listen");
        return;
    }

    HC_LOG_DEBUG ("Waiting for remote tunnel connections ...");

    char s[INET6_ADDRSTRLEN];
    while (true) {
        sin_size = sizeof (remote_addr);
        tunfd = accept (sockfd, (struct sockaddr*)&remote_addr, &sin_size);
        if (tunfd == -1) {
            HC_LOG_ERROR ("listen_loop: accept");
            continue;
        }

        inet_ntop (remote_addr.ss_family,
                   get_in_addr((struct sockaddr *)&remote_addr),
                   s, sizeof(s));

        HC_LOG_DEBUG ("listen_loop: got tunnel connection from " << s);
        ++s_tunnel_counter;
        // setup new tunnel interface
        int la_len = sizeof (local_addr);
        std::copy (p->ai_addr, p->ai_addr + sizeof(remote_addr), (struct sockaddr*) &remote_addr);
        if (getsockname(sockfd, (struct sockaddr*)&local_addr, (socklen_t*)&la_len) == -1) {
            HC_LOG_ERROR ("Failed to get IP of local tunnel endpoint!");
        }
        // set interface meta infos
        std::string name_key = "if_name";
        std::string name_val = tunnel_name;
        std::stringstream ss;
        ss << s_tunnel_counter;
        tunnel_name += ss.str();
        std::string addr_key = "if_addr";
        std::string addr_val = "tun://" + string(s);
        std::string tech_key = "if_tech";
        std::string tech_val = "IP";
        if (local_addr.ss_family == AF_INET) {
            tech_val += "v4_";
        }
        else if (local_addr.ss_family == AF_INET6) {
            tech_val += "v6_";
        }
        else {
            tech_val += "invalid_";
        }
        tech_val += "TCP_TUNNEL";

        hc_kvp_list_t name;
        name.key = name_key.c_str();
        name.value = name_val.c_str();
        name.next = 0;

        hc_kvp_list_t addr;
        addr.key = addr_key.c_str();
        addr.value = addr_val.c_str();
        addr.next = &name;

        hc_kvp_list_t tech;
        tech.key = tech_key.c_str();
        tech.value = tech_val.c_str();
        tech.next = &addr;
        tunnel *tun = new tunnel (log_fun, recv_cb, event_cb, msg_size_cb, sockfd, local_addr, remote_addr);
        hc_module_instance_handle_t hdl = new_instance_cb(tun, mod_handle, &tech, tun_mtu);
        tun->set_handle(hdl);
    }
}

#endif
hc_uri_list_t uri_list (const std::set<hamcast::uri>& uriset)
{
    HC_LOG_TRACE("Convert set to uri list.");

    hc_uri_list_t res;
    res.uri_str = NULL;
    res.uri_obj = NULL;
    res.type    = HC_IGNORED;
    res.next    = NULL;

    hc_uri_list_t *tmp = &res;
    std::set<hamcast::uri>::iterator it;
    for (it = uriset.begin(); it != uriset.end(); ++it) {
        tmp->uri_str = NULL;
        tmp->uri_obj = new hamcast::uri(*it);
        tmp->type    = HC_IGNORED;
        tmp->next    = NULL;
        tmp->next    = new hc_uri_list_t();
        tmp          = tmp->next;
    }
    delete (tmp->next);
    tmp->next = NULL;
    return res;
}

typedef map<string, pair<string,string> > endpoints;
extern "C" void hc_init(hc_log_fun_t log_fun,
                        struct hc_module_handle* mod_handle,
                        hc_new_instance_callback_t new_instance_cb,
                        hc_recv_callback_t recv_cb,
                        hc_event_callback_t event_cb,
                        hc_atomic_msg_size_callback_t msg_size_cb,
                        size_t msg_size,
                        hc_kvp_list_t* kvp_list)
{
     // setup log function
    hc_set_log_fun(log_fun);
    HC_LOG_TRACE("");
    hc_kvp_list_t* kvp = kvp_list;
    endpoints local;
    endpoints remote;
    set<string> tunnels;
    size_t tun_mtu = c_tun_default_mtu;
    // process module data from middleware.ini, see example above
    HC_LOG_DEBUG ("process parameter list");
    while (kvp) {
        string key = boost::to_upper_copy(string(kvp->key));
        string val = boost::to_upper_copy(string(kvp->value));
        if (key.find("LOCALADDR") != string::npos) {
            HC_LOG_DEBUG ("Local tunnel endpoint:");
            size_t name_sep = key.find_first_of('.');
            string t_name = key.substr(0, name_sep);
            tunnels.insert (t_name);
            HC_LOG_DEBUG (" - name: " << t_name << ", addr: " << val);
            endpoints::iterator it = local.find(t_name);
            if (it != local.end()) {
                pair<string, string>& p = it->second;
                p.first = val;
            }
            else {
                local.insert(pair<string, pair<string,string> >(t_name, pair<string, string>(val,"")));
            }
        }
        else if (key.find("LOCALPORT") != string::npos) {
            HC_LOG_DEBUG ("Local tunnel endpoint:");
            size_t name_sep = key.find_first_of('.');
            string t_name = key.substr(0, name_sep);
            tunnels.insert (t_name);
            HC_LOG_DEBUG (" - name: " << t_name << ", port: " << val);
            endpoints::iterator it = local.find(t_name);
            if (it != local.end()) {
                pair<string, string>& p = it->second;
                p.second = val;
            }
            else {
                local.insert(pair<string, pair<string,string> >(t_name, pair<string, string>("",val)));
            }
        }
        else if (key.find("REMOTEADDR") != string::npos) {
            HC_LOG_DEBUG ("Remote tunnel endpoint:");
            size_t name_sep = key.find_first_of('.');
            string t_name = key.substr(0, name_sep);
            tunnels.insert (t_name);
            HC_LOG_DEBUG (" - name: " << t_name << ", addr: " << val);
            endpoints::iterator it = remote.find(t_name);
            if (it != remote.end()) {
                pair<string, string>& p = it->second;
                p.first = val;
            }
            else {
                remote.insert(pair<string, pair<string, string> >(t_name, pair<string, string>(val,"")));
            }
        }
        else if (key.find("REMOTEPORT") != string::npos) {
            HC_LOG_DEBUG ("Remote tunnel endpoint:");
            size_t name_sep = key.find_first_of('.');
            string t_name = key.substr(0, name_sep);
            tunnels.insert (t_name);
            HC_LOG_DEBUG (" - name: " << t_name << ", port: " << val);
            endpoints::iterator it = remote.find(t_name);
            if (it != remote.end()) {
                pair<string, string>& p = it->second;
                p.second = val;
            }
            else {
                remote.insert(pair<string, pair<string, string> >(t_name, pair<string, string>("",val)));
            }
        }
        else {
            HC_LOG_DEBUG(" - got unknown param: " << key << ", width val: " << val);
        }
        kvp = kvp->next;
    }
    set<string>::iterator it;
    for (it = tunnels.begin (); it != tunnels.end (); ++it) {
        string l_addr, r_addr, l_port, r_port;
        string t_name = *it;
        endpoints::iterator eit = local.find(t_name);
        if (eit != local.end ()) {
            l_addr = eit->second.first;
            l_port = eit->second.second;
        }
        eit = remote.find(t_name);
        if (eit != remote.end ()) {
            r_addr = eit->second.first;
            r_port = eit->second.second;
        }
        // set interface meta infos
        std::string name_key = "if_name";
        std::string name_val = boost::to_lower_copy(t_name);
        std::string addr_key = "if_addr";
        std::string addr_val = "tun://" + l_addr + ":" + l_port;
        std::string tech_key = "if_tech";
        std::string tech_val = "tunnel";

        hc_kvp_list_t name;
        name.key = name_key.c_str();
        name.value = name_val.c_str();
        name.next = 0;

        hc_kvp_list_t addr;
        addr.key = addr_key.c_str();
        addr.value = addr_val.c_str();
        addr.next = &name;

        hc_kvp_list_t tech;
        tech.key = tech_key.c_str();
        tech.value = tech_val.c_str();
        tech.next = &addr;
        tunnel *tun = new tunnel (log_fun, event_cb, recv_cb, msg_size_cb,
                                  l_addr, l_port, r_addr, r_port);
        if (tun->open()) {
            hc_module_instance_handle_t hdl = new_instance_cb(tun, mod_handle, &tech, tun_mtu);
            tun->set_handle(hdl);
        }
    }
}

int hc_join(hc_module_instance_t instance,
            const hc_uri_t* u, const char*)
{
    HC_LOG_TRACE("Try to join group ...");
    return self(instance)->join (*u);
}

int hc_leave(hc_module_instance_t instance,
             const hc_uri_t* u, const char*)
{
    HC_LOG_TRACE("Try to leave group ...");
    return self(instance)->leave (*u);
}

int hc_sendto(hc_module_instance_t instance,
              const void* buf, int slen, unsigned char ttl,
              const hc_uri_t* u, const char*)
{
    HC_LOG_TRACE("");
    return self(instance)->send (*u, buf, slen, ttl);
}

void hc_delete_instance(hc_module_instance_t instance)
{
    HC_LOG_TRACE("Delete tunnel instance ...");
    delete self(instance);
}

void hc_shutdown ()
{
    listen_thread.join ();
    HC_LOG_TRACE("DIE, DIE, DIE my darling ...");
}

hc_uri_result_t hc_map(hc_module_instance_t instance,
                       const hc_uri_t* u, const char*)
{
    HC_LOG_TRACE("");
    return create_uri_result(self(instance)->map (*u));
}

hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance)
{
    HC_LOG_TRACE("");
    std::vector<uri> result;
    self(instance)->neighbor_set(result);
    return create_uri_list(result);
}

hc_uri_list_t hc_group_set(hc_module_instance_t instance)
{
    HC_LOG_TRACE("");
    std::vector<pair<uri, int> > result;
    self(instance)->group_set(result);
    return create_uri_list(result);
}

hc_uri_list_t hc_children_set(hc_module_instance_t instance,
                              const hc_uri_t* u, const char*)
{
    HC_LOG_TRACE("");
    std::vector<uri> result;
    self(instance)->children_set(result, *u);
    return create_uri_list(result);
}

hc_uri_list_t hc_parent_set(hc_module_instance_t instance,
                            const hc_uri_t* u, const char*)
{
    HC_LOG_TRACE("");
    std::vector<uri> result;
    self(instance)->parent_set(result, *u);
    return create_uri_list(result);
}

int hc_designated_host(hc_module_instance_t instance,
                       const hc_uri_t* u, const char*)
{
    return self(instance)->designated_host(*u);
}
