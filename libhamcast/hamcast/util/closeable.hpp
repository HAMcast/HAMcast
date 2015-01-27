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

#ifndef HAMCAST_UTIL_CLOSEABLE_HPP
#define HAMCAST_UTIL_CLOSEABLE_HPP

#include "hamcast/ref_counted.hpp"

namespace hamcast { namespace util {

/**
 * @brief Describes a closeable input and/or output channel.
 */
struct closeable : public ref_counted
{
    /**
     * @brief Close the data channel.
     */
    virtual void close() = 0;

    /**
     * @brief Check if the data channel is closed.
     * @returns <code>true</code> if <code>this</code> is closed for
     *         read/write operations; otherwise <code>false</code>.
     */
    virtual bool closed() const = 0;
};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_CLOSEABLE_HPP
