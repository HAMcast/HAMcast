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

/* standard headers */
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

#include "util.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::set;

using boost::bad_lexical_cast;
using boost::lexical_cast;
using boost::thread;

using namespace hamcast;

void recv_loop ()
{
}

void send_loop ()
{
}

/*
void receive_loop ()
{
    // recv socket
    hamcast::multicast_socket rsock;
    // set if to upstream
    rsock.set_interface(s_upstream_ifid);
    // send socket
    hamcast::multicast_socket ssock;
    // set if to downstream for forwarding
    ssock.set_interface(s_downstream_ifid);
    while (s_run) {
        rsock.join(s_group);
        hamcast::multicast_packet mp;
        while (s_run && !s_change) {
            if (rsock.try_receive (mp, 50)) {
                ssock.send(mp.from(),mp.size(),mp.data());
            }
        }
    }
}
*/

/**
  * @brief Receive all groups on upstream and forward to downstream
  *
  *
  */
/*
void upstream_loop ()
{
    // recv socket
    hamcast::multicast_socket rsock;
    // set if to upstream
    rsock.set_interface (s_upstream_ifid);
    // send socket
    hamcast::multicast_socket ssock;
    // set if to downstream for forwarding
    ssock.set_interface(s_downstream_ifid);

    while (s_run) {
        set<uri>::iterator it;
        for (it = s_upstream_groups.begin(); 
                        it != s_upstream_groups.end(); ++it) {
            rsock.join(*it);
        }
        // dummy join to establish mapping
        for (it = s_downstream_groups.begin();
                        it != s_downstream_groups.end(); ++it) {
            rsock.join(*it);
            rsock.leave(*it);
        }
        hamcast::multicast_packet mp;
        while (s_run && !s_change) {
            if (rsock.try_receive (mp, 50)) {
                ssock.send(mp.from(),mp.size(),mp.data());
            }
        }
    }
}
*/
/**
  * @brief reverse case of upstream loop
  *
  * Receive all groups on downtream forward to upstream
  */
/*
void downstream_loop ()
{
    hamcast::multicast_socket rsock;
    rsock.set_interface (s_downstream_ifid);
    hamcast::multicast_socket ssock;
    ssock.set_interface(s_upstream_ifid);

    while (s_run) {
        set<uri>::iterator it;
        for (it = s_downstream_groups.begin(); 
                        it != s_downstream_groups.end(); ++it) {
            rsock.join(*it);
        }
        hamcast::multicast_packet mp;
        while (s_run && !s_change) {
            if (rsock.try_receive (mp, 50)) {
                ssock.send(mp.from(),mp.size(),mp.data());
            }
        }
    }
}
*/
int get_interface_count ()
{
    std::vector<ipc::interface_property> ifs;
    ifs = ipc::get_interfaces();
    return ifs.size();
}

void list_interfaces ()
{
    std::vector<ipc::interface_property> ifs;
    ifs = ipc::get_interfaces();
    cout << " list interfaces: " << endl;
    for (size_t i = 0; i < ifs.size(); ++i) {
        cout << "\tID: " << ifs[i].id << ", name: " << ifs[i].name << endl;
        cout << "\t\taddress:    " << ifs[i].address << endl;
        cout << "\t\ttechnology: " << ifs[i].technology << endl;
        cout << endl;
    }
    cout << "++++++++++++++++++++++++" << endl;
    cout << endl;
}

/*
void join_leave_dummy()
{
    set<uri>::iterator it;
    hamcast::multicast_socket sock;
    for (it = s_upstream_groups.begin(); 
                    it != s_upstream_groups.end(); ++it) {
        sock.join(*it);
        sock.leave(*it);
    }
}
*/
