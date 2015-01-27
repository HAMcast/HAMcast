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

#ifndef HAMCAST_UTIL_BUFFERED_SINK_HPP
#define HAMCAST_UTIL_BUFFERED_SINK_HPP

#include <iostream>

#include "hamcast/config.hpp"
#include "hamcast/exception.hpp"
#include "hamcast/util/sink.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/util/write_buffer.hpp"

namespace hamcast { namespace util {

/**
 * @brief A decorator class that adds a write buffer
 *        to an otherwise unbuffered sink.
 *
 * @p block_size: buffered_sink allocates storage always in chunks to minimize
 *               resizing of the internal buffer.
 *
 * @p maximum_size: The maximum number of bytes buffered_sink should allocate.
 */
template<size_t block_size = default_block_size,
         size_t maximum_size = default_max_write_buffer_size>
class buffered_sink : public sink
{

    intrusive_ptr<sink> m_decorated;
    write_buffer<block_size, maximum_size> m_buf;

    int m_flush_hint;

 public:

    /**
     * @brief Create a buffered sink that decorates @p underlying_sink.
     * @param underlying_sink The original sink.
     * @pre @p underlying_sink is valid
     */
    buffered_sink(const intrusive_ptr<sink>& underlying_sink)
        : m_decorated(underlying_sink)
    {
        HC_REQUIRE(underlying_sink.get() != 0);
        m_flush_hint = underlying_sink->flush_hint();
        if (m_flush_hint < 100)
        {
            // ignore very small flush hints
            m_flush_hint = 0;
        }
    }

    // overrides sink::write
    virtual void write(size_t buf_size, const void* buf)
    {
        size_t new_size = m_buf.size() + buf_size;
        if (new_size > maximum_size)
        {
            flush();
        }
        else if (m_flush_hint > 0)
        {
            if (new_size > static_cast<size_t>(m_flush_hint))
            {
                flush();
            }
        }
        m_buf.write(buf_size, buf);
    }

    // overrides sink::flush()
    virtual void flush()
    {
        if (!m_buf.empty())
        {
            m_decorated->write(m_buf.size(), m_buf.data());
            m_decorated->flush();
            m_buf.clear();
        }
    }

    virtual int flush_hint() const
    {
        return m_flush_hint;
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

#endif // HAMCAST_UTIL_BUFFERED_SINK_HPP
