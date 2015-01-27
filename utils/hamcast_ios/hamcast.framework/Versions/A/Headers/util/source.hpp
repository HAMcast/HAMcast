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

#ifndef HAMCAST_UTIL_SOURCE_HPP
#define HAMCAST_UTIL_SOURCE_HPP

#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/util/closeable.hpp"

namespace hamcast { namespace util {

/**
 * @brief Describes an abstract (binary) data source.
 */
struct source : public virtual closeable
{

    typedef intrusive_ptr<source> ptr;

    /**
     * @brief Wait until this source becomes "ready" or until a timeout
     *        occurs.
     * @param seconds The seconds part of the maximum block time.
     * @param microseconds The microseconds part of the maximum block time.
     * @returns <code>true</code> if there is data to read;
     *         otherwise <code>false</code>
     * @pre lock_read() was called
     * @throws std::ios_base::failure if the source becomes closed
     */
    virtual bool wait_for_data(long seconds, long microseconds) = 0;

    /**
     * @brief Read up to @p buf_size bytes from the data source and store
     *        the bytes in @p buf. A closed source always returns 0.
     * @param buf_size The size of @p buf.
     * @param buf The target buffer.
     * @returns The number of readed bytes (might be < @p buf_size).
     * @pre lock_read() was called
     * @throws std::ios_base::failure on errors during read.
     */
    virtual size_t read_some(size_t buf_size, void* buf) = 0;

    /**
     * @brief Read @p buf_size bytes from the data source. This function
     *        blocks until enough bytes are available and throws and exception
     *        if the source becomes unreadable before @p buf_size bytes
     *        are read.
     * @param buf_size The size of @p buf.
     * @param buf The target buffer.
     * @pre lock_read() was called
     * @throws std::ios_base::failure on errors during read
     *                                (e.g. if the source is closed)
     */
    virtual void read(size_t buf_size, void* buf) = 0;

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_SOURCE_HPP
