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

#ifndef HAMCAST_IPC_EXCEPTION_ID_HPP
#define HAMCAST_IPC_EXCEPTION_ID_HPP

namespace hamcast { namespace ipc {

/**
 * @brief Holds the ID of an exception that was thrown during invocation
 *        of a synchronous request.
 * @ingroup IPC
 */
enum exception_id
{

    /**
     * @brief Indicates that no exception occured.
     */
    eid_none                             = 0x0000,

    /**
     * @brief Indicates that a requirement in the
     *        invoked function failed.
     *
     * This causes a synchronous request to throw an instance of
     * {@link hamcast::requirement_failed requirement_failed}.
     */
    eid_requirement_failed               = 0x0001,

    /**
     * @brief Indicates that an unexpected exception
     *        in a middleware module occured.
     *
     * This causes a synchronous request to throw an instance of
     * {@link hamcast::internal_interface_error internal_interface_error}.
     */
    eid_internal_interface_error         = 0x0002

};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_EXCEPTION_ID_HPP
