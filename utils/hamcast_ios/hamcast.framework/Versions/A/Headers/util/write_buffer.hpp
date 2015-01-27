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

#ifndef HAMCAST_UTIL_WRITE_BUFFER_HPP
#define HAMCAST_UTIL_WRITE_BUFFER_HPP

#include <cstring>

#include "hamcast/config.hpp"
#include "hamcast/exception.hpp"
#include "hamcast/util/sink.hpp"
#include "hamcast/hamcast_logging.h"

namespace hamcast { namespace util {

/**
 * @brief A mutable (growing) buffer that could be used as a
 *        {@link hamcast::util::sink sink}.
 *
 * The write_buffer class provides a representation of a mutable output
 * buffer that automatically grows up to @p maximum_size bytes.
 *
 * @p block_size {@link buffered_sink} allocates storage always in chunks to
 *               minimize resizing of the internal buffer.
 * @p maximum_size The maximum number of bytes
 *                 {@link buffered_sink} is allowed to allocate.
 * @warning The buffer is <b>not</b> thread safe <i>except</i>.
 */
template<size_t block_size = default_block_size,
         size_t maximum_size = default_max_write_buffer_size>
class write_buffer : public sink
{

    char* m_data;
    size_t m_wr_pos;
    size_t m_reserved;

    void lazy_resize(size_t new_min_size)
    {
        if (m_reserved < new_min_size)
        {
            // always allocate in block_size steps
            if ((new_min_size % block_size) == 0)
            {
                // new_min_size is a valid size
                m_reserved = new_min_size;
            }
            else
            {
                // get the nearest (larger) valid block size
                m_reserved = ((new_min_size / block_size) + 1) * block_size;
            }
            // but do never allocate more than maximum_size bytes
            if (m_reserved > maximum_size)
            {
                std::string err_msg = "Buffer exceeds maximum size";
                HC_LOG_FATAL(err_msg);
                throw std::ios_base::failure(err_msg);
            }
            else
            {
                char* new_data = new char[m_reserved];
                if (m_data)
                {
                    if (m_wr_pos > 0) (void) memcpy(new_data, m_data, m_wr_pos);
                    delete[] m_data;
                }
                m_data = new_data;
            }
        }
    }

 public:

    typedef intrusive_ptr<write_buffer> ptr;

    /**
     * @brief Create a buffered sink that decorates @p underlying_sink.
     * @pre @p underlying_sink is valid
     */
    write_buffer() : m_data(0), m_wr_pos(0), m_reserved(0) { }

    /**
     * @brief Deletes the decorated sink.
     */
    ~write_buffer()
    {
        if (m_data) delete[] m_data;
    }

    /**
     * @brief Increase {@link capacity()} to at least @p arg bytes.
     * @param arg The new minimum size of <code>this</code>.
     */
    void reserve(size_t arg) { if (arg > m_reserved) lazy_resize(arg); }

    /**
     * @brief Get the size of the allocated storage.
     * @returns The number of bytes this buffer could write wihtout
     *         re-allocation of memory storage.
     */
    inline size_t capacity() const { return m_reserved; }

    /**
     * @brief Get the number of remaining (unwritten) bytes in
     *        the allocated storage.
     * @returns <code>{@link capacity()} - {@link size()}</code>.
     */
    inline size_t remaining() const { return m_reserved - m_wr_pos; }

    /**
     * @brief Get the number of currently used bytes.
     * @returns The number of written bytes in the internal C-buffer.
     */
    inline size_t size() const { return m_wr_pos; }

    /**
     * @brief Get the written data.
     * @returns The internal C-buffer.
     */
    inline const char* data() const { return m_data; }

    inline bool empty() const { return size() == 0; }

    template<class Source>
    size_t append_some_from(Source& src, size_t buf_size)
    {
        lazy_resize(size() + buf_size);
        size_t written = src.read_some(buf_size, m_data + m_wr_pos);
        m_wr_pos += written;
        return written;
    }

    void erase(size_t position)
    {
        erase(position, position + 1);
    }

    // erase [from, to) from the buffer (including from but not to)
    void erase(size_t from, size_t to)
    {
        if (from == to)
            return;

        HC_REQUIRE(from < to);
        if (from >= m_wr_pos)
        {
            // nothing to do
            return;
        }
        else if (to >= m_wr_pos)
        {
            // erase a block at the end
            m_wr_pos = from;
        }
        else
        {
            char* dst = m_data + from;
            char* src = m_data + to;
            char* end = m_data + m_wr_pos;
            while (src != end)
            {
                *dst++ = *src++;
            }
            m_wr_pos -= (to - from);
        }
    }

    /**
     * @brief Writes @p buf_size bytes from @p buf to the internal buffer.
     * @param buf_size The size of @p buf in bytes.
     * @param buf A pointer to a C-buffer that contains the data to write.
     * @pre <code>buf_size > 0 && buf != NULL</code>.
     */
    void write(size_t buf_size, const void* buf)
    {
        lazy_resize(buf_size + m_wr_pos);
        (void) memcpy(m_data + m_wr_pos, buf, buf_size);
        m_wr_pos += buf_size;
    }

    /**
     * @brief Get the internal buffer and reset the buffer afterwards.
     * @returns <code>std::make_pair({@link size()}, {@link data()})</code>
     */
    std::pair<size_t, void*> take()
    {
        size_t s = size();
        void* ptr = m_data;
        clear();
        m_reserved = 0;
        m_data = 0;
        return std::make_pair(s, ptr);
    }

    void reset()
    {
        if (m_data) delete[] m_data;
        clear();
        m_data = 0;
        m_reserved = 0;
    }

    /**
     * @brief Clear the internal buffer.
     */
    void clear()
    {
        m_wr_pos = 0;
    }

    virtual void flush() { }

    virtual void close() { }

    virtual bool closed() const { return false; }

};

} }

#endif // HAMCAST_UTIL_WRITE_BUFFER_HPP
