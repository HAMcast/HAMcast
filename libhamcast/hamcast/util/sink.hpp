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

#ifndef HAMCAST_UTIL_SINK_HPP
#define HAMCAST_UTIL_SINK_HPP

#include <boost/cstdint.hpp>

#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/util/closeable.hpp"
#include "hamcast/util/native_socket.hpp"

namespace hamcast { namespace util {

/**
 * @brief Describes an abstract (binary) data sink.
 */
struct sink : public virtual closeable
{

    typedef intrusive_ptr<sink> ptr;

    /**
     * @brief Writes @p buf_size bytes from @p buf to the sink.
     * @param buf_size The number of bytes that should be written from @p buf.
     * @param buf The buffer.
     * @throws ios_base::failure on errors (e.g. if the sink is closed)
     */
    virtual void write(size_t buf_size, const void* buf) = 0;

    /**
     * @brief Force the data sink to flush any buffers.
     * @throws Any exception that might be thrown by {@link write()}.
     */
    virtual void flush() = 0;

    /**
     * @brief Return the optimal block size for write operations.
     * @returns A value > 0 if this device has an optimal block size;
     *         otherwise (default implementation) 0 is returned.
     */
    virtual int flush_hint() const;

    virtual native_socket write_handle() const = 0;

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_SINK_HPP
