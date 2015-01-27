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

#ifndef HAMCAST_CONFIG_HPP
#define HAMCAST_CONFIG_HPP

#include <string>
#include <boost/cstdint.hpp>

#ifdef __GNUC__
#define HC_STATIC_CONSTANT __attribute__ ((unused))
#else
#define HC_STATIC_CONSTANT
#endif

#ifdef HC_DOCUMENTATION
#define HAMCAST_MACOS
#define HAMCAST_LINUX
#define HAMCAST_WINDOWS
#define HAMCAST_64BIT
#endif

/**
 * @def HAMCAST_MACOS
 * @brief This macro is defined, if the host is running Mac OS X.
 * @ingroup Config
 *
 * @def HAMCAST_LINUX
 * @brief This macro is defined, if the host is running Linux.
 * @ingroup Config
 *
 * @def HAMCAST_WINDOWS
 * @brief This macro is defined, if the host is running Windows.
 * @ingroup Config

 * @def HAMCAST_64BIT
 * @brief This macro is defined, if the host is running a 64-bit system.
 * @ingroup Config
 */

#if defined(__APPLE__)
#  define HAMCAST_MACOS
#elif defined(__GNUC__) && defined(__linux__)
#  define HAMCAST_LINUX
#elif defined(WIN32)
#  define HAMCAST_WINDOWS
#else
#  error Platform and/or compiler not supportet
#endif

#if defined(__amd64__) || defined(__LP64__)
#  define HAMCAST_64BIT
#endif

namespace hamcast { namespace {

/**
 * @brief The HAMcast magic number.
 * @ingroup Config
 */
const boost::uint32_t magic_number HC_STATIC_CONSTANT = 0xDEADC0DE;

/**
 * @brief The major version of the library.
 * @ingroup Config
 */
const boost::uint32_t major_version HC_STATIC_CONSTANT = 0;

/**
 * @brief The minor version of the library.
 * @ingroup Config
 */
const boost::uint32_t minor_version HC_STATIC_CONSTANT = 6;

/**
 * @brief The minimum major version that is (IPC) compatible to this version.
 * @ingroup Config
 */
const boost::uint32_t min_compatile_major_version HC_STATIC_CONSTANT = 0;

/**
 * @brief The minimum minor version that is (IPC) compatible to this version.
 * @ingroup Config
 */
const boost::uint32_t min_compatile_minor_version HC_STATIC_CONSTANT = 6;

#ifndef HC_DEFAULT_MAX_BUFFER_SIZE
#define HC_DEFAULT_MAX_BUFFER_SIZE (16 * 1024 * 1024)
#endif

/**
 * @brief The default maximum buffer size (16MB).
 * @ingroup Config
 */
const size_t default_max_buffer_size HC_STATIC_CONSTANT = HC_DEFAULT_MAX_BUFFER_SIZE;

#ifndef HC_MAX_CHANNEL_QUEUE_SIZE
#define HC_MAX_CHANNEL_QUEUE_SIZE (16 * 1024 * 1024)
#endif // HC_MAX_CHANNEL_QUEUE_SIZE

/**
 * @brief The maximum buffer size of the internal queue for incoming messages
 *        (only used in HAMcast lite).
 */
const size_t max_channel_queue_size HC_STATIC_CONSTANT = HC_MAX_CHANNEL_QUEUE_SIZE;

/**
 * @brief The default chunk/block size for IO buffers.
 * @note This constant is used by {@link util::buffered_sink}.
 * @ingroup Config
 */
const size_t default_block_size HC_STATIC_CONSTANT = 512;

/**
 * @brief The maximum number of pending asynchronous send messages.
 * @ingroup Config
 */
const size_t max_pending_sends HC_STATIC_CONSTANT = 256;

/**
 * @brief The maximum number of buffered asynchronous send messages.
 * @ingroup Config
 * @note Should be factor of default_max_buffer_size.
 */
const size_t max_buffered_sends HC_STATIC_CONSTANT = 512;

/**
 * @brief Deduced from default_max_buffer_size and max_buffered_sends.
 * @ingroup Config
 */
const size_t min_buffer_chunk HC_STATIC_CONSTANT =
        default_max_buffer_size / max_buffered_sends;

/**
 * @brief The maximum number of cumulative acknowledged messages.
 * @ingroup Config
 */
const size_t max_ack_block_size HC_STATIC_CONSTANT = 64;

/**
 * @brief The time interval of forced acknowledgements in milliseconds.
 *
 * Default is 5ms. -- changed to 1ms by smeiling 13-09-26
 * @ingroup Config
 */
const int force_ack_ms_interval HC_STATIC_CONSTANT = 1;

/**
 * @brief The default maximum size of for IO buffers (5MB).
 * @ingroup Config
 * @note This constant is used by {@link util::buffered_sink}.
 */
const size_t default_max_write_buffer_size HC_STATIC_CONSTANT = (1024*1024*5);

//TODO: distinguish between POSIX and Windows
/**
 * @brief The path of the meeting point.
 *
 * The meeting point is a directory where the middleware
 * stores all config parameters needed by client applications.
 * @ingroup Config
 */
const char* meeting_point HC_STATIC_CONSTANT =
        "/tmp/hamcast/meeting_point/middleware/";

/**
 * @brief The path of the meeting point.
 *
 * The meeting point is a directory where the middleware
 * stores all config parameters needed by client applications.
 * @ingroup Config
 */
const char* socket_path HC_STATIC_CONSTANT = "/tmp/hamcast/socket";

/**
 * @brief The root part of the meeting point path.
 *
 * The HAMcast middleware creates all directories below this path
 * on startup and removes them on termination.
 */
const char* meeting_point_root HC_STATIC_CONSTANT = "/tmp/";

/**
 * @brief The filename of the middleware lock file.
 * @ingroup Config
 */
const char* lock_filename HC_STATIC_CONSTANT = "middleware.lock_file";

/**
 * @brief The filename of the middleware config file.
 * @ingroup Config
 */
const char* config_filename HC_STATIC_CONSTANT = "middleware.config_file";

/**
 * @brief The default maximum message size (8KB).
 */
const boost::uint32_t default_max_msg_size HC_STATIC_CONSTANT = 8 * 1024;

/**
 * @brief Utility function to get the library version as string in the format
 *        <code>{major_version}.{minor_version}</code>.
 * @ingroup Config
 * @returns The HAMcast version as string.
 */
std::string version_string();

} } // namespace hamcast::<anonymous>

#endif // HAMCAST_CONFIG_HPP
