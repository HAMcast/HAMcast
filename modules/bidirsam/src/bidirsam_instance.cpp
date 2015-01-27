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

//hamcast includes
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

// chimera
#include <chimera/chimera.h>
#include <chimera/route.h>
#include <chimera/key.h>

// local includes
#include "bidirsam_instance.hpp"
#include "bidirsam.hpp"

//more includes
#include <algorithm>
#include <boost/ptr_container/ptr_vector.hpp>
#include <sys/time.h>
#include <sstream>

using std::vector;
using std::pair;
using std::string;
using hamcast::uri;

void bidirsam_instance::shutdown ()
{
    HC_LOG_TRACE("");
}

const hamcast::uri& bidirsam_instance::map(const uri &group_uri)
{
    HC_LOG_TRACE("");
    return group_uri;
}

int bidirsam_instance::join (const uri &group_uri)
{
    HC_LOG_TRACE("");
    return join_leave_injection(BIDIRSAM_JOIN_MESSAGE, group_uri);
}

int bidirsam_instance::leave (const uri &group_uri)
{
    HC_LOG_TRACE("");
    return join_leave_injection(BIDIRSAM_LEAVE_MESSAGE, group_uri);
}

int bidirsam_instance::send (const uri &group_uri, const void* payload,
                             const size_t &plen, int)
{
    HC_LOG_TRACE("");
    // Create a Key from uri u and forward message into the group tree with
    Key group_key = uri_mapping(group_uri);
    uint32_t lcp = 0;
    HC_LOG_DEBUG("(grp,snd) " << group_uri << "," << key_to_string(m_mykey));
    return forward_multicast_data(lcp, (char*) payload, plen, group_uri, group_key);
}

void bidirsam_instance::neighbor_set(vector<uri>& result)
{
    HC_LOG_TRACE("");
    // Return all chimera routing neighbors
    ChimeraHost **leafset;
    leafset = route_neighbors (m_chimera_state, LEAFSET_SIZE);
    for (int k=0; leafset[k] != NULL; ++k) {
        result.push_back(uri ("bidir://" + key_to_string(leafset[k]->key)));
    }
    free(leafset);
}

void bidirsam_instance::group_set (vector< pair<uri,int> >& result)
{
    HC_LOG_TRACE("");
    // Copy joined groups list
    std::vector<Key> p_joined_groups;
    {
        boost::mutex::scoped_lock group_lock (m_grp_mutex);
        p_joined_groups = m_joined_groups;
    }
    // Copy mft list
    std::map<Key, bidirsam_mft> p_mfts;
    {
        boost::mutex::scoped_lock mft_lock (m_mft_mutex);
        p_mfts = m_mfts;
    }
    // Iterate over all mfts and joined groups and add them to result.
    // If this node is a receiver of a group set state to HC_LISTENER else
    // state is HC_SENDER_STATE.
    std::map<Key,bidirsam_mft>::iterator it;
    for (it = p_mfts.begin() ; it != p_mfts.end(); ++it){
        uri group_uri = key_mapping(it->first,0);
        if (group_uri.empty()) {
            HC_LOG_DEBUG ("Unable to map key to uri and back!");
            continue;
        }
        else{
            std::vector<Key>::iterator g_it = std::find(p_joined_groups.begin(),
                                                        p_joined_groups.end(),
                                                        it->first);
            // Node joined group state = Listener
            if(g_it != p_joined_groups.end()){
                bidirsam_mft& mft = it->second;
                if(mft.get_mft().empty()){
                    result.push_back(pair<uri,int>(group_uri, HC_LISTENER_STATE));
                }
                else {
                    result.push_back(pair<uri,int>(group_uri, HC_SENDER_AND_LISTENER_STATE));
                }
            }
            else{
                // Node is a possible forwarder for this Group
                result.push_back(pair<uri,int>(group_uri, HC_SENDER_STATE));
            }
        }
    }
}

void bidirsam_instance::parent_set (vector<uri>&, const uri &)
{
    HC_LOG_TRACE("");
    HC_LOG_DEBUG (" - parent: always empty");
}

void bidirsam_instance::children_set (vector<uri>& result, const uri &group_uri)
{
    HC_LOG_TRACE("");
    //HC_LOG_TRACE("CHILDREN_SET");
    Key group_key = uri_mapping(group_uri);
    //Lock mft mutex
    std::map<Key, bidirsam_mft> p_mfts;
    {
        boost::mutex::scoped_lock guard (m_mft_mutex);
        p_mfts = m_mfts;
    }
    // If group found, add all prefixes in mft into the result vector
    std::map<Key, bidirsam_mft>::iterator it = p_mfts.find(group_key);
    if (it != p_mfts.end()) {
        bidirsam_mft& mft = it->second;
        if(mft.get_mft().empty()){
            HC_LOG_INFO("Empty Children Set");
        }
        else{
            std::map<Key, int>::iterator it;
            for (it=mft.get_mft().begin() ; it != mft.get_mft().end(); ++it) {
                string c_addr ("bidir://" + key_to_string(it->first));
                HC_LOG_INFO (" - child: " << c_addr);
                result.push_back(uri(c_addr));
            }
        }
    }
    else{
        HC_LOG_INFO("No MFT FOUND");
    }
}

int bidirsam_instance::designated_host (const uri &group_uri)
{
    HC_LOG_TRACE("");
    // If the node has a mft for group u, he is a designated host for this group
    Key group_key = uri_mapping(group_uri);
    boost::mutex::scoped_lock mft_lock (m_mft_mutex);
    std::map<Key,bidirsam_mft>::iterator it = m_mfts.find(group_key);
    return it != m_mfts.end() ? 1 : 0;
}

void bidirsam_instance::prefix_flooding(int row,
                                        const uri& group_uri,
                                        const uint16_t &type,
                                        const Key &src)
{
    HC_LOG_TRACE("");
    // Iterate over the chimera routing table and forward join/leave message
    // to all node with key_index >= row
    ChimeraHost **rt = route_get_table(m_chimera_state);
    for (int i = 0; rt[i] != NULL; ++i)
    {
        int index =  key_index_int(m_mykey,rt[i]->key);
        if(index >= row)
        {
            Key destination = rt[i]->key;
            Message* msg = create_join_leave_message (group_uri, destination, type, src);
            message_send (m_chimera_state, rt[i], msg, false);
            message_free(msg);
        }
    }
    delete rt;
}

Key bidirsam_instance::uri_mapping(const uri &group_uri)
{
    HC_LOG_TRACE("");
    Key k;
    string tmp = plain_uri_str (group_uri);
    string prt = group_uri.port ();
    uint32_t prti = 0;
    boost::mutex::scoped_lock guard (m_map_mutex);
    std::map<string, Key>::iterator it;
    if (!prt.empty ()) {
        it = m_key_cache.find(tmp+":"+prt);
        prti = group_uri.port_as_int ();
    }
    else {
        it = m_key_cache.find(tmp);
    }

    if (it != m_key_cache.end()) {
        k = it->second;
    } else {
        key_makehash (NULL, &k, tmp.c_str());
        vector<uint32_t> v (k.t, k.t + KEY_ARRAY_SIZE);
        m_key_cache.insert(pair<string, Key>(tmp,k));
        m_uri_cache.insert(pair<vector<uint32_t>, uri>(v,uri(tmp)));
        if (prti != 0) {
            v.push_back (prti);
            m_key_cache.insert(pair<string, Key>(tmp+":"+prt,k));
            m_uri_cache.insert(pair<vector<uint32_t>, uri>(v,uri(tmp+":"+prt)));
        }
    }
    return k;
}

uri bidirsam_instance::key_mapping (const Key &k, const uint32_t &port)
{
    HC_LOG_TRACE("");
    vector<uint32_t> kvec (k.t, k.t + KEY_ARRAY_SIZE);
    if (port > 0) {
        kvec.push_back (port);
    }
    boost::mutex::scoped_lock guard (m_map_mutex);
    std::map<vector<uint32_t>, uri>::iterator it = m_uri_cache.find (kvec);
    if (it != m_uri_cache.end ()) {
        return (it->second);
    }
    else if (port > 0) {
        kvec.pop_back();
        it = m_uri_cache.find(kvec);
        if (it != m_uri_cache.end()) {
            std::stringstream stream;
            stream << it->second.str() << ":" << port;
            uri tmp = uri (stream.str());
            uri_mapping(tmp);
            return tmp;
        }
    }
    return (uri());
}

int bidirsam_instance::join_leave_injection(const uint16_t &type, const uri &group_uri)
{
    HC_LOG_TRACE("");
    //Remove port from uri
    Key group_key = uri_mapping(group_uri);
    // Remember joined Group
    boost::mutex::scoped_lock group_lock (m_grp_mutex);
    vector<Key>::iterator pos = std::find(m_joined_groups.begin(),
                                               m_joined_groups.end(),
                                               group_key);
    if ( pos != m_joined_groups.end() )
    {
        if(type == BIDIRSAM_LEAVE_MESSAGE)
        {
            m_joined_groups.erase(pos);
        }
    }
    else
    {
        if(type == BIDIRSAM_JOIN_MESSAGE)
        {
            m_joined_groups.push_back(group_key);
        }
    }

    //Find MFT or create MFT
    std::map<Key, bidirsam_mft>::iterator it;
    boost::mutex::scoped_lock mft_lock (m_mft_mutex);
    it = m_mfts.find(group_key);
    // MFT not found broadcast join to *
    if (it == m_mfts.end())
    {
        // create new mft
        bidirsam_mft new_mft;
        m_mfts.insert(std::pair<Key,bidirsam_mft>(group_key,new_mft));
        prefix_flooding(0, group_uri, type, m_mykey);
    }
    else
    {
        // Flood only subtree
        bidirsam_mft& mft =  it->second;
        //Get LCP
        Key k = mft.get_lcp(m_mykey);
        if( key_equal(m_mykey,k)) return -1;

        int row = key_index_int (m_mykey, k);
        // Flood message to prefix with from row
        prefix_flooding(row, group_uri, type, m_mykey);
    }
    return 0;
}

void bidirsam_instance::join_leave_processing(Message *msg)
{
    HC_LOG_TRACE("");
    uint16_t type = msg->type;
    int lcp = key_index(NULL, m_mykey, msg->src);
    Key prefix;
    create_prefix_int(msg->src, lcp+1, 0, prefix);
    char * payload = msg->payload;
    // Read the key of the joining node from msg buffer
    Key msg_source = read_key_from_buffer(payload);
    // Skip key size
    payload += KEY_ARRAY_SIZE * sizeof(uint32_t);
    // Read group uri from msg buffer
    uri group_uri(payload);
    // Map group to key
    Key group_key = uri_mapping(group_uri);

    if(type == BIDIRSAM_LEAVE_MESSAGE)
    {
        std::map<Key, bidirsam_mft>::iterator it;
        boost::mutex::scoped_lock mft_lock (m_mft_mutex);
        // Find mft for this group with group_key
        it = m_mfts.find(group_key);
        if (it != m_mfts.end()){
            // If group exsist, delete leaving node from mft
            bidirsam_mft& mft = it->second;
            mft.delete_entry(prefix);
            remove_prefix_ref_to_cache(prefix);
        }
    }
    else if(type == BIDIRSAM_JOIN_MESSAGE)
    {
        std::map<Key, bidirsam_mft>::iterator it;
        boost::mutex::scoped_lock mft_lock (m_mft_mutex);
        // If group exsist add joining node to mft,
        // else create new mft and add joining the nodes
        it = m_mfts.find(group_key);
        if (it != m_mfts.end())
        {
            bidirsam_mft& mft = it->second;
            mft.insert_entry(prefix);
            add_prefix_ref_to_cache(prefix);
        }
        else
        {
            bidirsam_mft new_mft;
            new_mft.insert_entry(prefix);
            m_mfts.insert(std::pair<Key,bidirsam_mft>(group_key,new_mft));
            add_prefix_ref_to_cache(prefix);
        }
    }
    // Create lcp with message source and forward join/leave message to subtree
    int new_lcp = key_index_int(m_mykey, msg_source);
    prefix_flooding(new_lcp+1, group_uri, type, msg->src);
}

int bidirsam_instance::forward_multicast_data(const uint32_t& lcp,
                                              char *payload,
                                              const size_t &plen,
                                              const uri &group_uri,
                                              const Key& group_key)
{
    HC_LOG_TRACE("LCP: " << lcp << ", PLEN: " << plen << ", URI: " << group_uri << ", KEY: " << key_to_string(group_key));
    std::map<Key, bidirsam_mft>::iterator it;
    boost::mutex::scoped_lock mft_lock (m_mft_mutex);
    ChimeraHost *host = NULL;
    uint32_t index = 0;
    it = m_mfts.find(group_key);
    //    Message* msg = NULL;
    if (it != m_mfts.end())
    {
        bidirsam_mft& mft = it->second;
        std::map<Key,int>& current_mft = mft.get_mft();
        std::map<Key,int>::iterator it;
        std::vector<Key> used_hosts;
        for ( it=current_mft.begin() ; it != current_mft.end(); ++it )
        {
            std::map<Key, std::pair<ChimeraHost*,uint32_t> >::iterator destination_host;
            destination_host = m_host_cache.find(it->first);
            if(destination_host != m_host_cache.end() )
            {
                host = destination_host->second.first;
                index = destination_host->second.second;
            }
            else
            {
                host = route_lookup (m_chimera_state, it->first, 1, 0)[0];
                // Not the nodes own Key. If no possible node is found destination = ownkey. Key has to have at least an lcp <= index
                if(!key_equal(m_mykey,host->key))
                {
                    index = key_index_int(m_mykey,host->key);
                    m_host_cache.insert(pair<Key,pair<ChimeraHost*,uint32_t> >(it->first, pair<ChimeraHost*,uint32_t>(host,index)));
                }
            }
            std::vector<Key>::iterator kit = std::find(used_hosts.begin(), used_hosts.end(), host->key);
            if((lcp <= index) && (host != NULL) && (kit == used_hosts.end()))
            {
                HC_LOG_DEBUG("HOST: " << key_to_string(host->key) << ", INDEX: " << index);
            // if(lcp < index && host != NULL) { // should be this?
//                Message* msg = create_multicast_message(payload,
//                                                        plen,
//                                                        index,
//                                                        group_key,
//                                                        host->key,
//                                                        group_uri);
                Message* msg = create_multicast_message(payload,
                                                        plen,
                                                        index+1,
                                                        group_key,
                                                        host->key,
                                                        group_uri);
                HC_LOG_DEBUG("(grp,src,dst,lcp) " << group_uri
                             << "," << key_to_string(m_mykey)
                             << "," << key_to_string(host->key)
                             << "," << (lcp+1));
                message_send(m_chimera_state, host, msg, false);
                message_free(msg);
                used_hosts.push_back(host->key);
            }
        }
    }
    return 0;
}

Message* bidirsam_instance::create_multicast_message(char *payload,
                                                     const size_t &plen,
                                                     const uint32_t& lcp,
                                                     const Key& group_key,
                                                     const Key& destination,
                                                     const uri &group_uri)
{
    HC_LOG_TRACE("");
    //Create multicast message, contains message destination Key, group_key and port, lcp
    size_t size = plen+sizeof(uint32_t);
    char* send_data = new char[size];
    memcpy(send_data, &lcp, sizeof(uint32_t));
    memcpy(send_data+sizeof(uint32_t), payload, plen);
    Message* msg = message_create(destination, group_key, group_uri.port_as_int(),
                                  BIDIRSAM_MULTICAST_MESSAGE, size, send_data);
    delete send_data;
    return msg;
}


void bidirsam_instance::deliver_multicast_data(Message *msg)
{
    HC_LOG_TRACE("");
    uint32_t row=0;
    memcpy(&row,msg->payload,sizeof(uint32_t));
    //row+=1;

    std::map<vector<uint32_t>,uri>::iterator m_it;
    // Decode multicast message
    vector<uint32_t> m_group_uri_finder(msg->src.t, msg->src.t + KEY_ARRAY_SIZE);
    m_group_uri_finder.push_back (msg->port);
    m_it = m_uri_cache.find(m_group_uri_finder);
    // Check for valid group uri
    if(m_it != m_uri_cache.end())
    {
        vector<Key>::iterator it;
        uri group_uri = m_it->second;
        HC_LOG_DEBUG("(grp,src,rcv,lcp) " << group_uri
                     << "," << key_to_string(msg->src)
                     << "," << key_to_string(m_mykey)
                     << "," << (row));
        Key& group_key = msg->src;
        boost::mutex::scoped_lock group_lock (m_grp_mutex);
        // If node is a receiver for this group, proceed data upward to application
        it = std::find(m_joined_groups.begin(), m_joined_groups.end(), group_key);
        size_t size = msg->size - sizeof(uint32_t);
        if (it != m_joined_groups.end())
        {
            m_recv_cb(m_handle, msg->payload+sizeof(uint32_t),
                      size, &group_uri, group_uri.c_str());
        }
        // Forward multicast message
        forward_multicast_data(row, msg->payload+sizeof(uint32_t),
                               size, group_uri, group_key);
    }
}

void bidirsam_instance::create_prefix_int(const Key &k,
                                          const size_t &len,
                                          const int &col,
                                          Key &p)
{
    HC_LOG_TRACE("");
    int jump = KEY_ARRAY_SIZE-1;
    key_assign(&p,k);    // Copy Key
    // Calculate the array field to jump to
    int can_jump = len / m_number_of_digits;
    jump -= can_jump;
    int to_move_now = len-(can_jump*m_number_of_digits); // = len - ((len/m_number_of_digits)*m_number_of_digits) = 0 ?! WTF ?!
    //int to_move_now = len-(jump*m_number_of_digits); // should be this, or not?!
    int right_shift = (m_number_of_digits-to_move_now) * BASE_B;
    if(right_shift == 32)
    {
        if(jump < KEY_ARRAY_SIZE && jump >=0)
        {
            p.t[jump] =  0;
        }
    }
    else
    {
        // What? Make this sense?
        p.t[jump] =  p.t[jump] >> right_shift;
        p.t[jump] =  p.t[jump] << right_shift;
    }
    for(int i = jump-1; i >= 0 ; i--)
    {
        p.t[i] =0;
    }
    if(col != 0)
    {
        uint32_t tmp = col;
        int to_shift = right_shift-BASE_B;
        tmp = tmp << to_shift;
        p.t[jump] = p.t[jump] | tmp;
    }
    HC_LOG_DEBUG("KEY:" << key_to_string(k) << ", LCP: " << len << ", COL: " << col << ", PREFIX: " << key_to_string(p));
}

Message *bidirsam_instance::create_join_leave_message(const uri& group_uri,
                                                      const Key& destination,
                                                      const uint16_t &type,
                                                      const Key &src)
{
    HC_LOG_TRACE("");
    // Create join or leave message, contains group uri, destination key,
    // joining/leaving node key
    int group_size = group_uri.str().length()+1;
    int size = group_size;
    size += (KEY_ARRAY_SIZE * sizeof(uint32_t));
    char * msg_payload = new char[size];
    write_key_to_buffer (m_mykey, msg_payload);
    memcpy(msg_payload+(KEY_ARRAY_SIZE * sizeof(uint32_t)), group_uri.c_str(), group_size);
    Message* msg = message_create(destination, src, group_uri.port_as_int(),
                                  type, size, msg_payload);
    delete msg_payload;
    return msg;
}

void bidirsam_instance::add_prefix_ref_to_cache(const Key &prefix)
{
    HC_LOG_TRACE("");
    // Add prefix to the host_cache
    std::map<Key, int>::iterator it;
    boost::mutex::scoped_lock cache_lock (m_cache_mutex);
    it = m_ref_count.find(prefix);
    if(it != m_ref_count.end() )
    {
        int & count = it->second;
        count++;
    }
    else
    {
        m_ref_count.insert(std::pair<Key,int>(prefix,0));
    }
}

void bidirsam_instance::remove_prefix_ref_to_cache(const Key &prefix)
{
    HC_LOG_TRACE("");
    // Remove prefix from host cache
    std::map<Key, int>::iterator it;
    boost::mutex::scoped_lock cache_lock (m_cache_mutex);
    it = m_ref_count.find(prefix);
    if(it != m_ref_count.end() )
    {
        int & count = it->second;
        count--;
        if(count < 1)
        {
            m_ref_count.erase(it);
        }
    }
}

void bidirsam_instance::maintenance()
{
    HC_LOG_TRACE("");
    while (m_mt)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(c_maintenance_timer));
        // wait done, do something
        send_rejoin();
        mft_maintenance();
    }
}

void bidirsam_instance::send_rejoin()
{
    // Send rejoin
    std::vector<Key> p_joined_groups;
    {
        boost::mutex::scoped_lock group_lock (m_grp_mutex);
        p_joined_groups = m_joined_groups;
    }
    //boost::mutex::scoped_lock mft_lock (m_mft_mutex);
    //std::map<Key,bidirsam_mft> p_mfts = m_mfts;

    for(size_t i = 0; i < p_joined_groups.size(); ++i)
    {
        Key& group = p_joined_groups.at(i);
        uri target_group_uri = key_mapping(group,0);
        prefix_flooding(0, target_group_uri, BIDIRSAM_JOIN_MESSAGE, m_mykey);
    }
}

void bidirsam_instance::mft_maintenance()
{
    // MFT Maintenance
     std::map<Key, bidirsam_mft>::iterator mft_it;
     {// mutex scope
        boost::mutex::scoped_lock mft_lock (m_mft_mutex);
        for (mft_it = m_mfts.begin(); mft_it != m_mfts.end(); mft_it++)
        {
            bidirsam_mft& mft = mft_it->second;
            std::map<Key,int>& current_mft = mft.get_mft();
            std::map<Key,int>::iterator it;
            for(it = current_mft.begin(); it != current_mft.end();it++)
            {
                Key k = it->first;
                mft.decrement_entry(k);
            }
        }
     }// mutex scope
     vector<string> rtable;
     get_routing_table (rtable);
     for (size_t s=0; s < rtable.size (); ++s)
     {
         HC_LOG_DEBUG(rtable[s]);
     }
}

void bidirsam_instance::get_routing_table (vector<string>& rtable)
{
    RouteGlobal *rg = (RouteGlobal *) m_chimera_state->route;
    rtable.push_back ("--- BIDIRSAM ROUTING TABLE ---");
    for (size_t i=0; i < MAX_ROW; ++i)
    {
        for (size_t j=0; j < MAX_COL; ++j)
        {
            for (size_t k=0; k < MAX_ENTRY; ++k)
            {
                if (rg->table[i][j][k] != NULL)
                {
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

