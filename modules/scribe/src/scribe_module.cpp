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

#include <cstddef>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>

#include <arpa/inet.h>

#include <boost/lexical_cast.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/thread.hpp>

#include "hamcast/exception.hpp"
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_logging.h"
#include "hamcast/hamcast_module.h"

#include "chimera/chimera.h"
#include "chimera/host.h"
#include "chimera/key.h"
#include "chimera/route.h"

#include "scribe.hpp"
#include "scribe_instance.hpp"

using namespace scribe;
using hamcast::uri;
using std::string;

namespace{
    scribe_instance *m_scribe;
}

inline scribe_instance* self(hc_module_instance_t instance)
{
    return reinterpret_cast<scribe_instance*>(instance);
}

    void scribe_forward_handler (Key **key, Message **msg,
                                                  ChimeraHost **nextHost)
    {
        HC_LOG_TRACE("");
        //Key *fkey = *key;
        //Message *fmsg = *msg;
        //ChimeraHost *fnextHost = *nextHost;
        if ((*msg)->type == SCRIBE_MSG_JOIN) {
            HC_LOG_DEBUG ("Forward Join ...");
            m_scribe->forward_join (*key, *msg, *nextHost);
        }
    }

    void scribe_deliver_handler (Key *key, Message *msg)
    {
        HC_LOG_TRACE("");
        switch (msg->type) {
            case SCRIBE_MSG_CREATE:
                m_scribe->deliver_create (key, msg);
                break;
            case SCRIBE_MSG_JOIN:
                m_scribe->deliver_join (key, msg);
                break;
            case SCRIBE_MSG_LEAVE:
                m_scribe->deliver_leave (key, msg);
                break;
            case SCRIBE_MSG_MULTICAST:
                m_scribe->deliver_multicast (key, msg);
                break;
            case SCRIBE_MSG_RP_REQUEST:
                m_scribe->deliver_rp_request (key, msg);
                break;
            case SCRIBE_MSG_RP_REPLY:
                m_scribe->deliver_rp_reply (key, msg);
                break;
            case SCRIBE_MSG_PARENT:
                m_scribe->deliver_parent (key, msg);
            default:
                // there are more message types, print log here
                break;
        }
    }

extern "C" void hc_init(hc_log_fun_t log_fun,
                        struct hc_module_handle* mod_handle,
                        hc_new_instance_callback_t new_instance_cb,
                        hc_recv_callback_t recv_cb,
                        hc_event_callback_t event_cb,
                        hc_atomic_msg_size_callback_t msg_size_cb,
                        size_t,// msg_size,
                        hc_kvp_list_t* kvp_list)
{
    hc_set_log_fun(log_fun);
    int bootstrap_port = SCRIBE_DEFAULT_PORT;
    int local_port = SCRIBE_DEFAULT_PORT;
    string bootstrap_addr;
    string local_addr;
    bool mack = false;
    bool mnt = false;
    hc_kvp_list_t* kvp = kvp_list;
    /* extract config infos from kvp_list */
    while (kvp) {
        string key (kvp->key);
        string value (kvp->value);
        HC_LOG_DEBUG ("KEY: " << key << ", VAL: " << value);
        if ((key == "port") || (key == "local.port")) {
            std::stringstream sstr (value);
            sstr >> local_port;
        }
        else if (key == "local.addr") {
            local_addr = string (value);
        }
        else if ((key == "bsport") || (key == "bootstrap.port")) {
            std::stringstream sstr (value);
            sstr >> bootstrap_port;
        } 
        else if ((key == "bsnode") || (key == "bootstrap.addr")) {
            bootstrap_addr = string (value);
        } 
        else if ((key == "reliable") &&
                 ((value == "true") || (value == "1") || (value == "on"))) {
            mack = true;
        }
        else if ((key == "maintenance") &&
                 ((value == "true") || (value == "1") || (value == "on"))) {
            mnt = true;
        }
        kvp = kvp->next;
    }

    /* init new scribe overlay */
    m_scribe = new scribe_instance(event_cb, recv_cb,msg_size_cb, mnt, false);

    /* if local_addr is set, then it overrides interface address
     * might be useful for NATs, i.e. set local_addr to public IP
     */
    if (local_addr.length () > 0) {
        m_scribe->set_state (chimera_init (local_addr.c_str (), local_port));
    }
    else {
        m_scribe->set_state (chimera_init (NULL, local_port));
    }

    if (m_scribe->get_state() == NULL) {
        HC_LOG_FATAL ("Unable to initialize CHIMERA overlay!");
        return;
        // FIXME: should break here or throw exception
    }

    ChimeraHost* bs_host = NULL;
    if (!bootstrap_addr.empty()) {
        bs_host = host_get (m_scribe->get_state(),
                            const_cast<char*>(bootstrap_addr.c_str()),
                            bootstrap_port);
        HC_LOG_INFO ("Found Bootstrap node: " << bs_host->name <<
                      "; addr: " << ntohl(bs_host->address) <<
                      "; port: " << bs_host->port <<
                      "; hash: " << key_to_string(bs_host->key));
    } else {
        HC_LOG_DEBUG ("Bootstrap node not set, init new overlay.");
    }

    // default: acking disabled; the chimera guys defined false=2, true=1
    int acking = 2;

    // enable acking for multicast messages
    if(mack) {
        HC_LOG_INFO ("ACKs enabled for all Scribe messages.");
        acking = 1;
    } else {
        HC_LOG_INFO ("ACKs disabled for all Scribe messages.");
    }

    HC_LOG_DEBUG ("Register Scribe messages in CHIMERA.");
    // register scribe standard message types
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_UNKNOWN, acking);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_CREATE, 1);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_JOIN, acking);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_LEAVE, acking);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_MULTICAST, acking);

    // register scribe control message types
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_PING, acking);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_HEARTBEAT, acking);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_RP_REQUEST, 1);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_RP_REPLY, 1);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_REPLICATE, 1);
    chimera_register (m_scribe->get_state(), SCRIBE_MSG_PARENT, 1);

    // register upcalls
    HC_LOG_DEBUG("Register Scibe upcalls in CHIMERA.");
    chimera_forward (m_scribe->get_state(), scribe_forward_handler);
    chimera_deliver (m_scribe->get_state(), scribe_deliver_handler);

    HC_LOG_DEBUG ("Try to join CHIMERA overlay.");
    chimera_join (m_scribe->get_state(), bs_host);

    // build kvp list
    string name_key ("if_name");
    string name_val ("scribe");
    string addr_key ("if_addr");
    string addr_val ("scribe://" + m_scribe->get_address());
    string tech_key ("if_tech");
    string tech_val ("ALM");
    HC_LOG_INFO("HASH: " << m_scribe->get_address ());

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
    /* here were return */
    hc_module_instance_handle_t hdl = new_instance_cb(m_scribe, mod_handle,
                                                      &tech, SCRIBE_DEFAULT_MTU);

    m_scribe->do_maintenance();
    m_scribe->set_handle(hdl);
}

extern "C" int hc_join(hc_module_instance_t instance,
            const hc_uri_t* u, const char*)
{
    return self(instance)->join (*u);
}

extern "C" int hc_leave(hc_module_instance_t instance,
             const hc_uri_t* u, const char*)
{
    return self(instance)->leave (*u);
}

extern "C" int hc_sendto(hc_module_instance_t instance,
              const void* buf, int slen, unsigned char ttl,
              const hc_uri_t* u, const char*)
{
    return self(instance)->send (*u, buf, slen, ttl);
}

extern "C" void hc_delete_instance(hc_module_instance_t instance)
{
    delete self(instance);
}

extern "C" void hc_shutdown ()
{
    HC_LOG_TRACE("DIE, DIE, DIE my darling ...");
}

extern "C" hc_uri_result_t hc_map(hc_module_instance_t instance,
                       const hc_uri_t* u, const char*)
{
    return create_uri_result(self(instance)->map (*u));
}

extern "C" hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance)
{
    std::vector<hamcast::uri> result;
    self(instance)->neighbor_set(result);
    return create_uri_list(result);
}

extern "C" hc_uri_list_t hc_group_set(hc_module_instance_t instance)
{
    std::vector<std::pair<hamcast::uri, int> > result;
    self(instance)->group_set(result);
    return create_uri_list(result);
}

extern "C" hc_uri_list_t hc_children_set(hc_module_instance_t instance,
                              const hc_uri_t* u, const char*)
{
    std::vector<hamcast::uri> result;
    self(instance)->children_set(result, *u);
    return create_uri_list(result);
}

extern "C" hc_uri_list_t hc_parent_set(hc_module_instance_t instance,
                            const hc_uri_t* u, const char*)
{
    std::vector<hamcast::uri> result;
    self(instance)->parent_set(result, *u);
    return create_uri_list(result);
}

extern "C" int hc_designated_host(hc_module_instance_t instance,
                       const hc_uri_t* u, const char*)
{
    return self(instance)->designated_host(*u);
}
