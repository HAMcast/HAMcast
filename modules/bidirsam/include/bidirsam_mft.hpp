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

#ifndef BIDIRSAM_MFT_HPP
#define BIDIRSAM_MFT_HPP
#include <set>
#include <map>

#include "chimera/key.h"
#include "bidirsam.hpp"


class bidirsam_mft
{

private:

    std::map<Key,int> m_mft;

public:

    /**
     * @brief Insert entry at row, col
     */
    void insert_entry(const Key &entry);

    /**
     * @brief Delete entry at row, col
     */
    void delete_entry(const Key &k);

    /**
      * @brief Decrement retransmit count for entry k
      * @param Key k
      */
    void decrement_entry(const Key &k);

    /**
     * @brief Get the Key with the longest common prefix with Key k
     * @param k
     * @return
     */
    const Key get_lcp(const Key &k);

    /**
     * @brief get_mft
     * @return
     */
    inline std::map<Key,int>& get_mft()
    {
        return m_mft;
    }
};

#endif // BIDIRSAM_MFT_HPP
