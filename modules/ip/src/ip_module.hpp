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

#ifndef IPMODULE_HPP
#define IPMODULE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>

#include "hamcast/hamcast_module.h"

#include "native_socket.hpp"

/**
  * @author Fabian Hollfer <hamcast (at) fholler.de>
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  *
  * @mainpage ipmodule
  *
  * @section intro Introduction
  *
  * @section details Details
  *
  * @section config Configuration
  * @example middleware.ini
  * This is an example of a minimal middleware.ini file to load and use the IP module.
  */

namespace ip_module
{

/**
  * @brief pointer to the recv callback function
  */
hc_recv_callback_t m_recv_callback;

/**
 *  @brief Describes internal data (state) of an ip module instance
 */
struct module_data{
    // map of uri to native socket
    std::map<hc_uri_t, boost::shared_ptr<ip_module::native_socket> > m_sockets;
    // mutex to lock m_sockets during add/del
    boost::mutex m_sockets_mtx;
    // receive worker thread
    boost::thread recv_thrd;
    // the underlying ip interface
    std::string iface_ip;
    // module handle for receive callback
    hc_module_instance_handle_t m_mihdl;
};

/**
 * @brief Send socket for module instance
 */
ip_module::native_socket m_send_socket;

/**
 * @brief checks if an given URI is an valid ipmodule address
 * @param addr must be an URL that contains only numerical IP-addresses,
 * @return true if given addr is an valid ipmodule address
 */
bool is_addr_valid(const hc_uri_t& addr);

/**
 *  @brief returns true if the Multicast group described by the given URI is subscribed
 *  @param m_sockets List of sockets opened by this module instance
 *  @param uri Group name to check
 *  @return
 */
bool is_subscribed(const std::map<hc_uri_t, boost::shared_ptr<ip_module::native_socket> >* m_sockets, const hc_uri_t& uri);

/**
 *  @brief function that will be started as thread to handle receiving on the native sockets
 *  @param instance_ptr Pointer to module_data
 */
void recv_thread(hc_module_instance_t instance_ptr);

/**
 *  @brief Casts void pointer to module_data pointer
 *  @param instance_ptr (void) Pointer, will be casted
 *  @return Pointer to module_data struct
 */
inline module_data* cast(hc_module_instance_t instance_ptr)
{
    return reinterpret_cast<module_data*>(instance_ptr);
}

}// namespace
#endif
