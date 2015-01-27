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

#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include <vector>
#include "hamcast/hamcast.hpp"

#include "session_fwd.hpp"
#include "tech_module.hpp"

namespace hamcast {

/**
 * @brief This namespace contains the implementation of the HAMcast
 *        middleware.
 */
namespace middleware {

/**
 * @brief An URI mapping consists of a group URI and a pointer to the
 *        corresponding technology module.
 */
typedef std::pair<hamcast::uri, tech_module::ptr> uri_mapping;

/**
 * @brief Implementation of the group address mapping.
 */
std::vector<uri_mapping> possible_mappings(const hamcast::uri& group);

/**
 * @brief Implementation of the service selection.
 */
uri_mapping select_mapping(const hamcast::uri& original_uri,
                           const std::vector<uri_mapping>& choices);

void subscribe_events(const session_ptr& subscriber);

void unsubscribe_events(const session_ptr& subscriber);

} } // namespace hamcast::middleware

#endif // MIDDLEWARE_HPP
