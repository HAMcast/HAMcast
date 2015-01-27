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

#include <sstream>
#include <boost/lexical_cast.hpp>

#include "hamcast/exception.hpp"

namespace hamcast {

connection_to_middleware_failed::~connection_to_middleware_failed() throw()
{
}

const char* connection_to_middleware_failed::what() const throw()
{
	switch (m_occurred_error)
	{

	 case no_config_file_found:
		return "no config file found";

	 case no_running_middleware_found:
		return "no running middleware found";

	 case socket_creation_failed:
		return "socket creation failed";

	 case incompatible_middleware_found:
		return "incompatible middleware found";

	 default: return "an unknown error occured";

	}
}

void build_req_failed_message(std::string& what,
							  int line,
							  const char* file,
							  const char* msg)
{
	what  = "Requirement failed in file \"";
	what += file;
	what += "\" on line ";
	what += boost::lexical_cast<std::string>(line);
	what += ": ";
	what += msg;
}

requirement_failed::requirement_failed(int line,
									   const char* file,
									   const char* msg)
{
	build_req_failed_message(m_what, line, file, msg);
}

requirement_failed::requirement_failed(int line,
									   const char* file,
									   const std::string& msg)
{
	build_req_failed_message(m_what, line, file, msg.c_str());
}

requirement_failed::requirement_failed(const std::string &msg) : m_what(msg)
{
}

const char* requirement_failed::what() const throw()
{
	return m_what.c_str();
}

requirement_failed::~requirement_failed() throw()
{
}

internal_interface_error::internal_interface_error(const std::string& msg)
	: m_what(msg)
{
}

const char* internal_interface_error::what() const throw()
{
	return m_what.c_str();
}

internal_interface_error::~internal_interface_error() throw()
{
}

} // namespace hamcast
