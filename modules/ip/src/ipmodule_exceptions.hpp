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

#ifndef IPMODULE_EXCEPTIONS_HPP
#define IPMODULE_EXCEPTIONS_HPP

#include <stdexcept>

namespace ip_module{

///@author Fabian Holler<hamcast@fholler.de>
class ipmodule_exception : public std::logic_error
{
 public:
    inline ipmodule_exception(const std::string& msg = "ipmodule Exception") : logic_error(msg){ }

};

class socket_create_exception : public ipmodule_exception {
 public:
    inline socket_create_exception(const std::string& msg = "socket creation exception") : ipmodule_exception(msg) { }
};

class setsockopt_exception : public ipmodule_exception {
 public:
    inline setsockopt_exception(const std::string& msg = std::string("setsockopt Exception")) : ipmodule_exception(msg) { }
};

class recv_exception : public ipmodule_exception{
 public:
    inline recv_exception(const std::string& msg = std::string("recv Exception")) : ipmodule_exception(msg) { }
};

class sendto_exception : public ipmodule_exception{
 public:
    inline sendto_exception(const std::string& msg = std::string("sendto Exception")) : ipmodule_exception(msg) { }
};

class bind_exception : public ipmodule_exception{
 public:
    inline bind_exception(const std::string& msg = std::string("bind exception")) : ipmodule_exception(msg) { }
};

class inet_ntop_exception : public ipmodule_exception{
 public:
    inline inet_ntop_exception(const std::string& msg = std::string("inet_ntop exception")) : ipmodule_exception(msg) { }
};

class name_resolve_exception : public ipmodule_exception{
 public:
    inline name_resolve_exception(const std::string& msg = "name resolve Exception") : ipmodule_exception(msg) { }
};

class not_subscribed_exception : public ipmodule_exception{
 public:
    inline not_subscribed_exception(const std::string& msg = "not subscribed") : ipmodule_exception(msg) { }
};

class asm_group_allready_subscribed_exception : public ipmodule_exception{
 public:
    inline asm_group_allready_subscribed_exception(const std::string& msg ="asm group allready subscribed") : ipmodule_exception(msg) { }
};

class wrong_address_family_exception : public ipmodule_exception{
 public:
    inline wrong_address_family_exception(const std::string& msg="wrong address family") : ipmodule_exception(msg) { }
};

class pselect_exception : public ipmodule_exception{
 public:
    inline pselect_exception(const std::string& msg="pselect Exception") : ipmodule_exception(msg) { }
};

class invalid_address_exception : public ipmodule_exception{
 public:
    inline invalid_address_exception(const std::string& msg="invalid address") : ipmodule_exception(msg) { }
};

class ioctl_exception : public ipmodule_exception{
 public:
    inline ioctl_exception(const std::string& msg="ioctl Exception") : ipmodule_exception(msg) { }
};

class no_interfaces_found_exception : public ipmodule_exception{
 public:
    inline no_interfaces_found_exception(const std::string& msg="no interfaces found") : ipmodule_exception(msg) { }
};

class interface_not_exist_exception : public ipmodule_exception{
 public:
    inline interface_not_exist_exception(const std::string& msg="interface does not exist") : ipmodule_exception(msg) { }
};

class not_implemented_exception : public ipmodule_exception{
 public:
    inline not_implemented_exception(const std::string& msg="not implemented") : ipmodule_exception(msg) { }
};

class realloc_exception : public ipmodule_exception{
 public:
    inline realloc_exception(const std::string& msg="realloc Exception") : ipmodule_exception(msg) { }
};
class epoll_exception : public ipmodule_exception{
 public:
    inline epoll_exception(const std::string& msg="epoll Exception") : ipmodule_exception(msg) { }
};
class io_exception : public ipmodule_exception{
 public:
    inline io_exception(const std::string& msg="I/O Exception") : ipmodule_exception(msg) { }
};

}
#endif
