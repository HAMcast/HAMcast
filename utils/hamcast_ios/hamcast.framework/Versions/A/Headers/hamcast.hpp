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

#ifndef HAMCAST_HAMCAST_HPP
#define HAMCAST_HAMCAST_HPP

#include <string>
#include <vector>
#include <exception>
#include <stdexcept>

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/preprocessor/stringize.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/config.hpp"
#include "hamcast/socket_id.hpp"
#include "hamcast/exception.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/hamcast_logging.h"
#include "hamcast/membership_event.hpp"
#include "hamcast/multicast_packet.hpp"
#include "hamcast/multicast_socket.hpp"
#include "hamcast/interface_property.hpp"

#include "hamcast/ipc/api.hpp"

/**
 * @author Dominik Charousset <dominik.charousset\@haw-hamburg.de>
 *
 * @mainpage libhamcast
 *
 * @section Intro Introduction
 *
 * This library is part of the HAMcast project. It provides an easy to use
 * C++ API that uses the HAMcast middleware (see @ref IPC_arch).
 *
 * All classes and functions needed to write a HAMcast based application
 * are in the namespace {@link hamcast}. Usually, all you need
 * to know about are the classes {@link hamcast::uri},
 * {@link hamcast::multicast_socket} and {@link hamcast::multicast_packet}.
 *
 * {@link hamcast::util} contains utility classes and helper functions
 * that might be useful if you're implementing a technology module.
 * IPC related classes and function reside in the namespace
 * {@link hamcast::ipc}, but are usually uninteresting for application
 * developers. The implementation details and the IPC protocol
 * documentation is fundamental if you want to port libhamcast to
 * another language and/or unsupported plattform.
 *
 * Note that undocumented parts of the library aren't safe to use because
 * they might be changed and/or removed even in minor updates or the library.
 *
 * @section GettingStarted Getting started with libhamcast
 *
 * See the {@link simple_receiver.cpp} and {@link simple_sender.cpp}
 * examples to become a first overview.
 *
 * @section IPC_arch Architecture
 *
 * HAMcast uses a middleware to abstract from native network communication.
 * Thus, HAMcast based application are independent from multicast
 * technologies and the middleware chooses the best available transport
 * protocol at runtime (no compile-time dependencies & Late Binding).
 *
 * \image html ipc_arch.png
 *
 * @section Compilation Compilation and installation
 *
 * HAMcast uses @c Automake on Unix platforms (Linux, Mac OS X, etc.).
 * The usual way to build libhamcast is:
 *
 * <code>
 * automake -i<br>
 * ./configure<br>
 * make<br>
 * make install [as root]
 * </code>
 *
 * @subsection compilation_note Note for Linux users
 *
 * If you're getting the error
 * <b><code>"No CAS for this plattform"</code></b>, then you need to set your
 * hardware architecture by hand using the <code>configure</code> script:
 *
 * - run <code>uname -m</code> to see your hardware architecture; this
 *   should be one of @c i568, @c i686 or @c x86_64
 * - run <code>./configure CXXFLAGS="-march={YOUR_ARCH}"</code> where
 *   @c {YOUR_ARCH} is your hardware architecture
 *   (note: GCC expects <code>x86-64</code> instead of <code>x86_64</code>)
 * - continue with @c make
 *
 * @example simple_receiver.cpp
 * @relates hamcast::multicast_socket
 * This example shows how to receive multicast packets with a multicast socket.
 *
 * @example simple_sender.cpp
 * @relates hamcast::multicast_socket
 * This example shows how to send data to a multicast group.
 *
 * @namespace hamcast
 *
 * @brief This is the root namespace of hamcast.
 *
 * The <b>hamcast</b> namespace contains all needed classes and functions
 * to write HAMcast based applications.
 *
 * @namespace hamcast::ipc
 *
 * @brief This namespace contains all IPC related parts of the HAMcast
 *        library.
 *
 * @namespace hamcast::util
 *
 * @brief This namespace contains utility classes.
 *
 * @defgroup ExceptionHandling Exception handling.
 * @brief All HAMcast exceptions inherit std::exception.
 *
 * {@link hamcast::connection_to_middleware_failed
 *        connection_to_middleware_failed}
 * and
 * {@link hamcast::connection_to_middleware_lost
 *        connection_to_middleware_lost}
 * are thrown by libhamcast if communication to the middleware couldn't be
 * established or is interrupted. A libhamcast based application terminates
 * if one of those exception was thrown.
 *
 * {@link hamcast::internal_interface_error internal_interface_error} is
 * thrown if an unexpected exception occurs in a middleware module. This
 * exception is forwarded to the client on IPC calls.
 *
 * {@link hamcast::requirement_failed requirement_failed} is thrown if
 * you call member functions of an opject that's in an invalid state or
 * if you pass invalid parameters to a function.
 *
 * @defgroup Config Configuration constants, macros and utility functions.
 *
 */
namespace hamcast {

// HC_DOCUMENTATION is predefined by doxygen.
// This block "hides" the real implementation from the documentation.
#ifdef HC_DOCUMENTATION
/**
 * @copydoc hamcast::ipc::get_interfaces(void)
 * @see {@link hamcast::ipc::get_interfaces(void)
 *             ipc::group_interfaces()}
 */
std::vector<interface_property> get_interfaces() { }

/**
 * @copydoc hamcast::ipc::group_set(interface_id)
 * @see {@link hamcast::ipc::group_set(interface_id) ipc::group_set(void)}
 */
std::vector<std::pair<uri, boost::uint32_t> > group_set(interface_id iid) { }

/**
 * @copydoc hamcast::ipc::parent_set(interface_id,const uri&)
 * @see {@link hamcast::ipc::parent_set(interface_id,const uri&)
 *             ipc::parent_set()}
 */
std::vector<uri> parent_set(interface_id iid, const uri& group) { }

/* @copydoc hamcast::ipc::children_set(interface_id,const uri&)
 * @see {@link hamcast::ipc::children_set(interface_id,const uri&)
 *             ipc::children_set()}
 */
std::vector<uri> children_set(interface_id iid, const uri& group) { }

/**
 * @copydoc hamcast::ipc::neighbor_set(interface_id)
 * @see {@link hamcast::ipc::neighbor_set(interface_id) ipc::neighbor_set()}
 */
std::vector<uri> neighbor_set(interface_id iid) { }

/**
 * @copydoc hamcast::ipc::designated_host(interface_id,const uri&)
 * @see {@link hamcast::ipc::designated_host(interface_id, const uri&)
 *             ipc::designated_host(interface_id, const uri&)}
 */
bool designated_host(interface_id iid, const uri& group) { }

#endif

using ipc::get_interfaces;
using ipc::group_set;
using ipc::parent_set;
using ipc::children_set;
using ipc::neighbor_set;
using ipc::designated_host;

} // namespace hamcast

#endif // HAMCAST_HAMCAST_HPP
