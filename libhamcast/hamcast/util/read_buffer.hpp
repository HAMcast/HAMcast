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

#ifndef HAMCAST_UTIL_READ_BUFFER_HPP
#define HAMCAST_UTIL_READ_BUFFER_HPP

#include <ios>
#include <limits>
#include <cstring>
#include <stdexcept>

#include "hamcast/util/source.hpp"
#include "hamcast/util/const_buffer.hpp"

namespace hamcast { namespace util {

/**
 * @brief An immutable buffer that could be used as a
 *        {@link hamcast::util::source source}.
 *
 * The read_buffer class provides a {@link hamcast::util::source source}
 * implementation that reads from a
 * {@link hamcast::util::const_buffer const_buffer}.
 */
class read_buffer : public source
{

    // read position
    const_buffer::size_type m_rd_pos;
    // buffer to read from
    const_buffer m_buf;

    // copy from current read position and increase it afterwards
    void do_copy(size_t buf_size, void* buf);

 public:

    typedef const_buffer::size_type size_type;

    inline read_buffer() : m_rd_pos(0) { }

    /**
     * @brief Swaps @p cb with the internal buffer.
     * @param cb Will be swapped with the internal {@link const_buffer}.
     */
    inline read_buffer(const_buffer& cb) : m_rd_pos(0)
    {
        m_buf.swap(cb);
    }

    /**
     * @brief Constructs the internal const_buffer
     *        with @p buf_size and @p buf.
     * @param buf_size The size of @p buf.
     * @param buf The const C-buffer.
     * @see const_buffer::const_buffer(size_type, const void*)
     * @warning @p buf must stay valid for the whole
     *          lifetime of this object.
     */
    inline read_buffer(size_type buf_size, const void* buf)
        : m_rd_pos(0), m_buf(buf_size, buf)
    {
    }

    /**
     * @brief Constructs the internal const_buffer with
     *        @p buf_size, @p buf and @p ownership.
     * @param buf_size The size of @p buf.
     * @param buf The C-buffer.
     * @param ownership Denotes if this object should
     *        take ownership of @p buf.
     * @see const_buffer::const_buffer(size_type, void*, bool)
     * @warning @p buf must stay valid for the whole lifetime of this
     *          object if <code>@p ownership == false</code>.
     */
    inline read_buffer(size_type buf_size, void* buf, bool ownership)
        : m_rd_pos(0), m_buf(buf_size, buf, ownership)
    {
    }

    /**
     * @brief Constructs the internal const_buffer with
     *        @p buf and @p ownership.
     * @param buf The C-buffer and its size as std::pair.
     * @param ownership Denotes if this object should take
     *                  ownership of @p buf.
     * @see const_buffer::const_buffer(size_type, void*, bool)
     * @param ownership Denotes if this object should take
     *                  ownership of @p buf.
     */
    inline read_buffer(std::pair<size_type, void*> buf, bool ownership)
        : m_rd_pos(0), m_buf(buf.first, buf.second, ownership)
    {
    }

    bool wait_for_data(long seconds, long microseconds);

    // overrides source::read_some
    size_t read_some(size_t buf_size, void* buf);

    inline void reset()
    {
        m_rd_pos = 0;
        m_buf.reset();
    }

    void reset(size_type buf_size, const void* buf)
    {
        m_rd_pos = 0;
        m_buf.reset(buf_size, buf);
    }

    void reset(size_type buf_size, void* buf, bool ownership)
    {
        m_rd_pos = 0;
        m_buf.reset(buf_size, buf, ownership);
    }

    /**
     * @brief Get the number of available (unread) bytes.
     * @returns The number of bytes that could be read before
     *         <code>std::ios_base::failure</code> is thrown.
     */
    inline size_t available() const
    {
        size_t bsize = m_buf.size();
        return (bsize > m_rd_pos) ? bsize - m_rd_pos : 0;
    }

    inline size_type read_position() const
    {
        return m_rd_pos;
    }

    inline void skip(size_t num_bytes)
    {
        m_rd_pos += static_cast<const_buffer::size_type>(num_bytes);
    }

    // pointer to the current read position
    inline const void* data() const
    {
        return reinterpret_cast<const char*>(m_buf.data()) + m_rd_pos;
    }

    // @note if <code>@p buf == NULL</code> then this call will only change
    //       the read position
    // overrides source::lock_read
    virtual void read(size_t buf_size, void* buf);

    // overrides source::close
    virtual void close();

    // overrides source::closed
    virtual bool closed() const;

    // overrides source::read_handle
    native_socket read_handle() const;

    bool has_buffered_data() const;

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_READ_BUFFER_HPP
