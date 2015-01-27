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

#ifndef HAMCAST_UTIL_DESERIALIZER_HPP
#define HAMCAST_UTIL_DESERIALIZER_HPP

#include "hamcast/util/source.hpp"

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

namespace hamcast { namespace util {

/**
 * @brief Deserializes objects from a given source.
 */
class deserializer : public boost::noncopyable
{

    intrusive_ptr<source> m_src;

 public:

    /**
     * @brief Create a deserializer with no data source.
     */
    deserializer();

    /**
     * @brief Create a deserializer that reads from @p src_ptr.
     * @param src_ptr The used data source.
     * @pre <code>src_ptr.get() != NULL</code>.
     */
    explicit deserializer(const intrusive_ptr<source>& src_ptr);

    ~deserializer();

    /**
     * @brief Change the used data source.
     * @param new_source The new data source for <code>this</code>.
     */
    void reset(const intrusive_ptr<source>& new_source);


    /**
     * @brief Read @p buf_size bytes from the source
     *        and store them in @p buf.
     * @param buf_size Size of @p buf in bytes.
     * @param buf C-buffer to store the result of this operation.
     * @pre <code>buf_size > 0 && buf != NULL</code>.
     */
    inline void read(size_t buf_size, void* buf)
    {
        if (m_src) m_src->read(buf_size, buf);
    }

};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_DESERIALIZER_HPP
