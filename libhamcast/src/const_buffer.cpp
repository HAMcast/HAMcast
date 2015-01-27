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

#include <algorithm>
#include "hamcast/exception.hpp"
#include "hamcast/util/const_buffer.hpp"

namespace hamcast { namespace util {

const_buffer::const_buffer() : m_size(0), m_data(0), m_ownership(false)
{
}

const_buffer::const_buffer(const_buffer::size_type buf_size, const void* buf)
	: m_size(buf_size), m_data(const_cast<void*>(buf)), m_ownership(false)
{
}

const_buffer::const_buffer(const_buffer::size_type buf_size, void* buf, bool oship)
	: m_size(buf_size), m_data(buf), m_ownership(oship)
{
}

const_buffer::~const_buffer()
{
	if (m_ownership && m_data)
	{
		delete[] reinterpret_cast<char*>(m_data);
	}
	m_size = 0;
	m_data = 0;
	m_ownership = false;
}

void const_buffer::swap(const_buffer& other)
{
	std::swap(m_size, other.m_size);
	std::swap(m_data, other.m_data);
	std::swap(m_ownership, other.m_ownership);
}

void const_buffer::reset()
{
	const_buffer cb;
	swap(cb);
}

void const_buffer::reset(size_type buf_size, const void* buf)
{
	const_buffer cb(buf_size, buf);
	swap(cb);
}

void const_buffer::reset(size_type buf_size, void* buf, bool ownership)
{
	const_buffer cb(buf_size, buf, ownership);
	swap(cb);
}

std::pair<const_buffer::size_type, void*> const_buffer::take()
{
	HC_REQUIRE(ownership());
	std::pair<const_buffer::size_type, void*> result(m_size, m_data);
	reset();
	return result;
}

} } // namespace hamcast::util
