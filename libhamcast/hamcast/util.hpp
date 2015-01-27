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

#ifndef HAMCAST_UTIL_HPP
#define HAMCAST_UTIL_HPP

#include "hamcast/util/buffered_sink.hpp"
#include "hamcast/util/buffered_source.hpp"
#include "hamcast/util/closeable.hpp"
#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/deserializer.hpp"
#include "hamcast/util/if_else_t.hpp"
#include "hamcast/util/mock_mutex.hpp"
#include "hamcast/util/read_buffer.hpp"
#include "hamcast/util/serializer.hpp"
#include "hamcast/util/sink.hpp"
#include "hamcast/util/socket_io.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/util/unit.hpp"
#include "hamcast/util/write_buffer.hpp"

namespace hamcast {

/**
 * @namespace hamcast::util
 * @brief This namespace contains utility classes.
 */
namespace util { }

} // namespace hamcast

#endif // HAMCAST_UTIL_HPP
