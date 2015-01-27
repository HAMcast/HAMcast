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

#include <iostream>

#include "hamcast/hamcast_logging.h"

#include "bidirsam_mft.hpp"

void bidirsam_mft::insert_entry(const Key &entry)
{
    HC_LOG_TRACE("");
    std::map<Key, int>::iterator it;
    it = m_mft.find(entry);
    // If prefix allready in list increment retransmit count else create new Entry
    if(it != m_mft.end())
    {
        if (it->second < c_default_count)
        {
            it->second++;
        }
    }
    else{
        m_mft.insert(std::pair<Key, int>(entry,c_default_count));
    }
}

void bidirsam_mft::delete_entry(const Key &k)
{
    HC_LOG_TRACE("");
    std::map<Key, int>::iterator it = m_mft.find (k);
    if (it != m_mft.end())
        m_mft.erase (it);
}

void bidirsam_mft::decrement_entry(const Key &k)
{
    std::map<Key, int>::iterator it;
    it = m_mft.find (k);
    if(it != m_mft.end()){
        it->second--;
        if(it->second <= 0)
        {
            m_mft.erase (it);
            //delete_entry(k);
        }
    }
}

const Key bidirsam_mft::get_lcp(const Key &k)
{
    HC_LOG_TRACE("");
    // Find the Key with the longest common prefix with Key k
    std::map<Key, int>::iterator it;
    int index = 0;
    Key res;
    memset(&res, 0, sizeof(Key));
    for ( it=m_mft.begin() ; it != m_mft.end(); ++it ) {
        Key g = it->first;
        int tmp = key_index_int(k,g);
        if(tmp >= index) {
            index = tmp;
            key_assign (&res, g);
            //res = g;
        }
    }
    return res;
}
