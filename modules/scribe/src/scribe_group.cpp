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

#include <map>
#include <list>

#include <boost/thread.hpp>

#include "chimera/host.h"
#include "chimera/key.h"

#include "hamcast/hamcast_logging.h"

#include "scribe.hpp"
#include "scribe_group.hpp"

using std::map;
using std::pair;
using std::string;
using std::list;
using hamcast::uri;

using namespace scribe;

scribe_group::scribe_group (const uri &group_uri) : m_parent(0), m_rp(0), m_count(0)
{
    HC_LOG_TRACE("");
    set_uri(group_uri);
}

void scribe_group::add_child (ChimeraHost *c)
{
    HC_LOG_TRACE("");
    children_map_t::iterator it = m_children.find(c->key);
    if (it != m_children.end()) {
        HC_LOG_DEBUG ("Child found ...");
        ++(it->second);
    }
    else {
        HC_LOG_DEBUG ("Child not found ...");
        scribe_node child(c);
        m_children.insert(pair<Key, scribe_node>(c->key, child));
    }
}

void scribe_group::del_child (const Key &child_key)
{
    HC_LOG_TRACE("DELETE CHILD");
    children_map_t::iterator it = m_children.find(child_key);
    if (it != m_children.end()) {
        HC_LOG_DEBUG("Child found, delete!");
        m_children.erase(it);
    }
    else {
        HC_LOG_DEBUG ("Child not found!");
    }
}

bool scribe_group::has_child (const Key &child_key)
{
    HC_LOG_TRACE("");
    children_map_t::iterator it = m_children.find(child_key);
    if (it != m_children.end()) {
        HC_LOG_DEBUG ("Child found ...");
        return true;
    }
    HC_LOG_DEBUG ("Child not found ...");
    return false;
}
