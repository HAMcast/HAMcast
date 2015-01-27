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

#ifndef HAMCAST_UTIL_ID_GENERATOR_HPP
#define HAMCAST_UTIL_ID_GENERATOR_HPP

#include <boost/thread/mutex.hpp>

namespace hamcast { namespace util {

/**
 * @brief Utility class that generates continuous ID numbers.
 */
template<typename IdType>
class id_generator
{

    IdType m_val;

 public:

    typedef IdType id_type;

 public:

    id_generator(id_type first_id = id_type()) : m_val(first_id) { }

    /**
     * @brief Get the next ID.
     * @returns A new, unique ID.
     */
    id_type next()
    {
        return m_val++;
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_ID_GENERATOR_HPP
