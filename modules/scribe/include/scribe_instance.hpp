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

#ifndef _SCRIBE_INSTANCE_HPP_
#define _SCRIBE_INSTANCE_HPP_

/**
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  */

/* STD includes */
#include <map>
#include <set>
#include <string>
#include <vector>

/* BOOST includes */
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

/* HAMCAST includes */
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

/* CHIMERA includes */
#include "chimera/chimera.h"
#include "chimera/host.h"
#include "chimera/key.h"
#include "chimera/message.h"

/* SCRIBE includes */
#include "scribe.hpp"
#include "scribe_group.hpp"



namespace scribe
{

typedef std::map<std::string, Key> key_map_t;
typedef std::map<std::vector<uint32_t>, hamcast::uri> uri_map_t;
typedef std::map<Key, scribe_group> group_map_t;

class scribe_instance
{
    
private:

    /* scribe internal state */
    hc_event_callback_t             m_event_cb; // event callback
    hc_recv_callback_t              m_recv_cb;  // receive callback
    hc_atomic_msg_size_callback_t   m_msg_cb;   // atomic msg size callback
    hc_module_instance_handle_t     m_handle;
    ChimeraState*                   m_state;
    std::string                     m_name;
    Key                             m_mykey;
    group_map_t                     m_groups;
    boost::mutex                    m_grp_mutex;
    key_map_t                       m_key_cache;
    uri_map_t                       m_uri_cache;
    boost::mutex                    m_map_mutex;
    boost::thread                   m_worker;
    
    /* scribe config params */
    bool                            m_maintenance;
    bool                            m_replicate;

    Key uri_mapping (const hamcast::uri &group_uri);
    hamcast::uri key_mapping (const Key &k, const uint32_t &port);

    /**
      * @brief
      */
    void replicate ();

    /**
     * @brief maintenance
     */
    void maintenance ();
    
public:

    /**
      * @brief Run maintenance
      */
    void do_maintenance ();

    /**
      * @brief Minimal constructor to create scribe instance
      * @param rcb Receive callback function pointer
      * @param ecb Event callback function pointer
      */
    scribe_instance(hc_event_callback_t ecb,
                    hc_recv_callback_t rcb,
                    hc_atomic_msg_size_callback_t mcb) :
            m_event_cb(ecb), m_recv_cb(rcb), m_msg_cb(mcb),
            m_maintenance(0), m_replicate(0)
    {
        m_name = "scribe";
    }

    /**
      * @brief Extended constructor to create a new Scribe instance
      * @param rcb Receive callback function pointer
      * @param ecb Event callback function pointer
      * @param mnt Dis/En-able maintenance, tree repair and stuff
      * @param rpl Dis/En-able state replication to neighbor nodes
      */
    scribe_instance(hc_event_callback_t ecb,
                    hc_recv_callback_t rcb,
                    hc_atomic_msg_size_callback_t mcb,
                    bool mnt, bool rpl) :
            m_event_cb(ecb), m_recv_cb(rcb), m_msg_cb(mcb),
            m_maintenance(mnt), m_replicate(rpl)
    {
        m_name = "scribe";
    }

    /**
     * @brief Gracefully shutdown this scribe instance
     */
    ~scribe_instance ()
    {
        this->shutdown();
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
      * @brief Get the chimera (overlay) state of this scribe instance (node)
      * @return ChimeraState
      */
    inline ChimeraState* get_state()
    { 
        return m_state;
    }

    /**
      * @brief Set/store chimera (overlay) state of this node
      * @param s ChimeraState
      */
    inline void set_state(ChimeraState* s)
    {
        m_state = s;
        ChimeraGlobal* chglo = (ChimeraGlobal*)m_state->chimera;
        m_mykey = chglo->me->key;
    }
    
    /**
     * @brief get_maintenance
     * @return
     */
    inline bool get_maintenance ()
    {
        return m_maintenance;
    }
    
    /**
     * @brief set_maintenance
     * @param m
     */
    inline void set_maintenance (const bool m)
    {
        m_maintenance = m;
    }

    /**
      * @brief Retrieve the key of this node in the chimera overlay
      * @return Key (hash) of this node
      */
    inline std::string get_address ()
    {
        HC_LOG_TRACE("");
        ChimeraGlobal* chglo =
                    reinterpret_cast<ChimeraGlobal*>(m_state->chimera);
        char keystr[KEY_SIZE];
        key_to_cstr(&(chglo->me->key),keystr, KEY_SIZE);
        return (std::string (keystr));
    }

    /**
      * @brief Get name of this scribe interface
      * @return string
      */
    inline std::string get_name ()
    {
        HC_LOG_TRACE("");
        return m_name;
    }


    /**
      * @brief Set name of this scribe interface
      * @param name
      */
    inline void set_name (std::string &name)
    {
        HC_LOG_TRACE("");
        if (!name.empty ())
            m_name = name;
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
    hamcast::uri map (const hamcast::uri &group_uri);

    /**
      * @brief Create/Register a new Scribe group in the network
      * @param group_uri Group name
      * @return error code
      */
    int create (const hamcast::uri &group_uri);

    /**
      * @brief Join a Scribe group
      * @param group_uri Group name (URI) to join
      * @return error code
      */
    int join (const hamcast::uri &group_uri);

    /**
     * @brief Send  a LEAVE msg for a given group id
     * @param group_uri Group name (URI) to leave
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
              const size_t &plen,
              const int);

    /**
      * @brief Return list of routing neighbors for this host
      * @return List of routing neighbors
      */
    void neighbor_set(std::vector<hamcast::uri> &result);

    /**
      * @brief Return list of groups joined by this host
      * @return List of joined multicast groups
      */
    void group_set(std::vector<std::pair<hamcast::uri, int> > &result);

    /**
      * @brief Return (list of) parent(s) for a group
      * @param group_uri Group name (URI)
      * @return List of parent nodes for given group
      */
    void parent_set (std::vector<hamcast::uri> &result,
                     const hamcast::uri &group_uri);

    /**
      * @brief Return list of children for a group
      * @param group_uri Group name (URI)
      * @return List of children for given group
      */
    void children_set (std::vector<hamcast::uri> &result,
                       const hamcast::uri &group_uri);

    /**
      * @brief Check whether this node is forwarder for a group
      * @param group_uri Group name (URI)
      * @return error code
      */
    int designated_host (const hamcast::uri &group_uri);

    /**
      * @brief
      * @param group_uri Group name (URI)
      */
    void send_rp_request (const hamcast::uri &group_uri);

    /* deliver handler */
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_create (Key *, Message *msg);
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_join (Key *key, Message *msg);
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_leave (Key *, Message *msg);
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_multicast (Key*, Message *msg);
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_rp_request (Key*, Message *msg);
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_rp_reply (Key*, Message *msg);
    
    /**
      * @brief
      * @param key
      * @param msg
      */
    void deliver_parent (Key*, Message *msg);
    
    /**
      * @brief
      */
    void forward_join (Key *, Message *msg, ChimeraHost *host);

    /**
     * @brief scribe_instance::get_routing_table
     * @param rtable
     */
    void get_routing_table (std::vector<std::string>& rtable);
};

} // namespace
#endif
