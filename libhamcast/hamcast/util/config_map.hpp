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


#ifndef HAMCAST_CONFIG_MAP_HPP
#define HAMCAST_CONFIG_MAP_HPP

#include <map>
#include <string>
#include <stdexcept>

namespace hamcast { namespace util {

class config_map
{

 public:

    typedef std::string                    key_type;
    typedef std::map<key_type,key_type>    mapped_type;
    typedef std::map<key_type,mapped_type> container_type;
    typedef container_type::const_iterator const_iterator;

    /**
     * @throws runtime_error
     */
    void read_ini(const key_type& filename);

    inline bool has_group(const key_type& group) const {
        return m_data.count(group) > 0;
    }

    inline const_iterator begin() const { return m_data.begin(); }

    inline const_iterator end() const { return m_data.end(); }

    inline const_iterator find(const key_type& group) const {
        return m_data.find(group);
    }

    inline const key_type& get(const_iterator group, const key_type& key) const {
        mapped_type::const_iterator j = group->second.find(key);
        if (j != group->second.end()) return j->second;
        return m_empty;
    }

    inline const key_type& get(const key_type& group, const key_type& key) const {
        container_type::const_iterator i = m_data.find(group);
        if (i != m_data.end()) return get(i, key);
        return m_empty;
    }

    /**
     * @throws range_error if @p group is unknown
     */
    inline const mapped_type& operator[](const key_type& group) const {
        container_type::const_iterator i = m_data.find(group);
        if (i == m_data.end()) throw std::range_error("unknown group: " + group);
        return i->second;
    }

 private:

    key_type m_empty;
    container_type m_data;

};

} } // namespace hamcast::util

#endif // HAMCAST_CONFIG_MAP_HPP
