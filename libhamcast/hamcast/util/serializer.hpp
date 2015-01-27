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

#ifndef HAMCAST_UTIL_SERIALIZER_HPP
#define HAMCAST_UTIL_SERIALIZER_HPP

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>

#include "hamcast/util/sink.hpp"

namespace hamcast { namespace util {

/**
 * @brief Serializes objects to a given sink.
 */
class serializer : boost::noncopyable
{

    intrusive_ptr<sink> m_sink;

 public:

    /**
     * @brief Create a serializer with no data sink.
     */
    serializer();

    /**
     * @brief Create a serializer that writes from @p sink_ptr.
     * @param sink_ptr The used data sink.
     * @pre <code>sink_ptr.get() != NULL</code>.
     */
    explicit serializer(const intrusive_ptr<sink>& sink_ptr);

    ~serializer();

    /**
     * @brief Change the used data sink.
     * @param new_sink The new data sink for <code>this</code>.
     */
    void reset(const intrusive_ptr<sink>& new_sink);

    /**
     * @brief Write @p buf_size bytes from @p buf to the sink.
     * @param buf_size Size of @p buf in bytes.
     * @param buf C-buffer that contains the outgoing bytes.
     * @pre <code>buf_size > 0 && buf != NULL</code>.
     */
    inline void write(size_t buf_size, const void* buf)
    {
        if (m_sink) m_sink->write(buf_size, buf);
    }

    /**
     * @brief Flush all internal buffers of the used data sink.
     */
    inline void flush()
    {
        if (m_sink) m_sink->flush();
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_SERIALIZER_HPP
