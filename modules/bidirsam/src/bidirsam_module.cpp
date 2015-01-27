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

#include <vector>
#include <cstddef>
#include <sstream>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>

#include <boost/thread.hpp>
#include <boost/preprocessor/stringize.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

#include "chimera/chimera.h"
#include "bidirsam.hpp"
#include "bidirsam_instance.hpp"

using std::vector;
using std::string;
using std::pair;
using hamcast::uri;

namespace {
    bidirsam_instance *m_bidirsam;
}

inline bidirsam_instance* self(hc_module_instance_t instance)
{
    return reinterpret_cast<bidirsam_instance*>(instance);
}

void bidirsam_deliver_handler (Key *, Message *msg)
{
    HC_LOG_TRACE("");
    switch (msg->type) {
    case BIDIRSAM_LEAVE_MESSAGE:
        m_bidirsam->join_leave_processing(msg);
        break;
    case BIDIRSAM_JOIN_MESSAGE:
        m_bidirsam->join_leave_processing(msg);
        break;
    case BIDIRSAM_MULTICAST_MESSAGE:
        m_bidirsam->deliver_multicast_data(msg);
        break;
    default:
        break;
    }
}

extern "C" void hc_init(hc_log_fun_t log_fun,
                        struct hc_module_handle* mod_handle,
                        hc_new_instance_callback_t new_instance_cb,
                        hc_recv_callback_t recv_cb,
                        hc_event_callback_t event_cb,
                        hc_atomic_msg_size_callback_t msg_size_cb,// msg_size_cb,
                        size_t,// msg_size,
                        hc_kvp_list_t* kvp_list)
{
    hc_set_log_fun(log_fun);
    int my_port = BIDIRSAM_DEFAULT_PORT;
    int bs_port = BIDIRSAM_DEFAULT_PORT;
    ChimeraHost* bs_host = NULL;
    string bs_addr;
    string local_addr;
    hc_kvp_list_t* kvp = kvp_list;
    bool maintenance = false;
    HC_LOG_INFO("Start BIDIR-SAM module");
    /* extract config infos from kvp_list */
    while (kvp) {
        string key (kvp->key);
        string value (kvp->value);
        //HC_LOG_DEBUG ("KEY: " << key << ", VAL: " << value);
        if ((key == "port") || (key == "local.port")) {
            std::stringstream sstr (value);
            sstr >> my_port;
        }
        else if (key == "local.addr") {
            local_addr = string (value);
        }
        else if ((key == "bsport") || (key == "bootstrap.port")) {
            std::stringstream sstr (value);
            sstr >> bs_port;
        }
        else if ((key == "bsnode") || (key == "bootstrap.addr")) {
            bs_addr = string (value);
        }
        else if ((key == "maintenance") &&
                 ((value == "true") || (value == "1") || (value == "on"))) {
                    maintenance = true;
        }
        kvp = kvp->next;
    }
    HC_LOG_DEBUG (" - Local port: " << my_port);
    HC_LOG_DEBUG (" - Bootstrap port: " << bs_port);

    /* init chimera overlay */
    m_bidirsam = new bidirsam_instance(event_cb, recv_cb, msg_size_cb, maintenance);
    /* if local_addr is set, then it overrides interface address
     * might be useful for NATs, i.e. set local_addr to public IP
     */
    if (local_addr.length () > 0) {
        m_bidirsam->set_chimera_state (chimera_init (local_addr.c_str (), my_port));
    }
    else {
        m_bidirsam->set_chimera_state (chimera_init (NULL, my_port));
    }

    if (m_bidirsam->get_chimera_state() == NULL) {
        HC_LOG_FATAL ("Unable to initialize CHIMERA overlay!");
        return;
        // FIXME: should break here or throw exception
    }

    if (!bs_port) {
        HC_LOG_DEBUG (" - Port of bootstrap node not set, use default.");
        bs_port = BIDIRSAM_DEFAULT_PORT;
    }
    if (!bs_addr.empty ()) {
        bs_host = host_get (m_bidirsam->get_chimera_state(),
                            const_cast<char*>(bs_addr.c_str()), bs_port);
        HC_LOG_INFO (" - Found Bootstrap node: " << bs_host->name <<
                     "; addr: " << ntohl(bs_host->address) <<
                     "; port: " << bs_host->port <<
                     "; hash: " << key_to_string(bs_host->key));
    } else {
        HC_LOG_DEBUG (" - Bootstrap node not set, init new overlay.");
    }


    // default: acking disabled; the chimera guys defined false=2, true=1
    int acking = 2;
    HC_LOG_DEBUG (" - ACKs disabled for all BIDIRSAM messages.");

    HC_LOG_DEBUG(" - Register BIDIRSAM messages in CHIMERA.");
    // register BIDIRSAM standard message types
    chimera_register (m_bidirsam->get_chimera_state(), BIDIRSAM_JOIN_MESSAGE, acking);
    chimera_register (m_bidirsam->get_chimera_state(), BIDIRSAM_LEAVE_MESSAGE, acking);
    chimera_register (m_bidirsam->get_chimera_state(), BIDIRSAM_MULTICAST_MESSAGE, acking);
    // register upcalls
    HC_LOG_DEBUG(" - Register bidirsam upcalls in CHIMERA.");
    chimera_deliver (m_bidirsam->get_chimera_state(), bidirsam_deliver_handler);

    HC_LOG_DEBUG (" - Try to join CHIMERA overlay.");
    chimera_join (m_bidirsam->get_chimera_state(), bs_host);
    // build kvp list
    string name_key ("if_name");
    string name_val ("bidirsam");
    string addr_key ("if_addr");
    string addr_val ("bidirsam://" + m_bidirsam->address());
    string tech_key ("if_tech");
    string tech_val ("ALM");
    HC_LOG_INFO("HASH: " << m_bidirsam->address ());

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
    hc_module_instance_handle_t hdl = new_instance_cb(m_bidirsam, mod_handle,
                                                      &tech, STD_MTU_SIZE);
    m_bidirsam->set_handle(hdl);
    HC_LOG_DEBUG(" - maintenance : " << maintenance);
    m_bidirsam->start_maintenance();
    Key k =m_bidirsam->get_key();
    HC_LOG_DEBUG(" - own_key : " << key_to_string(k));
}


extern "C" int hc_join(hc_module_instance_t instance,
                       const hc_uri_t* group_uri,
                       const char*)
{
    return self(instance)->join(*group_uri);
}

extern "C" int hc_leave(hc_module_instance_t instance,
                        const hc_uri_t* group_uri,
                        const char*)
{
    return self(instance)->leave(*group_uri);
}

extern "C" int hc_sendto(hc_module_instance_t instance,
                         const void* buf,
                         int slen,
                         unsigned char ttl,
                         const hc_uri_t* group_uri,
                         const char*)
{
    return self(instance)->send(*group_uri,buf,slen,ttl);
}

extern "C" void hc_delete_instance(hc_module_instance_t instance)
{
    delete self(instance);
}

extern "C" void hc_shutdown()
{
}

extern "C" hc_uri_result_t hc_map(hc_module_instance_t instance,
                                  const hc_uri_t* group_uri,
                                  const char*)
{
    return hamcast::create_uri_result(self(instance)->map(*group_uri));
}

extern "C" hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance)
{
    vector<uri> result;
    self(instance)->neighbor_set(result);
    return create_uri_list(result);
}

extern "C" hc_uri_list_t hc_group_set(hc_module_instance_t instance)
{
    vector<pair<uri, int> > result;
    self(instance)->group_set(result);
    return create_uri_list(result);
}

extern "C" hc_uri_list_t hc_children_set(hc_module_instance_t instance,
                                         const hc_uri_t* arg0,
                                         const char*)
{
    vector<uri> result;
    self(instance)->children_set(result, *arg0);
    return create_uri_list(result);
}

extern "C" hc_uri_list_t hc_parent_set(hc_module_instance_t instance,
                                       const hc_uri_t* arg0,
                                       const char*)
{
    vector<uri> result;
    self(instance)->parent_set(result, *arg0);
    return create_uri_list(result);
}

extern "C" int hc_designated_host(hc_module_instance_t instance,
                                  const hc_uri_t* arg0,
                                  const char*)
{
    return self(instance)->designated_host(*arg0);
}
