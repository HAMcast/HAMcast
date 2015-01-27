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

#ifndef HAMCAST_UTIL_CONST_BUFFER_HPP
#define HAMCAST_UTIL_CONST_BUFFER_HPP

#include <boost/cstdint.hpp>
#include "hamcast/ref_counted.hpp"

namespace hamcast { namespace util {

/**
 * @brief Holds a buffer that cannot be modified.
 *
 * The const_buffer class provides a representation of an immutable
 * buffer. The object deletes the its data if it has ownership
 * but does not modify it otherwise.
 *
 * @warning The given (const) buffer must stay valid for the whole
 *          lifetime of the buffer object if
 *          <code>{@link ownership()} == false</code>.
 * @warning The data must have the type <code>char*</code> if
 *          <code>{@link ownership()} == true</code> because the dtor
 *          calls <code>delete[] reinterpret_cast<char*>(data)</code>.
 */
class const_buffer : boost::noncopyable
{

 public:

    /**
     * @brief The used integer type to store the size of the buffer.
     */
    typedef boost::uint32_t size_type;

 private:

    // size of m_data
    size_type m_size;
    // content of the buffer
    void* m_data;
    // true if the buffer should delete its data in its dtor
    bool m_ownership;

 public:

    /**
     * @brief Creates a buffer without data and {@link size()} == 0.
     */
    const_buffer();

    /**
     * @brief Creates a buffer without ownership.
     * @param buf_size The size of @p buf.
     * @param buf The const C-buffer.
     * @warning @p buf must stay valid for the
     *          whole lifetime of this object.
     */
    const_buffer(size_type buf_size, const void* buf);

    /**
     * @brief Creates a buffer with optional ownership.
     * @param buf_size The size of @p buf.
     * @param buf The C-buffer.
     * @param ownership Denotes if this object should
     *                  take ownership of @p buf.
     * @warning @p buf must stay valid for the whole lifetime of this object
     *          if <code>{@link ownership()} == false</code>.
     * @warning @p buf must be allocated with <code>new char[...]</code>
     *          if <code>{@link ownership()} == true</code>.
     */
    const_buffer(size_type buf_size, void* buf, bool ownership);

    /**
     * @brief Deletes its data if <code>{@link ownership()} == false</code>.
     * @warning Calls <code>delete[] reinterpret_cast<char*>(data)</code>
     *          if the const buffer has ownership of its data.
     */
    ~const_buffer();

    /**
     * @brief Exchange the contents of the two buffers.
     * @param other The const_buffer that should
     *              be swapped with this object.
     */
    void swap(const_buffer& other);

    /**
     * @brief Equivalent to <code>const_buffer().swap(*this)</code>.
     */
    void reset();

    /**
     * @brief Equivalent to
     *        <code>const_buffer(buf_size, buf).swap(*this)</code>.
     * @param buf_size The size of @p buf.
     * @param buf The new const buffer.
     */
    void reset(size_type buf_size, const void* buf);

    /**
     * @brief Equivalent to <code>const_buffer(...).swap(*this)</code>.
     * @param buf_size The size of @p buf.
     * @param buf The new buffer.
     * @param ownership Denotes if this object should
     *                  take ownership of @p buf.
     */
    void reset(size_type buf_size, void* buf, bool ownership);

    /**
     * @brief Get the size of {@link data()}.
     * @returns The size of the internal C-buffer.
     */
    inline size_type size() const { return m_size; }

    /**
     * @brief Check if the buffer owns its data.
     * @returns <code>true</code> if this object owns the internal C-buffer;
     *         otherwise <code>false</code>.
     */
    inline bool ownership() const { return m_ownership; }

    /**
     * @brief Get the data of this buffer.
     * @returns The internal C-buffer.
     */
    inline const void* data() const { return m_data; }

    /**
     * @brief Check if the buffer is empty
     *        (equivalent to <code>size() == 0</code>).
     * @returns <code>true</code> if {@link size()} == 0;
     *         otherwise <code>false</code>
     */
    inline bool empty() const { return size() == 0; }

    /**
     * @brief Get the internal buffer and {@link reset() reset}
     *        the buffer afterwards.
     * @returns <code>std::make_pair({@link size()}, {@link data()})</code>
     * @warning Throws an exception if <code>!{@link ownership()}</code>.
     * @pre <code>{@link ownership()} || {@link empty()}</code>.
     */
    std::pair<size_type, void*> take();

};

} }


#endif // HAMCAST_UTIL_CONST_BUFFER_HPP
