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

#ifndef _SCRIBE_NODE_HPP_
#define _SCRIBE_NODE_HPP_

/**
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  */

#include <map>

#include "hamcast/uri.hpp"

#include "chimera/host.h"
#include "chimera/key.h"

#include "scribe.hpp"

namespace scribe 
{

class scribe_node
{

private:

    ChimeraHost*    m_host;     // chimera host representation
    int             m_count;    // robustness counter for maintenance

public:

    /**
     * @brief Constructor of scribe_node
     * @param host: chimera host
     * @param count: initial value
     */
    scribe_node (ChimeraHost* h, const int& c) : m_host(h), m_count (c)
    {}
    
    /**
     * @brief Constructor of scribe_node
     * @param host: chimera host
     */
    scribe_node (ChimeraHost* h) : m_host(h), m_count (c_default_count)
    {}
    
    /**
     * @brief Get hash key of node
     * @return hash id
     */
    inline const Key& get_key () const { return m_host->key; }
    
    /**
     * @brief Get chimera host
     * @return chimera host
     */
    inline ChimeraHost* get_host () { return m_host; }
    
    /**
     * @brief Get current value of robustness counter
     * @return counter
     */
    inline int get_count () { return m_count; }
    
    /**
     * @brief Reset robustness counter to default value
     */
    inline void reset_count () { m_count = c_default_count; }
    
    /**
     * @brief Increase robustness counter
     * @return
     */
    scribe_node& operator++() 
    {
        if (m_count < c_default_count)
            ++m_count;
        return *this;
    }
    
    /**
     * @brief Decrease robustness counter
     * @return
     */
    scribe_node& operator--()
    {
        if (m_count > 0)
            --m_count;
        return *this;
    }

};

} /* namespace scribe */

#endif /* _SCRIBE_GROUP_HPP_ */
