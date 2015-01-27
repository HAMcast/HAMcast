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

#ifndef HAMCAST_INTERFACE_PROPERTY_HPP
#define HAMCAST_INTERFACE_PROPERTY_HPP

#include <string>
#include "hamcast/interface_id.hpp"

namespace hamcast {

/**
 * @brief Holds context informations about an (middleware) interface.
 */
struct interface_property
{
    /**
     * @brief The internal used ID of this interface.
     */
    interface_id id;

    /**
     * @brief A human-readable name (e.g. "eth0", "lo", ...).
     */
    std::string name;

    /**
     * @brief A technology specific address, encoded as human-readable string.
     */
    std::string address;

    /**
     * @brief The name of the used technology (e.g. "ip", "overlay", ...).
     */
    std::string technology;
};

} // namespace hamcast

#endif // HAMCAST_INTERFACE_PROPERTY_HPP
