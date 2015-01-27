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

#ifndef IP_EXCEPTIONS_HPP
#define IP_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace ipm
{
    class ipm_exception : public std::runtime_error
    {
    public:
        inline ipm_exception(const std::string& msg = "EXCEPTION IN IP MODULE") : std::runtime_error(msg){ }
    };
    
    class ipm_socket_create_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_socket_create_exception(const std::string& msg = "Create socket failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_mcast_operation_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_mcast_operation_exception(const std::string& msg = "Multicast group operation failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_interface_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_interface_exception(const std::string& msg = "Interface operation failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_bind_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_bind_exception(const std::string& msg = "Bind failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_not_implemented_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_not_implemented_exception(const std::string& msg = "Not implemented yet.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_recv_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_recv_exception(const std::string& msg = "Socket receive failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_sendto_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_sendto_exception(const std::string& msg = "Socket sendto failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_setsockopt_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_setsockopt_exception(const std::string& msg = "Socket setsockopt failure.") : ipm::ipm_exception(msg){ }
    };
    
    class ipm_address_exception : public ipm::ipm_exception
    {
    public:
        inline ipm_address_exception(const std::string& msg = "Address conversion failure.") : ipm::ipm_exception(msg){ }
    };
}

#endif // IP_EXCEPTIONS_HPP
