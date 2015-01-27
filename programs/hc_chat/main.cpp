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

#include <iostream>
#include <iterator>
#include <algorithm>

#include <boost/thread.hpp>

#include "hamcast/ipc.hpp"
#include "hamcast/hamcast.hpp"

using std::cin;
using std::cerr;
using std::cout;
using std::endl;

namespace { volatile bool s_run = true; }

void receive_loop(hamcast::uri group)
{
    hamcast::multicast_socket s;
    s.join(group);
    hamcast::multicast_packet mp;
    while (s_run)
    {
        if (s.try_receive(mp, 50))
        {
	    std::cout << "multicast message received" <<  std::endl;
            std::string tmp;
            const char* msg = reinterpret_cast<const char*>(mp.data());
            std::copy(msg, msg + mp.size(),
                      std::back_insert_iterator<std::string>(tmp));
            tmp += '\n';
            cout << tmp;
            cout.flush();
        }
        // else: check flag again
    }
}

int main(int argc, char** argv)
{
    hc_set_default_log_fun(HC_LOG_TRACE_LVL);
    hamcast::uri group;
    if (argc == 2)
    {
        group = argv[1];
        if (group.empty())
        {
            cerr << " " << argv[1] << " is not a valid URI" << endl;
            return 1;
        }
    }
    else if (argc == 1)
    {
        group = "ip://239.0.0.1:1234";
    }
    else
    {
        cout << "usage: hc_char [group]" << endl;
        return 2;
    }
    std::string username;
    do
    {
        cout << "Enter a nickname: ";
        cout.flush();
        std::getline(cin, username);
    }
    while (username.empty());
    // handshake with receiver thread
    cout << "Hello " << username << "\ntype 'quit' to exit\n";
    cout.flush();
    boost::thread receiver(receive_loop, group);
    std::string message;
    hamcast::multicast_socket s;
    for (;;)
    {
        std::string tmp;
        std::getline(cin, tmp);
        if (tmp == "quit")
        {
            cout << "goodbye\n";
            cout.flush();
            s_run = false;
            receiver.join();
            return 0;
        }
        message  = username;
        message += ": ";
        message += tmp;
        s.send(group, message.size(), message.c_str());
    }
}
