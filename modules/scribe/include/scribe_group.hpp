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

#ifndef _SCRIBE_GROUP_HPP_
#define _SCRIBE_GROUP_HPP_

/**
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  */

#include <list>
#include <map>

#include <boost/thread.hpp>

#include "hamcast/uri.hpp"

#include "chimera/host.h"
#include "chimera/key.h"

#include "scribe.hpp"
#include "scribe_node.hpp"

namespace scribe 
{

typedef std::map<Key, scribe_node> children_map_t;
class scribe_group 
{

private:

    Key             m_gid;      // hash of group name
    hamcast::uri    m_uri;      // URI of group name (without port)
    ChimeraHost*    m_parent;   // parent in scribe tree
    ChimeraHost*    m_rp;       // scribe rendezvous point
    children_map_t  m_children; // children in scribe tree
    int             m_count;    // robustness counter

public:

/* con/destructors */

    /**
     * @brief Create scribe group from URI
     * @param group URI
     */
    scribe_group (const hamcast::uri &group_uri);

    /**
     * @brief Create scribe group from hash key
     * @param hash key
     */
    scribe_group (const Key &group_key) : m_gid(group_key), m_parent(0), m_rp(0), m_count(0)
    {
    }

/* misc functions */
    
    /**
     *  @brief Add new child host to the groups child list
     *  @param child: Host to add to child list
     */
    void add_child (ChimeraHost* child);
       
    /**
     *  @brief Delete a (existing) child host from the groups child list
     *  @param k: Key of the host to delete
     */
    void del_child (const Key& child_key);

    /**
     *  @brief Check if child with the given key exists.
     *  @param k: Key of the host to find
     */
    bool has_child (const Key& child_key);

    /**
     *  @brief Get the number of children for this group
     *  @return Number of children
     */
    inline size_t children ()
    {
        HC_LOG_TRACE("");
        return m_children.size();
    }

    /**
     *  @brief Set heartbeat
     */
    inline void heartbeat ()
    {
        HC_LOG_TRACE("");
        m_count = 1;
    }

/* Getter and Setter */

    /**
     *  @brief Set parent host 
     *  @param p: Host to set as parent for this group
     */
    inline void set_parent (ChimeraHost* p) 
    {
        HC_LOG_TRACE("");
        m_parent = p;
    }

    /**
     *  @brief Get parent host for this group
     *  @return ChimeraHost parent node
     */
    inline ChimeraHost* get_parent ()
    {
        HC_LOG_TRACE("");
        return m_parent;
    }

    /**
     *  @brief Set rendez-vous point host
     *  @param rp: Host to set as rendez-vous point for this group
     */
    inline void set_rp (ChimeraHost* rp)
    {
        HC_LOG_TRACE("");
        if (rp != NULL) {
            m_rp = rp;
        }
        else {
            HC_LOG_WARN("Invalid Rendezvous-Point");
        }
    }

    /**
     *  @brief Return rendezvous point for this group
     *  @return rendezvous point ChimeraHost
     */
    inline ChimeraHost* get_rp ()
    {
        HC_LOG_TRACE("");
        return m_rp;
    }

    /**
     *  @brief Get group id
     *  @return Key group id 
     */
    inline const Key get_gid () const
    {
        HC_LOG_TRACE("");
        return m_gid;
    }

    /**
     *  @brief Set group uri
     *  @param group_uri Group URI
     */
    inline void set_uri(const hamcast::uri &group_uri)
    {
        HC_LOG_TRACE("");
        if (!group_uri.empty ()) {
            m_uri = group_uri;
        }
        else {
            HC_LOG_WARN("Invalid or empty group uri");
        }
    }

    /**
     *  @brief Get group uri
     *  @return group uri
     */
    inline const hamcast::uri get_uri () const
    {
        HC_LOG_TRACE("");
        return m_uri;
    }

    /**
     *  @brief List of children for this group
     *  @return children map
     */    
    inline std::map<Key, scribe_node>& get_children ()
    {
        HC_LOG_TRACE("");
        return m_children;
    }

};

} /* namespace scribe */

#endif /* _SCRIBE_GROUP_HPP_ */
