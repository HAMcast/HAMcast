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
#include <iostream>
#include <signal.h>
#include <string.h>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <boost/thread/thread.hpp>
#include "boost/scoped_array.hpp"


#include "hamcast/hamcast_logging.h"

#include "hcDiscovery/service_discovery.hpp"
#include "ip_module.hpp"
#include "ipmodule_exceptions.hpp"
#include "ip_utils.hpp"
#include "native_socket.hpp"
#include "utils.hpp"

using boost::shared_ptr;
using boost::thread;
using std::exception;
using std::pair;
using std::string;

using namespace ip_module;

extern "C" void hc_init(hc_log_fun_t log_fun,
                        struct hc_module_handle* mod_handle,
                        hc_new_instance_callback_t new_instance_cb,
                        hc_recv_callback_t recv_cb,
                        hc_event_callback_t event_cb,
                        hc_kvp_list_t* kvp_list)
{
    hc_set_log_fun(log_fun);
    //hc_set_default_log_fun(HC_LOG_TRACE_LVL);
    HC_LOG_TRACE("");
    m_recv_callback = recv_cb;


    //Service Discovery Stuff:
    string name_key = "if_name";
#ifdef IPV6
    string name_val = "IPv6";
#else
    string name_val = "IPv4";
#endif
    
    string addr_key = "if_addr";
    string addr_val = "ip://";

    char hostname[40];
    if(gethostname(hostname, sizeof(hostname)) != -1)
    {
       addr_val += hostname; 
    }
    else
    {
        addr_val += "localhost";
    }


    //service discovery
    /* Documentation:
    for(unsigned int i=0; i< vif.size();i++){
        sd_if f = vif[i]; //service discovery interface
        f.get_ifname(); //string: e.g. "eth0"
        f.get_addr_family(); //AF_INET or AF_INET6
        f.get_ifaddr(); //sockaddr_storage
        f.get_mcast_ver(); //1, 2, 3 = mld/igmp version || 0 = unknown version but protocol found  || -1 = no protocol found,
        f.get_parent(); //sockaddr_storage (querier)
    }
    */

#ifdef ENABLE_SD

     //IP4, IP6, IPALL
     //OFF, LIGHT, FULL
    service_discovery sd(OFF,IPALL);
    vector<sd_if> vif = sd.getSD_if(); //interface vector
    sd.testoutput_service_discovery();
    vector<sd_if>::iterator vif_it;
    for(vif_it = vif.begin();vif_it != vif.end();vif_it++){
    #ifdef IPV6
        if(vif_it->get_addr_family() == AF_INET6)
    #else
        if(vif_it->get_addr_family() == AF_INET)
    #endif
        {
            struct sockaddr_storage if_addr = vif_it->get_ifaddr();
            string ip = sockaddr_to_numeric_string(reinterpret_cast<struct sockaddr*>(&if_addr), sizeof(struct sockaddr_storage)).first;
            module_data* data = new module_data;
            data->iface_ip = ip;

            hc_kvp_list_t name;
            name.key = "if_name";
            name.value = vif_it->get_ifname().c_str();
            name.next = 0;

            hc_kvp_list_t addr;
            addr.key = "if_addr";
            addr.value = data->iface_ip.c_str();
            addr.next = &name;

            hc_kvp_list_t tech;
            tech.key = "if_tech";
    #ifdef IPV6
            tech.value = "IPv6";
    #else
            tech.value = "IPv4";
    #endif
            tech.next = &addr;

            hc_module_instance_handle_t hdl = new_instance_cb (data, mod_handle, &tech);
        }
    }
#else
    module_data* data = new module_data;
    data->iface_ip = "ip://0.0.0.0";
    
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;
    
    getifaddrs(&ifAddrStruct);
    
    /* FIXME: this is crap, but we need it, thus its useful */
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            string checkaddr (addressBuffer);
            string checkname (ifa->ifa_name);
            if ((checkaddr.find("127.0.0.1",0)== string::npos) && (checkname.find("eth",0)!= string::npos)) {
                data->iface_ip = string("ip://");
                data->iface_ip += string (addressBuffer);
                break;
            }
        } else if (ifa->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        } 
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

            hc_kvp_list_t name;
            name.key = "if_name";
            name.value = "IP";
            name.next = NULL;

            hc_kvp_list_t addr;
            addr.key = "if_addr";
            addr.value = data->iface_ip.c_str();
            addr.next = &name;

            hc_kvp_list_t tech;
            tech.key = "if_tech";
    #ifdef IPV6
            tech.value = "IPv6";
    #else
            tech.value = "IPv4";
    #endif
            tech.next = &addr;
            hc_module_instance_handle_t hdl = new_instance_cb (data, mod_handle, &tech);
#endif
            data->m_mihdl = hdl;
}

void hc_shutdown()
{
    HC_LOG_TRACE("");
}


void hc_delete_instance(hc_module_instance_t instance)
{
    HC_LOG_TRACE("");
    module_data* data = cast(instance);
    map<hc_uri_t, shared_ptr<native_socket> >* m_sockets = &(data->m_sockets);

    map<hc_uri_t, shared_ptr<native_socket> >::iterator it;
    for(it = m_sockets->begin(); it != m_sockets->end();it++)
    {
        //leave all groups
        hc_leave(instance, &(it->first), "");
        //destroy all m_sockets (incl. native_socket)
        {
            boost::mutex::scoped_lock guard (data->m_sockets_mtx);
            m_sockets->erase(it);
        }
    }
    delete(data);
}

hc_uri_list_t hc_group_set(hc_module_instance_t instance){
    HC_LOG_TRACE("");
    module_data* data = cast(instance);
    map<hc_uri_t, shared_ptr<native_socket> >* m_sockets = &(data->m_sockets);

    hc_uri_list_t first;
    first.uri_str = NULL;
    first.uri_obj = NULL;
    first.type = HC_IGNORED;
    first.next = NULL;

    if(m_sockets->empty())
        return first;

    hc_uri_list_t* cur = &first;
    map<hc_uri_t, shared_ptr<native_socket> >::const_iterator it;
    bool first_run = true;
    for(it = m_sockets->begin(); it != m_sockets->end();it++)
    {
        if(!first_run)
        {
            hc_uri_list_t* prev = cur;
            cur = new hc_uri_list_t;

            prev->next = cur; 
        }
        else
        {
            first_run=false;
        }

        hc_uri_t* uri = new hc_uri_t(it->first);
        cur->uri_obj = uri;
        cur->uri_str = NULL;
    }
    cur->next = NULL;
    return first;
}

hc_uri_list_t hc_children_set(hc_module_instance_t instance_ptr, const hc_uri_t*, const char*){
    HC_LOG_TRACE("CHILDREN_SET");
    hc_uri_list_t res;
    res.uri_str = NULL;
    res.uri_obj = NULL;
    res.type    = HC_IGNORED;
    res.next    = NULL;
    return res;
}


//soll IGMPquerier returnen, trac ticket #29
hc_uri_list_t hc_parent_set(hc_module_instance_t instance_ptr, const hc_uri_t*, const char*){
    HC_LOG_TRACE("PARENT_SET");
    hc_uri_list_t res;
    res.uri_str = NULL;
    res.uri_obj = NULL;
    res.type    = HC_IGNORED;
    res.next    = NULL;
    return res;
}

int hc_designated_host(hc_module_instance_t, const hc_uri_t*, const char*){
    return HC_IS_NOT_A_DESIGNATED_HOST;
}

//TODO: returns empty list is host isnt a multicast router, else all routing neighbours
hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance_ptr)
{
    HC_LOG_TRACE("NEIGHBOR_SET");
    hc_uri_list_t res;
    res.uri_str = NULL;
    res.uri_obj = NULL;
    res.type    = HC_IGNORED;
    res.next    = NULL;
    return res;
}


hc_uri_result_t hc_map(hc_module_instance_t instance_ptr, const hc_uri_t* group_uri, const char* uri_str)
{
    HC_LOG_TRACE("");

    hc_uri_t* uri = new hc_uri_t(*group_uri);
    hc_uri_result_t result;
    result.uri_str = NULL;
    result.uri_obj = NULL;

    if(!group_uri->empty())
    {
        try
        {
            vector<pair<string, string> > grp = numeric_resolve(group_uri->host(), group_uri->port());
            if(!grp.empty() && !grp.front().first.empty() && !grp.front().second.empty())
            {
                stringstream result_str;
                result_str<<"ip://";

                //prepend Source host if present
                if(!group_uri->user_information().empty())
                {
                    vector<pair<string, string> > src = numeric_resolve(group_uri->host(), group_uri->port());
                    if(!src.empty() && !src.front().first.empty())
                    {
                        result_str<<src.front().first << "@";
                    }
                }

#ifdef IPV6
                result_str << "[" << grp.front().first <<"]" << ":" << grp.front().second;
#else
                result_str << grp.front().first << ":" << grp.front().second;
#endif
                hc_uri_t* uri = new hc_uri_t(result_str.str());
            }
        }
        catch(ipmodule_exception& e)
        {
            HC_LOG_ERROR("ip_module exception: " <<  e.what());
        }
        catch(exception &e)
        {
            HC_LOG_ERROR("unknown exception: " << e.what());
        }
    }

    if(is_addr_valid(*uri))
    {
        result.uri_obj = uri;
    }
    else
    {
        HC_LOG_ERROR("invalid URL:" +  group_uri->str());
    }

    return result;
}


int hc_join(hc_module_instance_t instance_ptr, const hc_uri_t* group_uri, const char* group_uri_str)
{
    HC_LOG_TRACE("");
    module_data* data = cast(instance_ptr);
    map<hc_uri_t, shared_ptr<native_socket> >* m_sockets = &(data->m_sockets);

    if(!is_addr_valid(*group_uri)) {
        return HC_INVALID_URI;
    }

    try {

        //check if socket is allready subscribed to this group
        if(is_subscribed(m_sockets, *group_uri)) {
            //return successfull if socket is allready subscribed
            HC_LOG_INFO("socket is allready subscribed to:" + group_uri->str());
        } else {
            //create new native socket & join group
            HC_LOG_DEBUG("data->iface_ip: "+data->iface_ip);
            shared_ptr<native_socket> s(new native_socket(data->iface_ip));
            struct sockaddr_storage group_addr;
            //if user_information was supplied make an ASM join, else SSM
            if(group_uri->user_information().empty()) {
                uri_to_sockaddr_storage(*group_uri, &group_addr, NULL);
                s->join(&(group_addr));
            } else {
                struct sockaddr_storage src_addr;
                uri_to_sockaddr_storage(*group_uri, &group_addr, &src_addr);
                ///@TODO überarbeiten so das uri_to_sockaddr_storage nur den sin_addr teil zurückgibt der auch benötigt wird
                s->join(&(group_addr), &(src_addr));
            }

            s->bind(group_uri->port_as_int());
            HC_LOG_INFO("successfull subscribed to group: " + group_uri->str());

            //insert (uri, socket) into m_sockets
            {
                boost::mutex::scoped_lock guard (data->m_sockets_mtx);
                (*m_sockets)[*group_uri] = s;
            }
        }

        //start recv thread if it isn't running
        if(data->recv_thrd.get_id() == thread::id()) {
            data->recv_thrd = thread(&ip_module::recv_thread, instance_ptr);
        } else { //notify recv_thread that we have a new socket to watch
            pthread_kill(data->recv_thrd.native_handle(), SIGUSR1);
        }
    }
    catch(ipmodule_exception& e) {
        HC_LOG_ERROR("ip_module exception; cannot join \"" << group_uri->str() << "\": " <<  e.what());
        return HC_UNKNOWN_ERROR;
    }
    catch(exception &e) {
        HC_LOG_ERROR("unknown exception; cannot join \"" << group_uri->str() << "\": " << e.what());
        return HC_UNKNOWN_ERROR;
    }
    
    return HC_SUCCESS;
}

int hc_leave(hc_module_instance_t instance_ptr, const hc_uri_t* group_uri, const char* group_uri_str)
{
    HC_LOG_TRACE("");

    module_data* data = cast(instance_ptr);
    map<hc_uri_t, shared_ptr<native_socket> >* m_sockets = &(data->m_sockets);

    if(!is_addr_valid(*group_uri)) {
        return HC_INVALID_URI;
    }

    try
    {
        map<hc_uri_t, shared_ptr<native_socket> >::iterator it = m_sockets->find(*group_uri);
        if(it == m_sockets->end()) {
            HC_LOG_INFO("unsubscribe impossible, not subscribed to group: " + group_uri->str());
            return HC_UNKNOWN_ERROR;
        }

        //shared_ptr<native_socket> socket(it->second); // native socket that is subscribed to the group
        shared_ptr<native_socket> socket = it->second;
        struct sockaddr_storage group_addr;
        if(group_uri->user_information().empty()) {
            uri_to_sockaddr_storage(*group_uri, &group_addr, NULL);
            socket->leave(&group_addr);
        } else {
            struct sockaddr_storage src_addr;
            uri_to_sockaddr_storage(*group_uri, &group_addr, &src_addr);
            ///@TODO überarbeiten so das uri_to_sockaddr_storage nur den sin_addr teil zurückgibt der auch benötigt wird
            socket->leave(&(group_addr), &(src_addr));
        }

        //delete socket from m_sockets
        {
            boost::mutex::scoped_lock guard (data->m_sockets_mtx);
            m_sockets->erase(*group_uri);
        }
        // notify thread, list of watched recv sockets has changed!
        pthread_kill(data->recv_thrd.native_handle(), SIGUSR1);
        HC_LOG_INFO("unsubscribed group: " + group_uri->str());

        return HC_SUCCESS;

    }
    catch(ipmodule_exception& e) {
        HC_LOG_ERROR(string("ip_module exception: ") + string(e.what()));
        return HC_UNKNOWN_ERROR;
    }
    catch(exception &e) {
        HC_LOG_ERROR(string("unknown exception: ") + string(e.what()));
        return HC_UNKNOWN_ERROR;
    }
}

int hc_sendto(hc_module_instance_t instance_ptr, const void* buf, int len, unsigned char ttl, const hc_uri_t* group_uri, const char* group_uri_str)
{
    HC_LOG_TRACE("");

    if(!is_addr_valid(*group_uri)){
        return HC_INVALID_URI;
    }

    //set TTL
#ifdef IPV6
    if(ttl == 0)
    {
        HC_LOG_ERROR("invalid TTL value: " << ttl);
        return HC_UNKNOWN_ERROR; 
    }
#else
    if(ttl == 0)
    {
        HC_LOG_ERROR("invalid TTL value: " << ttl);
        return HC_UNKNOWN_ERROR; 
    }
#endif
    else
    {
        if(m_send_socket.get_multicast_ttl() != ttl)
        {
            m_send_socket.set_multicast_ttl(ttl);
            HC_LOG_DEBUG("TTL set to. "<<ttl);
        }

    }

    try
    {
        struct sockaddr_storage addr;
        uri_to_sockaddr_storage(*group_uri, &addr, NULL);
        return m_send_socket.sendto(&addr, buf, len);
        /*
           //funktioniert so nicht, da der sending socket da auch im pselect abgefragt wird..
        //create new socket for sending if no subscribe happened before + no native socket exists
        if(m_socket->empty)
        {
            m_socket->insert(pair<hc_uri_t, shared_ptr<native_socket>>)(hamcast::uri("ip://send_socket", shared_ptr<native_socket>()));
            m_socket_ver++;
        }

        struct sockaddr_storage addr;
        uri_to_sockaddr_storage(group_uri, &addr);
        //use first socket in map for sending, doesn't matter which native_socket we use
        return m_socket->begin->second.send(&addr, buf, len);
        */
    }

    catch(ipmodule_exception &e)
    {
        HC_LOG_ERROR(string("ip_module exception: ") + string(e.what()));
        return HC_UNKNOWN_ERROR;
    }
    catch(exception &e)
    {
        HC_LOG_ERROR(string("unknown exception: ") + string(e.what()));
        return HC_UNKNOWN_ERROR;
    }
}

namespace ip_module{

bool is_addr_valid(const hc_uri_t& uri)
{
    HC_LOG_TRACE("");
#ifndef IPV6
    if((uri.empty()) || (!uri.host_is_ipv4addr()) || (uri.port_as_int() == 0) || (uri.scheme() != "ip")
      || (uri.host() == "0.0.0.0") || (!uri.user_information().empty() && uri.user_information() == "0.0.0.0"))
#else
    if((uri.empty()) || (!uri.host_is_ipv6addr()) || (uri.port_as_int() == 0) || (uri.scheme() != "ip")
      || (uri.host() == "::") || (!uri.user_information().empty() && uri.user_information() == "::"))
#endif
    {
        return false;
    }
    return true;
}

bool is_subscribed(const map<hc_uri_t, shared_ptr<native_socket> >* m_sockets, const hc_uri_t& uri)
{
    HC_LOG_TRACE("");

    map<hc_uri_t, shared_ptr<native_socket> >::const_iterator it;
    it = m_sockets->find(uri);
    if(it != m_sockets->end() && it->second != NULL)
        return true;
    return false;
}

//signal handler that will be called to cancel waiting if new socket was
//added
inline void useless_signal_handler(int x){
    return;
}

void recv_thread(hc_module_instance_t instance_ptr)
{
    /* Rewritten to use select instead of epoll, by smeiling 110508 */
    HC_LOG_TRACE("");
    HC_LOG_DEBUG("new recv thread started, pid: " << getpid());
    signal(SIGUSR1, useless_signal_handler); //dont terminate on SIGUSR1, we use it in recv_thread(...)

    module_data* data = cast(instance_ptr);
    map<hc_uri_t, shared_ptr<native_socket> >* m_sockets = &(data->m_sockets);
    map<int, hc_uri_t> uri_map;
    hc_module_instance_handle_t mhdl = data->m_mihdl;

    const int recv_buf_len = IPMODULE_RBUFLEN;
    char recv_buf[recv_buf_len];

    fd_set rset_org, rset;
    int maxfd;
    bool init = true;
    int size = 0;
    //specifies max. number on recv() call on an socket that has new data arrived till next epoll_pwait round
    const int maxreads = IPMODULE_MAXREADS; 

    // main receive loop for all sockets
    while(!m_sockets->empty()) {
        try {
            // init select for all watched sockets
            if (init) {
                HC_LOG_DEBUG ("Run init for select.");
                FD_ZERO(&rset_org);
                maxfd = 0;
                map<hc_uri_t, shared_ptr<native_socket> >::iterator it;
                for(it = m_sockets->begin(); it != m_sockets->end();it++)
                {
                    int sockfd = it->second->get_socketfd();
                    uri_map.insert(pair<int, hc_uri_t>(sockfd,it->first));
                    FD_SET(sockfd, &rset_org);
                    if (sockfd > maxfd)
                        maxfd = sockfd;
                }
                HC_LOG_DEBUG ("Max sock id: " << maxfd << "." );
                init = false;
            }
            //rset = rset_org;
            memcpy(&rset, &rset_org, sizeof(rset_org));
            // do select, check for sockets ready to receive
            int select_status = select (maxfd+1, &rset, NULL,NULL,NULL);

            // if error or interupt rerun init, otherwise do recv on 
            // ready sockets
            if (select_status < 0) {
                HC_LOG_DEBUG ("Error or interupt during select().");
                init = true;
            } else {
                // loop through all sockets
                for(int i=0; i < maxfd+1; ++i) {
                    // check if and which socket is ready to recv
                    if (FD_ISSET(i, &rset)) {
                        // do maxreads x recv 
                        for(int j=0; j < maxreads; ++j)
                        {
                            hc_uri_t *uri = &(uri_map.find(i)->second);
                            int ret = 0;
                            // push data to middleware if recv ok,
                            // otherwise stop receiving
                            if(((*m_sockets)[*uri] != NULL)&&((ret = (*m_sockets)[*uri]->recv_async(recv_buf,recv_buf_len)) > 0)){ 
                                //socket returned with new data
                                m_recv_callback(mhdl, recv_buf, ret, uri, NULL);
                            } else {
                                j = maxreads;
                            }
                        }
                    }
                }
            }
        }
        catch(ipmodule_exception &e)
        {
            HC_LOG_ERROR(string("ip_module exception: ") + string(e.what()));
        }
        catch(exception &e)
        {
            HC_LOG_ERROR(string("unknown exception: ") + string(e.what()));
        }
    }

    data->recv_thrd = thread();
    HC_LOG_DEBUG("recv thread terminating");
}

}
