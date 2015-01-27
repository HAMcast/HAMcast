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

#ifndef HAMCAST_UTIL_BUFFERED_SOURCE_HPP
#define HAMCAST_UTIL_BUFFERED_SOURCE_HPP

#include <algorithm>
#include "hamcast/config.hpp"
#include "hamcast/exception.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/util/write_buffer.hpp"

namespace hamcast { namespace util {

/**
 * @brief A decorator class that adds a read buffer
 *        to an otherwise unbuffered source.
 *
 * @param block_size buffered_source allocates storage always in chunks
 *                   to minimize resizing of the internal buffer.
 *
 * @param maximum_size The maximum number of bytes
 *                     buffered_source should allocate.
 */
template<size_t block_size = default_block_size,
         size_t maximum_size = default_max_write_buffer_size>
class buffered_source : public source
{

    // decorated sink
    intrusive_ptr<source> m_decorated;
    // current read position
    boost::uint32_t m_rd_pos;
    // first buffer
    write_buffer<block_size, maximum_size> m_buf1;
    // second buffer
    write_buffer<block_size, maximum_size> m_buf2;
    // pointer to the currently used buffer
    write_buffer<block_size, maximum_size>* m_data;

    // swap buffers if we consumed the half of the buffer
    void lazy_swap_buffers()
    {
        size_t r = m_data->capacity();
        if (m_rd_pos > (r / 2))
        {
            if (m_data == &m_buf1)
            {
                m_buf2.clear();
                m_buf2.write(m_buf1.size() - m_rd_pos,
                             m_buf1.data() + m_rd_pos);
                m_data = &m_buf2;
            }
            else if (m_data == &m_buf2)
            {
                m_buf1.clear();
                m_buf1.write(m_buf2.size() - m_rd_pos,
                             m_buf2.data() + m_rd_pos);
                m_data = &m_buf1;
            }
            else
            {
                throw std::logic_error("m_data is invalid");
            }
            m_rd_pos = 0;
        }
    }

    void try_reset_buffer()
    {
        if (m_rd_pos >= m_data->size())
        {
            m_rd_pos = 0;
            m_data->clear();
        }
    }

    // @pre (buf_size + m_rd_pos) <= m_data->size()
    void do_copy(size_t buf_size, void* buf)
    {
        HC_REQUIRE((buf_size + m_rd_pos) <= m_data->size());
        (void) memcpy(buf, m_data->data() + m_rd_pos, buf_size);
        m_rd_pos += buf_size;
        lazy_swap_buffers();
    }

    // get the number of available bytes from m_data
    size_t available()
    {
        try_reset_buffer();
        if (m_rd_pos >= m_data->size()) return 0;
        else return m_data->size() - m_rd_pos;
    }

 public:

    virtual bool wait_for_data(long sec, long usec)
    {
        return (available() > 0) ? true : m_decorated->wait_for_data(sec,
                                                                     usec);
    }

    /**
     * @brief Create a buffered source that decorates @p underlying_source.
     * @param underlying_source The original source.
     * @pre @p underlying_source is valid
     */
    buffered_source(const intrusive_ptr<source>& underlying_source)
        : m_decorated(underlying_source), m_rd_pos(0)
    {
        HC_REQUIRE(underlying_source.get() != 0);
        m_buf1.reserve(2 * block_size);
        m_buf2.reserve(2 * block_size);
        m_data = &m_buf1;
    }

    // overrides source::read_some
    virtual size_t read_some(size_t buf_size, void* buf)
    {
        // stupid-user-test
        if (buf_size == 0) return 0;
        if (available() == 0)
        {
            // we're facing an empty buffer
            m_data->append_some_from(*m_decorated, m_data->remaining());
            size_t rd_size = std::min(available(), buf_size);
            if (rd_size > 0)
            {
                do_copy(rd_size, buf);
            }
            return rd_size;
        }
        else
        {
            // buffer not empty, read from the internal buffer only
            size_t rd_size = std::min(available(), buf_size);
            do_copy(rd_size, buf);
            return rd_size;
        }
    }

    // overrides source::read
    virtual void read(size_t buf_size, void* buf)
    {
        // read some data until the buffer contains at least buf_size bytes
        while (available() < buf_size)
        {
            // try to read as much as possible without resizing m_data
            size_t rd_size = m_data->remaining();
            // but read more than remaining() if needed
            if (rd_size < buf_size) rd_size = buf_size;
            size_t written = m_data->append_some_from(*m_decorated,rd_size);
            if (written == 0)
            {
                throw std::ios_base::failure("Can't read from sorce");
            }
        }
        do_copy(buf_size, buf);
    }

    // overrides closeable::close
    virtual void close()
    {
        m_decorated->close();
    }

    // overrides closeable::closed
    virtual bool closed() const
    {
        return m_decorated->closed();
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_BUFFERED_SOURCE_HPP
