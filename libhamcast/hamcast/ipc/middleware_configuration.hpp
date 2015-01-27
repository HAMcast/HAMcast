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

#ifndef HAMCAST_IPC_MIDDLEWARE_CONFIGURATION_HPP
#define HAMCAST_IPC_MIDDLEWARE_CONFIGURATION_HPP

#include <string>
#include <boost/cstdint.hpp>

#ifdef WIN_32
#else
#	include <unistd.h>
    namespace hamcast { namespace ipc { typedef ::pid_t process_id; } }
#endif

namespace hamcast { namespace ipc {

/**
 * @brief Stores the configuration of the currently running middleware.
 * @ingroup IPC
 */
class middleware_configuration
{

    // port this middleware is running at
    boost::uint16_t m_port;
    // m_port as string
    std::string m_str_port;
    // proccess id of the running middleware
    process_id m_pid;

    bool write_to(const char* filename);
    bool read_from(const char* filename);

 public:

    /**
     * @brief Get the process id of the middleware.
     * @returns The process id of the middleware as plattform-specific
     *         integer type.
     */
    inline const process_id& pid() const { return m_pid; }

    /**
     * @brief Get the port the middleware is running at.
     * @returns The port as (16-bit, unsigned) integer.
     */
    inline boost::uint16_t port() const { return m_port; }

    /**
     * @brief Get the port as string representation.
     * @returns The port as string representation.
     */
    inline const std::string& port_as_string() const { return m_str_port; }

    /**
     * @brief Create an "empty" configuration with port = 0.
     */
    middleware_configuration();

    /**
     * @brief Create a valid configuration with given port and detect
     *        process id automatically.
     * @param port The port as (16-bit, unsigned) integer.
     */
    middleware_configuration(boost::uint16_t port);

    /**
     * @brief Read to configuration from the middleware.
     * @returns <code>true</code> if a config file from a running middleware
     *         was found and read; otherwise <code>false</code>.
     */
    bool read();

    /**
     * @brief Write this configuration to disc.
     * @returns <code>true</code> if a config file was written;
     *         otherwise <code>false</code>.
     */
    bool write();

};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_MIDDLEWARE_CONFIGURATION_HPP
