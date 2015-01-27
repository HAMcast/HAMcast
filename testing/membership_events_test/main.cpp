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

#include <map>
#include <vector>
#include <iostream>

#include <boost/thread.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/ipc/api.hpp"

using std::cout;
using std::endl;

using namespace hamcast;

namespace {

std::map<interface_id, interface_property> s_ifaces;
boost::condition_variable s_cv;
boost::mutex s_mtx;

const char* event_type_name(membership_event_type etype)
{
	switch (etype)
	{
	 case join_event: return "join event";
	 case leave_event: return "leave event";
	 case new_source_event: return "new source event";
	 default: return "((invalid event type))";
	}
}

} // namespace <anonymous>

void event_callback(const membership_event& event)
{
	cout << "event:\n"
	     << "\ttype = " << event_type_name(event.type()) << "\n"
		 << "\tinterface = " << event.iface_id() << " [name: " << s_ifaces[event.iface_id()].name << "]\n"
		 << "\tgroup = " << event.group().str() << endl;
}

void get_interfaces()
{
	std::vector<interface_property> vec = ipc::get_interfaces();
	for (std::vector<interface_property>::iterator i = vec.begin();
	     i != vec.end();
	     ++i)
	{
		s_ifaces.insert(std::make_pair(i->id, *i));
	}
}

void some_socket_action()
{
	multicast_socket ms;
	ms.join("ip://239.0.0.1:1234");
}

int main(int, char**)
{
	hc_set_default_log_fun(HC_LOG_TRACE_LVL);

	get_interfaces();
	register_event_callback(event_callback);

	// provoke some events
	some_socket_action();

	// suspend main thread (forever)
	boost::unique_lock<boost::mutex> ulock(s_mtx);
	for (;;) { s_cv.wait(ulock); }

	return 0;
}

