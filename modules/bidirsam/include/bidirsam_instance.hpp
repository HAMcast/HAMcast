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

#ifndef BIDIRSAM_INSTANCE_HPP
#define BIDIRSAM_INSTANCE_HPP

#include <utility>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

//hamcast includes
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

// chimera
#include <chimera/chimera.h>
#include <chimera/route.h>
#include <chimera/key.h>

// local includes
#include "bidirsam.hpp"
#include "bidirsam_mft.hpp"


typedef std::map<std::string, Key> key_map_t;
typedef std::map<std::vector<uint32_t>, hamcast::uri> uri_map_t;

class bidirsam_instance
{

//#define DEBUG

private:

    ChimeraState *                  m_chimera_state;
    hc_log_fun_t                    m_log_fun;  // logging function
    hc_event_callback_t             m_event_cb; // event callback
    hc_recv_callback_t              m_recv_cb;  // receive callback
    hc_atomic_msg_size_callback_t   m_msg_cb;   // atomic msg size callback
    hc_module_instance_handle_t     m_handle;
    Key                             m_mykey;
    std::vector<Key>                m_joined_groups;
    std::map<Key,bidirsam_mft>      m_mfts;
    std::map<Key, int>              m_ref_count;
    key_map_t                       m_key_cache;
    uri_map_t                       m_uri_cache;
    std::map<Key, std::pair<ChimeraHost*,uint32_t> >    m_host_cache;

    int             m_port;
    int             m_number_of_digits;
    bool            m_mt;
    std::string     m_tmp_string;

    // THREADING
    boost::thread   m_maintenance_thread;
    boost::mutex    m_grp_mutex;
    boost::mutex    m_mft_mutex;
    boost::mutex    m_cache_mutex;
    boost::mutex    m_map_mutex;

    /**
      * @brief Maps hamcast::uri to Chimera Key
      * @param group_uri
      * @return returns Chimera Key for hamcast uri u
      */
    Key uri_mapping (const hamcast::uri &group_uri);

    /**
      * @brief Maps Chimera Key to hamcast::uri
      * @param k
      * @param port
      * @return returns hamcast::uri for Key k
    */
    hamcast::uri key_mapping (const Key &k, const uint32_t &port);

    /**
     * @brief plain_uri_str
     * @param group_uri
     * @return string of uri without port and path
     */
    inline std::string plain_uri_str (const hamcast::uri &group_uri)
    {
        if(group_uri.instantiation().empty())
        {
            return (group_uri.ham_scheme() + group_uri.ham_namespace() + ":" + group_uri.group());
        }
        else
        {
            return (group_uri.ham_scheme() + group_uri.ham_namespace() + ":" + group_uri.group() + "@" + group_uri.instantiation());
        }
    }

    /**
     * @brief join_leave_injection
     * @param type
     * @param u
     * @return
     */
    int join_leave_injection(const uint16_t &type,
                             const hamcast::uri &group_uri);

    /**
     * @brief uri_from_msg
     * @param msg
     * @return
     */
    hamcast::uri uri_from_msg(Message* msg);

    /**
     * @brief create_multicast_message
     * @param payload
     * @param plen
     * @param lcp
     * @param group_key
     * @param destination
     * @param group_uri
     * @return
     */
    Message* create_multicast_message(char* payload,
                                      const size_t &plen,
                                      const uint32_t &lcp,
                                      const Key &group_key,
                                      const Key &destination,
                                      const hamcast::uri &group_uri);

    /**
     * @brief create_join_leave_message
     * @param group_uri
     * @param destination
     * @param type
     * @param src
     * @return
     */
    Message* create_join_leave_message(const hamcast::uri &group_uri,
                                       const Key &destination,
                                       const uint16_t &type,
                                       const Key &src);

    /**
     * @brief add_prefix_ref_to_cache
     * @param prefix
     */
    void add_prefix_ref_to_cache(const Key &prefix);

    /**
     * @brief remove_prefix_ref_to_cache
     * @param prefix
     */
    void remove_prefix_ref_to_cache(const Key &prefix);

    /**
     * @brief maintenance
     */
    void maintenance();

    /**
      * @brief sends rejoin message to all joined groups
      */
    void send_rejoin();

    /**
      * @brief removes dead prefixes from mfts
      */
    void mft_maintenance();

public:

    /**
     * @brief bidirsam_instance
     * @param recv_cb
     * @param mt
     */
    bidirsam_instance(hc_recv_callback_t rcb,
                      bool mt) : m_recv_cb(rcb), m_mt(mt)
    {
        m_number_of_digits = (sizeof(uint32_t)*8)/BASE_B;
    }

    bidirsam_instance(hc_event_callback_t ecb,
                      hc_recv_callback_t rcb,
                      hc_atomic_msg_size_callback_t mcb,
                      bool mt) : m_event_cb(ecb), m_recv_cb(rcb), m_msg_cb(mcb), m_mt(mt)
    {
        m_number_of_digits = (sizeof(uint32_t)*8)/BASE_B;
    }

    /**
     * @brief create a prefix from a key
     * @param k is source key
     * @param len is length of prefix
     * @param col is last digit of prefix
     */
    void create_prefix_int(const Key &k,
                           const size_t &len,
                           const int &col, Key &p);


    /**
     * @brief prefix flooding of bidirsam
     * @param row is the starting point for the prefix flooding = 0 broadcast,
     * @param group_uri
     * @param type
     * @param src
     */
    void prefix_flooding(int row,
                         const hamcast::uri &group_uri,
                         const uint16_t &type,
                         const Key &src);

    /**
     * @brief join_leave_processing
     * @param msg
     */
    void join_leave_processing(Message *msg);

    /**
     * @brief forward_multicast_data
     * @param lcp
     * @param payload
     * @param plen
     * @param group_uri
     * @param group_key
     * @return
     */
    int forward_multicast_data(const uint32_t &lcp,
                               char* payload,
                               const size_t &plen,
                               const hamcast::uri &group_uri,
                               const Key &group_key);

    /**
     * @brief deliver_multicast_data
     * @param msg
     */
    void deliver_multicast_data(Message* msg);

    /**
     * @brief start_maintenance
     */
    inline void start_maintenance()
    {
        if (m_mt) {
            m_maintenance_thread = boost::thread(&bidirsam_instance::maintenance, this);
        }
    }

    /**
     * @brief Set the ChimeraState
     * @param ChimeraState*
     */
    inline void set_chimera_state(ChimeraState* state)
    {
        HC_LOG_TRACE("");
        m_chimera_state = state;
        //TODO: MAYBE not so SAVE
        ChimeraGlobal* chglo = reinterpret_cast<ChimeraGlobal*>(m_chimera_state->chimera);
        key_assign(&m_mykey, chglo->me->key);
    }

    /**
     * @brief get_key
     * @return
     */
    inline Key get_key(){
        return m_mykey;
    }

    /**
     * @brief Get the ChimeraState
     * @return ChimeraState*
     */
    inline ChimeraState* get_chimera_state(){
        return m_chimera_state;
    }

    /**
     * @brief Set the module handle of this scribe instance
     * @param hdl
     */
    inline void set_handle(hc_module_instance_handle_t hdl)
    {
        m_handle = hdl;
    }

    /**
     * @brief Retrieve the key of this node in the chimera overlay
     * @return Key (hash) of this node
     */
    inline std::string address ()
    {
        ChimeraGlobal* chglo =
                reinterpret_cast<ChimeraGlobal*>(m_chimera_state->chimera);
        char keystr[KEY_SIZE];
        key_to_cstr(&(chglo->me->key),keystr, KEY_SIZE);
        return (std::string (keystr));
    }

    /**
     * @brief Stop instance, do some cleanup if necessary
     */
    void shutdown ();

    /**
     * @brief Maps an uri for middleware
     * @param group_uri Group name (URI) to be mapped
     * @return Mapping result
     */
    const hamcast::uri &map(const hamcast::uri &group_uri);

    /**
     * @brief Join a multicast group
     * @param group_uri Group name (URI) to join
     * @return error code
     */
    int join (const hamcast::uri &group_uri);

    /**
     * @brief Send  a LEAVE msg for a given group id
     * @param u Group name (URI) to leave
     * @return error code
     */
    int leave (const hamcast::uri &group_uri);

    /**
     * @brief Send data to multicast group
     * @param group_uri Destination group name (URI)
     * @param payload Multicast data to transmit
     * @param plen Length of payload (multicast data)
     * @return error code
     */
    int send (const hamcast::uri &group_uri,
              const void* payload,
              const size_t &plen, int );

    /**
     * @brief Return list of routing neighbors for this host
     * @return List of routing neighbors
     */
    void neighbor_set(std::vector<hamcast::uri>& result);

    /**
     * @brief Return list of groups joined by this host
     * @return List of joined multicast groups
     */
    void group_set(std::vector<std::pair<hamcast::uri, int> >& result);

    /**
     * @brief Return (list of) parent(s) for a group
     * @param u Group name (URI)
     * @return List of parent nodes for given group
     */
    void parent_set (std::vector<hamcast::uri>&, const hamcast::uri &);

    /**
     * @brief Return list of children for a group
     * @param u Group name (URI)
     * @return List of children for given group
     */
    void children_set (std::vector<hamcast::uri>& result,
                       const hamcast::uri &group_uri);

    /**
     * @brief Check whether this node is forwarder for a group
     * @param u Group name (URI)
     * @return error code
     */
    int designated_host (const hamcast::uri &group_uri);

    /**
     * @brief get_routing_table
     * @param rtable
     */
    void get_routing_table (std::vector<std::string>& rtable);

};

#endif // BIDIRSAM_INSTANCE_HPP
