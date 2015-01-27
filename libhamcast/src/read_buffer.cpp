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

#include "hamcast/util/read_buffer.hpp"

namespace hamcast { namespace util {

void read_buffer::do_copy(size_t buf_size, void* buf)
{
    if (buf)
    {
        memcpy(buf,
               reinterpret_cast<const char*>(m_buf.data())+ m_rd_pos,
               buf_size);
    }
    m_rd_pos += static_cast<const_buffer::size_type>(buf_size);
}

bool read_buffer::wait_for_data(long, long)
{
    if (available() > 0) return true;
    throw std::ios_base::failure("No more data to read");
}

size_t read_buffer::read_some(size_t buf_size, void* buf)
{
    if (available() > 0)
    {
        size_t rd_size = std::min(buf_size, available());
        do_copy(rd_size, buf);
        return rd_size;
    }
    return 0;
}

void read_buffer::read(size_t buf_size, void* buf)
{
    if (buf_size <= available())
    {
        do_copy(buf_size, buf);
    }
    else throw std::ios_base::failure("Not enough data to read");
}

void read_buffer::close()
{
    m_buf.reset();
}

// overrides source::closed
bool read_buffer::closed() const
{
    return available() == 0;
}

native_socket read_buffer::read_handle() const
{
    return invalid_socket;
}

bool read_buffer::has_buffered_data() const
{
    return available() > 0;
}

} } // namespace hamcast
