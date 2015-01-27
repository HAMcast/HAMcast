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

#include <list>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "hamcast/exception.hpp"
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_logging.h"
#include "hamcast/hamcast_module.h"
#include "hamcast/uri.hpp"

#include "chimera/chimera.h"
#include "chimera/host.h"
#include "chimera/key.h"
#include "chimera/route.h"

#include "scribe.hpp"
#include "scribe_instance.hpp"
#include "scribe_node.hpp"


using namespace scribe;
using hamcast::uri;

using std::list;
using std::set;
using std::pair;
using std::vector;
using std::string;

/*** PRIVATE STUFF ***/

Key scribe_instance::uri_mapping(const uri &group_uri)
{
    HC_LOG_TRACE("");
    Key k;
    string sch = group_uri.scheme ();
    string grp = group_uri.user_information_and_host ();
    string prt = group_uri.port ();
    uint32_t prti = 0;
    boost::mutex::scoped_lock guard (m_map_mutex);
    key_map_t::iterator it;
    if (!prt.empty ()) {
        it = m_key_cache.find(sch+"://"+grp+":"+prt);
        prti = group_uri.port_as_int ();
    }
    else {
        it = m_key_cache.find(grp);
    }

    if (it != m_key_cache.end()) {
        k = it->second;
    } else {
        key_makehash (NULL, &k, grp.c_str());
        vector<uint32_t> v (k.t, k.t + KEY_ARRAY_SIZE);
        m_key_cache.insert(pair<string, Key>(sch+"://"+grp,k));
        m_uri_cache.insert(pair<vector<uint32_t>, uri>(v,uri(sch+"://"+grp)));
        if (prti != 0) {
            v.push_back (prti);
            m_key_cache.insert(pair<string, Key>(sch+"://"+grp+":"+prt,k));
            m_uri_cache.insert(pair<vector<uint32_t>, uri>(v,uri(sch+"://"+grp+":"+prt)));
        }
    }
    return k;
}

uri scribe_instance::key_mapping (const Key &k, const uint32_t &port)
{
    HC_LOG_TRACE("");
    vector<uint32_t> kvec (k.t, k.t + KEY_ARRAY_SIZE);
    if (port > 0) {
        kvec.push_back (port);
    }

    uri_map_t::iterator it;
    {
        boost::mutex::scoped_lock guard (m_map_mutex);
        it = m_uri_cache.find (kvec);
        if (it != m_uri_cache.end ()) {
            return (it->second);
        }
    }
    if (port > 0) {
        kvec.pop_back();
        string tmp_str;
        {
            boost::mutex::scoped_lock guard (m_map_mutex);
            it = m_uri_cache.find(kvec);
            if (it != m_uri_cache.end()) {
                std::stringstream stream;
                stream << it->second.str() << ":" << port;
                tmp_str = stream.str();
            }
        }
        uri tmp_uri = uri (tmp_str);
        uri_mapping(tmp_uri);
        return tmp_uri;
    }
    return (uri());
}

void scribe_instance::maintenance ()
{
    HC_LOG_TRACE ("");
    while (m_maintenance) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(c_maintenance_timer));
        // wait done, do something
        group_map_t::iterator it;
        set<uri> j_groups;
        { // lock scope
            boost::mutex::scoped_lock guard (m_grp_mutex);
            for (it = m_groups.begin(); it != m_groups.end(); ++it) {
                children_map_t& children = it->second.get_children();
                children_map_t::iterator cit;
                set<Key> to_delete;
                for (cit = children.begin(); cit != children.end(); ++cit) {
                    if (m_mykey== cit->first) {
                        uri group_uri (it->second.get_uri());
                        if (!group_uri.empty()) {
                            j_groups.insert(group_uri);
                        }
                    } else {
                        --(cit->second);
                    }
                    if (cit->second.get_count() == 0) {
                        to_delete.insert(cit->first);
                    }
                }
                set<Key>::iterator dit;
                for (dit = to_delete.begin(); dit != to_delete.end(); ++dit) {
                    it->second.del_child(*dit);
                }
            }
        } // lock scope end
        set<uri>::iterator sit;
        for (sit = j_groups.begin(); sit!=j_groups.end(); ++sit) {
            HC_LOG_DEBUG("REJOIN (grp) " << sit->str());
            join (*sit);
        }
        vector<string> rtable;
        get_routing_table (rtable);
        for (size_t s=0; s < rtable.size (); ++s) {
            HC_LOG_DEBUG(rtable[s]);
        }
    }
}

void scribe_instance::replicate ()
{
    HC_LOG_TRACE ("");
    std::cerr << " NOT IMPLEMENTED YET! " << std::endl;
}

/*** PUBLIC STUFF ***/

void scribe_instance::do_maintenance()
{
    HC_LOG_TRACE("");
    if (m_maintenance) {
        HC_LOG_INFO ("Maintenance set active, start thread.");
        m_worker = boost::thread(&scribe_instance::maintenance, this);
    }
    else {
        HC_LOG_INFO ("Maintenance set inactive, nothing to do.");
    }
}

void scribe_instance::deliver_create (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    HC_LOG_INFO ("Deliver create for group: " << key_to_string(msg->src));
    uri group_uri = key_mapping(msg->src, msg->port); // dummy call, to establish mapping
    group_uri = key_mapping (msg->src, 0);

    if (group_uri.empty ()) {
        HC_LOG_WARN("Cache Miss: group uri unknown.");
        if (msg->size >= HOST_CODED_SIZE) {
            HC_LOG_INFO ("decode create message.");
            // msg: [ [HOST]:[GROUP] ]
            char *group_str = (msg->payload+HOST_CODED_SIZE+1);
            uri tmp_uri = uri(group_str);
            if (!tmp_uri.empty()) {
                Key groupkey = uri_mapping(tmp_uri);
                group_uri = key_mapping(groupkey, 0);
            }
            HC_LOG_INFO ("Groupstr: " << group_str);
            HC_LOG_INFO ("Groupkey: " << key_to_string(msg->src));
        }
        else {
            HC_LOG_WARN ("FAILED to decode create msg, group key: " << key_to_string(msg->src));
            return;
        }
    }
    else {
        HC_LOG_INFO("Cache Hit: group uri known.");
    }

    if (!group_uri.empty()) {
        boost::mutex::scoped_lock guard (m_grp_mutex);
        m_groups.insert(pair<Key, scribe_group>(msg->src, scribe_group(group_uri)));
        uri *event_uri = new hamcast::uri(group_uri);
        m_event_cb(m_handle, static_cast<int>(hamcast::new_source_event), event_uri, NULL);
    }
}

void scribe_instance::deliver_join (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    HC_LOG_INFO ("Deliver join for group: " << key_to_string(msg->src));
    uri group_uri = key_mapping(msg->src, msg->port); // dummy call, to establish mapping
    group_uri = key_mapping (msg->src, 0);

    if (group_uri.empty ()) {
        HC_LOG_WARN("Cache Miss: group uri unknown.");
        if (msg->size >= HOST_CODED_SIZE) {
            HC_LOG_INFO ("decode join message.");
            // msg: [ [HOST]:[GROUP] ]
            char *group_str = (msg->payload+HOST_CODED_SIZE+1);
            uri tmp_uri = uri(group_str);
            if (!tmp_uri.empty()) {
                Key group_key = uri_mapping(tmp_uri);
                group_uri = key_mapping(group_key, 0);
            }
            HC_LOG_INFO ("Groupstr: " << group_str);
            HC_LOG_INFO ("Groupkey: " << key_to_string(msg->src));
        }
        else {
            HC_LOG_WARN ("FAILED to decode join msg, group key: " << key_to_string(msg->src));
            return;
        }
    }
    else {
        HC_LOG_INFO("Cache Hit: group uri known.");
    }

    /*
     * add this node as child to local group
     * if group not exists, create and add it
     */
    group_map_t::iterator it;
    ChimeraHost *host = host_decode(m_state, msg->payload);
    { // scope for lock
        boost::mutex::scoped_lock guard (m_grp_mutex);
        it = m_groups.find (msg->src);
        if (it == m_groups.end()) {
            HC_LOG_INFO ("Group not found, create!");
            it = m_groups.insert(pair<Key, scribe_group>(msg->src, scribe_group(group_uri))).first;
        }
        scribe_group &group = it->second;
        HC_LOG_INFO(" - add host as child.");
        group.add_child(host);
    }
    ChimeraGlobal* chglo = (ChimeraGlobal*)m_state->chimera;
    if (key_equal (m_mykey, host->key)) {
        HC_LOG_INFO ("Do not send myself as parent to me!");
    }
    else {
        HC_LOG_DEBUG("(grp,child,parent) "  << group_uri << ","
                                            << key_to_string(host->key) << ","
                                            << key_to_string(m_mykey));
        char s[256];
        size_t slen = host_encode (s, sizeof(s), chglo->me);
        Message *pmsg;
        pmsg = message_create (host->key, msg->src, msg->port,
                               SCRIBE_MSG_PARENT, slen, s);
        message_send (m_state, host, pmsg, 0);
        message_free(pmsg);
    }
    if (!group_uri.empty()) {
        hamcast::uri *event_uri = new hamcast::uri(group_uri);
        m_event_cb(m_handle, static_cast<int>(hamcast::join_event), event_uri, NULL);
    }
}

void scribe_instance::deliver_leave (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    group_map_t::iterator it;
    scribe_group* group = NULL;
    { // scope for lock
        boost::mutex::scoped_lock guard (m_grp_mutex);
        it = m_groups.find (msg->src);
        if (it == m_groups.end()) {
            HC_LOG_WARN("No such group to leave!");
            return;
        }
        HC_LOG_INFO("Group found.");
        group = &(it->second);

        // delete child from list
        ChimeraHost *host = host_decode (m_state, msg->payload);
        if (host == NULL) {
            HC_LOG_WARN ("host_decode failed!");
            return;
        }
        // group found, delete child
        HC_LOG_INFO(" - del child host.");
        group->del_child (host->key);
    }
    // if no children then forward leave to parent
    if(group->get_children().size() == 0) {
        HC_LOG_INFO("No more children, leaving too!");
        char s[256];
        Message *lmsg;
        ChimeraHost *parent = group->get_parent();
        ChimeraGlobal* chglo = (ChimeraGlobal*)m_state->chimera;
        size_t slen = host_encode (s, sizeof(s), chglo->me);
        if (parent != NULL && !key_equal(m_mykey, parent->key)) {
            lmsg = message_create (parent->key, msg->src, msg->port,
                                   SCRIBE_MSG_LEAVE, slen, s);
            message_send (m_state, parent, lmsg, 0);
            message_free(lmsg);
        }
    }
}

void scribe_instance::deliver_multicast (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    /*
     * add this node as child to local group
     * if group not exists, create and add it
     */
    group_map_t::iterator it;

    // LOCK mutex
    m_grp_mutex.lock ();
        it = m_groups.find (msg->src);
        if (it == m_groups.end()) {
            HC_LOG_WARN ("No such group, cannot deliver message!");
            return;
        }
        // copy group data and release lock
        scribe_group group = it->second;
    m_grp_mutex.unlock ();
    // UNLOCK mutex
    uri group_uri = key_mapping (msg->src, msg->port);
    if( msg->size == 0) {
        HC_LOG_INFO ("No data, just heartbeat ...");
    }
    else {
        children_map_t children = group.get_children();
        children_map_t::iterator cit;
        for (cit = children.begin(); cit != children.end(); ++cit) {

            if (cit->first == m_mykey) {
                HC_LOG_DEBUG("(grp,src,rcv) " << group_uri << ","
                                              << key_to_string(msg->src) << ","
                                              << key_to_string(m_mykey));

                m_recv_cb(m_handle, msg->payload, msg->size, &group_uri, group_uri.c_str());
            }
            else {
                Key dst = cit->first;
                HC_LOG_DEBUG("(grp,src,dst) " << group_uri << ","
                                              << key_to_string(m_mykey) << ","
                                              << key_to_string(dst));
                // some other child, forward message
                Message *cmsg = message_create(dst, msg->src, msg->port,
                                               msg->type, msg->size, msg->payload);
                message_send (m_state, cit->second.get_host(), cmsg, 0);
                message_free (cmsg);
            }
        }
    }
}

void scribe_instance::deliver_rp_request (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    ChimeraGlobal* chglo = (ChimeraGlobal*)m_state->chimera;
    if (msg->size < HOST_CODED_SIZE) {
        HC_LOG_WARN ("host_decode failed, msg too small!");
        return;
    }
    ChimeraHost *host = host_decode(m_state, msg->payload);
    if (host != NULL) {
        char s[256];
        Message *rmsg;
        size_t slen = host_encode (s, sizeof(s), chglo->me);
        rmsg = message_create (host->key, msg->src, msg->port,
                               SCRIBE_MSG_RP_REPLY, slen, s);
        message_send (m_state, host, rmsg, 0);
        message_free (rmsg);
    }
}

void scribe_instance::deliver_rp_reply (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    if (msg->size < HOST_CODED_SIZE) {
        HC_LOG_WARN ("host_decode failed, msg too small!");
        return;
    }
    ChimeraHost *host = host_decode(m_state, msg->payload);
    if (host != NULL) {
        boost::mutex::scoped_lock guard (m_grp_mutex);
        group_map_t::iterator it = m_groups.find (msg->src);
        if (it == m_groups.end()) {
            HC_LOG_INFO ("Unknown group, forget about RP.");
        }
        else {
            HC_LOG_DEBUG("Set RP (grp,rp) " << it->second.get_uri().str()
                         << "," << key_to_string (host->key));
            it->second.set_rp(host);
        }
    }
}

void scribe_instance::deliver_parent (Key*, Message *msg)
{
    HC_LOG_TRACE("");
    if (msg->size < HOST_CODED_SIZE) {
        HC_LOG_WARN ("host_decode failed, msg too small!");
        return;
    }
    ChimeraHost *host = host_decode(m_state, msg->payload);

    boost::mutex::scoped_lock guard (m_grp_mutex);
    group_map_t::iterator it = m_groups.find (msg->src);
    if (it != m_groups.end() && !key_equal(m_mykey, host->key)) {
        HC_LOG_INFO ("set parent");
        it->second.set_parent(host);
    }
}

uri scribe_instance::map (const uri &group_uri)
{
    HC_LOG_TRACE("");
    return group_uri;
}

int scribe_instance::create (const uri &group_uri)
{
    HC_LOG_TRACE("");
    ChimeraGlobal* chglo = (ChimeraGlobal*) m_state->chimera;
    Key group_key = uri_mapping(group_uri);
    uri tmp_uri = key_mapping(group_key, 0);
    // encode host AND send group uri with each create, to enable mapping
    size_t ulen= tmp_uri.str().length();
    size_t plen= HOST_CODED_SIZE + 1 + ulen + 1;
    char* t = new char[plen];
    //char t[plen];
    size_t hlen = host_encode(t, 256, chglo->me);
    if (hlen > HOST_CODED_SIZE) {
        HC_LOG_WARN ("host_encode failed, msg too large!");
        return HC_UNKNOWN_ERROR;
    }
    // place separator
    t[hlen] = ':';
    // copy encoded host to larger array
    char* temp = t + hlen + 1;
    tmp_uri.str().copy(temp, ulen, 0);
    t[hlen+1+ulen] = '\0';
    chimera_send(m_state, group_key, group_key, group_uri.port_as_int(),
                 SCRIBE_MSG_CREATE, plen, t);
    delete[] t;

    uri *event_uri = new uri(tmp_uri);
    m_event_cb(m_handle, static_cast<int>(hamcast::new_source_event), event_uri, NULL);
    return HC_SUCCESS;
}

int scribe_instance::join (const uri &group_uri)
{
    HC_LOG_TRACE("");

    Key group_key = uri_mapping(group_uri);
    uri tmp_uri = key_mapping(group_key, 0);
    ChimeraGlobal* chglo = (ChimeraGlobal*) m_state->chimera;
    group_map_t::iterator  it;
    {
        boost::mutex::scoped_lock guard (m_grp_mutex);
        it = m_groups.find(group_key);
        if (it != m_groups.end()) {
            scribe_group &group = it->second;
            if (group.get_uri().empty()) {
                group.set_uri(tmp_uri);
            }
            group.add_child(chglo->me);
        } else {
            scribe_group newgroup (tmp_uri);
            newgroup.add_child(chglo->me);
            it = m_groups.insert(pair<Key,scribe_group>(group_key,newgroup)).first;
        }
    }
    // encode host AND send group uri with each join, to enable mapping
    size_t ulen= tmp_uri.str().length();
    size_t plen= HOST_CODED_SIZE + 1 + ulen + 1;
    char* t = new char[plen];
    size_t hlen = host_encode(t, 256, chglo->me);
    if (hlen > HOST_CODED_SIZE) {
        HC_LOG_WARN ("host_encode failed, msg too large!");
        return HC_UNKNOWN_ERROR;
    }
    // place separator
    t[hlen] = ':';
    // copy uri string to buffer
    char* temp = t + hlen + 1;
    tmp_uri.str().copy(temp,ulen,0);
    t[hlen+1+ulen] = '\0';
    chimera_send(m_state, group_key, group_key, group_uri.port_as_int(),
                 SCRIBE_MSG_JOIN, plen, t);
    delete[] t;
    // inform any application listing for events
    uri *event_uri = new uri(it->second.get_uri());
    m_event_cb(m_handle, static_cast<int>(hamcast::join_event), event_uri, NULL);
    return HC_SUCCESS;
}

int scribe_instance::leave (const uri &group_uri)
{
    HC_LOG_TRACE("");

    Key group_key = uri_mapping(group_uri);
    {
        //boost::mutex::scoped_lock guard (m_grp_mutex);
        group_map_t::iterator it = m_groups.find(group_key);
        if (it != m_groups.end()) {
            scribe_group &group = it->second;
            {
                boost::mutex::scoped_lock guard (m_grp_mutex);
                group.del_child(m_mykey);
            }
            if (group.get_children().size() == 0) {
                char s[256];
                Message *msg;
                ChimeraGlobal* chglo = (ChimeraGlobal*) m_state->chimera;
                size_t slen = host_encode (s, sizeof(s), chglo->me);
                if(group.get_parent() != NULL) {
                    ChimeraHost parent = *(group.get_parent());
                    if (!key_equal(m_mykey, parent.key)) {
                        msg = message_create (parent.key, group_key,
                                              group_uri.port_as_int(),
                                              SCRIBE_MSG_LEAVE, slen, s);
                        message_send (m_state, &parent, msg, 0);
                        message_free(msg);
                    }
                } else {
                    msg = message_create (group_key, group_key,
                                          group_uri.port_as_int(),
                                          SCRIBE_MSG_LEAVE, slen, s);
                    chimera_message(m_state, msg);
                    message_free(msg);
                }
            }
        } else {
            HC_LOG_INFO ("No such group to leave!");
        }
    }
    // inform any application listing for events
    uri *event_uri = new uri(group_uri);
    m_event_cb(m_handle, static_cast<int>(hamcast::join_event), event_uri, NULL);
    return HC_SUCCESS;
}

int scribe_instance::send (const uri &group_uri,
                           const void* payload,
                           const size_t &plen,
                           const int)
{
    HC_LOG_TRACE("");
    Key group_key = uri_mapping (group_uri);
    uri tmp_uri = key_mapping (group_key, 0);
    Message* msg;
    ChimeraHost* rp = NULL;
    {// mutex locked (CASE A/B)}
        boost::mutex::scoped_lock guard (m_grp_mutex);
        group_map_t::iterator it = m_groups.find(group_key);
        if (it != m_groups.end()) {
            rp = it->second.get_rp();
        }
        else {
            HC_LOG_INFO ("Group not found");
            scribe_group newgroup (tmp_uri);
            m_groups.insert(std::pair<Key, scribe_group>(group_key, newgroup));
        }
    }
    if (rp) {
        HC_LOG_INFO ("Group RP found");
        msg = message_create (rp->key, group_key, group_uri.port_as_int(),
                              SCRIBE_MSG_MULTICAST, plen,
                              static_cast<const char*>(payload));
        if (key_equal(rp->key, m_mykey)) {
            deliver_multicast(&(rp->key), msg);
        } else {
            HC_LOG_DEBUG("(grp,src,dst) " << group_uri << ","
                                          << key_to_string(m_mykey) << ","
                                          << key_to_string(rp->key));
            message_send (m_state, rp, msg, 0);
        }
        message_free(msg);
    }
    else {
        HC_LOG_INFO ("Group RP not found");
        send_rp_request (group_uri);
        HC_LOG_INFO (" - send RP request");
        msg = message_create (group_key, group_key, group_uri.port_as_int(),
                              SCRIBE_MSG_MULTICAST,
                              plen, static_cast<const char*>(payload));
        HC_LOG_DEBUG("(grp,src,dst) " << group_uri << ","
                                      << key_to_string(m_mykey) << ","
                                      << key_to_string(group_key));
        chimera_message (m_state, msg);
        message_free(msg);
    }
    return plen;
}

void scribe_instance::send_rp_request(const hamcast::uri &group_uri)
{
    HC_LOG_TRACE("");
    Key group_key = uri_mapping (group_uri);
    ChimeraGlobal* chglo = (ChimeraGlobal*)m_state->chimera;
    char s[256];
    size_t slen = host_encode (s, sizeof(s), chglo->me);
    chimera_send(m_state, group_key, group_key, group_uri.port_as_int(),
                 SCRIBE_MSG_RP_REQUEST, slen, s);
}

void scribe_instance::neighbor_set(vector<uri> &result)
{
    HC_LOG_TRACE("");
    ChimeraHost **leafset;
    leafset = route_neighbors (m_state, LEAFSET_SIZE);
    for (int k=0; leafset[k] != NULL; ++k) {
        result.push_back(uri ("scribe://" + key_to_string(leafset[k]->key)));
    }
    free(leafset);
}

void scribe_instance::group_set (vector< pair<uri,int> > &result)
{
    HC_LOG_TRACE("");
    group_map_t::iterator it;
    group_map_t groups;
    {
        boost::mutex::scoped_lock guard (m_grp_mutex);
        groups = m_groups;
    }

    for (it = groups.begin(); it != groups.end(); ++it) {
        scribe_group& g = it->second;
        uri group_uri = g.get_uri();
        if (group_uri.empty()) {
            group_uri = key_mapping (g.get_gid (),0);
            if (group_uri.empty()) {
                HC_LOG_WARN ("Unable to map key to uri and back!");
                continue;
            }
        }
        if (g.has_child(m_mykey)) {
            if (g.children () > 1) { // okay, i'm not the only child
                result.push_back(pair<uri,int>(group_uri, HC_SENDER_AND_LISTENER_STATE));
            }
            else {
                result.push_back(pair<uri,int>(group_uri, HC_LISTENER_STATE));
            }
        }
        else {
            result.push_back(pair<uri,int>(group_uri, HC_SENDER_STATE));
        }
    }
}

void scribe_instance::parent_set(vector<uri> &result, const hamcast::uri &group_uri)
{
    HC_LOG_TRACE("");
    Key group_key = uri_mapping(group_uri);
    boost::mutex::scoped_lock guard (m_grp_mutex);
    group_map_t::iterator it = m_groups.find(group_key);
    if (it != m_groups.end()) {
        ChimeraHost *parent = it->second.get_parent();
        if (parent != NULL) {
            string p_addr ("scribe://" + key_to_string(parent->key));
            HC_LOG_INFO (" - parent: " << p_addr);
            result.push_back(uri(p_addr));
        }
        else {
            HC_LOG_INFO (" - no parent.");
        }
    }
}

void scribe_instance::children_set (vector<uri> &result, const uri &group_uri)
{
    HC_LOG_TRACE("CHILDREN_SET");
    Key group_key = uri_mapping(group_uri);
    boost::mutex::scoped_lock guard (m_grp_mutex);
    group_map_t::iterator it = m_groups.find(group_key);
    if (it != m_groups.end()) {
        scribe_group& group = it->second;
        children_map_t children = group.get_children();
        children_map_t::iterator cit;
        HC_LOG_INFO ("Size of children list: " << children.size());
        for (cit = children.begin(); cit!=children.end(); ++cit) {
            ChimeraHost *child = cit->second.get_host();
            if (!key_equal(m_mykey, child->key)) {
                string c_addr ("scribe://" + key_to_string(child->key));
                HC_LOG_INFO (" - child: " << c_addr);
                result.push_back(uri(c_addr));
            }
        }
    }
}

int scribe_instance::designated_host (const hamcast::uri &group_uri)
{
    HC_LOG_TRACE("");
    Key group_key = uri_mapping (group_uri);
    boost::mutex::scoped_lock guard (m_grp_mutex);
    group_map_t::iterator it = m_groups.find(group_key);
    int ret = 0;    /* 0=false, 1=true */
    if (it != m_groups.end()){
        scribe_group &g = it->second;
        if( (g.get_children().size() > 1) ||
            ( (g.get_children().size() == 1) && (!g.has_child(m_mykey)) ) ) {
            ret = 1;
        }
    }
    return ret;
}

void scribe_instance::forward_join (Key*, Message *msg, ChimeraHost *nextHost)
{
    HC_LOG_TRACE("");
    HC_LOG_INFO ("Forward join for group: " << key_to_string(msg->src));
    uri group_uri = key_mapping(msg->src, msg->port); // dummy call, to establish mapping
    group_uri = key_mapping (msg->src, 0);

    if (group_uri.empty ()) {
        HC_LOG_WARN ("Cache Miss: group uri unknown.");
        if (msg->size >= HOST_CODED_SIZE) {
            HC_LOG_INFO ("decode join message.");
            // msg: [ [HOST]:[GROUP] ]
            char *group_str = (msg->payload+HOST_CODED_SIZE+1);
            uri tmp_uri = uri(group_str);
            if (!tmp_uri.empty()) {
                Key group_key = uri_mapping(tmp_uri);
                group_uri = key_mapping(group_key, 0);
            }
            HC_LOG_INFO ("Groupstr: " << group_str);
            HC_LOG_INFO ("Groupkey: " << key_to_string(msg->src));
        }
        else {
            HC_LOG_WARN ("FAILED to decode join msg, group key: " << key_to_string(msg->src));
            return;
        }
    }
    else {
        HC_LOG_INFO ("Cache Hit: group uri known.");
    }

    /*
     * add this node as child to local group
     * if group not exists, create and add it
     */
    group_map_t::iterator it;
    ChimeraHost *host = host_decode(m_state, msg->payload);
    ChimeraGlobal* chglo = (ChimeraGlobal*)m_state->chimera;
    host_encode (msg->payload, HOST_CODED_SIZE, chglo->me);
    { // scope for lock
        boost::mutex::scoped_lock guard (m_grp_mutex);
        it = m_groups.find (msg->src);
        if (it == m_groups.end()) { //unknown group, create it
            HC_LOG_INFO ("Group not found, create!");
            it = m_groups.insert(pair<Key, scribe_group>(msg->src, scribe_group(group_uri))).first;
        }
        scribe_group &group = it->second;
//        if (group.children() > 0) {
//            HC_LOG_DEBUG("Group already joined, dont forward join.");
//            // do not forward join, already joined
//            nextHost = NULL;
//        }
//        else { // new group forward join
//            HC_LOG_DEBUG("Fist member, forward join.");
//            host_encode (msg->payload, HOST_CODED_SIZE, chglo->me);
//        }
        HC_LOG_INFO (" - add host as child.");
        group.add_child(host);
    }
    if (key_equal (m_mykey, host->key)) {
        HC_LOG_INFO ("Do not send myself as parent to me!");
    }
    else {
        HC_LOG_DEBUG("(grp,child,parent) "  << group_uri << ","
                                            << key_to_string(host->key) << ","
                                            << key_to_string(m_mykey));
        char s[256];
        size_t slen = host_encode (s, sizeof(s), chglo->me);
        Message *pmsg;
        pmsg = message_create (host->key, msg->src, msg->port,
                               SCRIBE_MSG_PARENT, slen, s);
        message_send (m_state, host, pmsg, 0);
        message_free(pmsg);
    }
    if (!group_uri.empty()) {
        uri *event_uri = new uri(group_uri);
        m_event_cb(m_handle, static_cast<int>(hamcast::join_event), event_uri, NULL);
    }
}

void scribe_instance::shutdown()
{
    HC_LOG_TRACE("DIE, DIE, DIE my darling ...");
}


void scribe_instance::get_routing_table (vector<string>& rtable)
{
    RouteGlobal *rg = (RouteGlobal *) m_state->route;
    rtable.push_back ("---- SCRIBE ROUTING TABLE ----");
    for (size_t i=0; i < MAX_ROW; ++i) {
        for (size_t j=0; j < MAX_COL; ++j) {
            for (size_t k=0; k < MAX_ENTRY; ++k) {
                if (rg->table[i][j][k] != NULL) {
                    std::stringstream ss;
                    ss << "(" << i << "," << j << "," << k << ") "
                       << key_to_string (rg->table[i][j][k]->key);
                    rtable.push_back (ss.str ());
                }
            }
        }
    }
    rtable.push_back ("------------------------------");
}
