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

#include <ctime>
#include <limits>
#include <iostream>

#include "hamcast/hamcast.hpp"

#include <boost/thread.hpp>

using std::cout;
using std::endl;

using namespace hamcast;

namespace {

size_t received = 0;
time_t last_sec = (time_t) 0;

} // namespace <anonymous>

void handle_packet(const uri&, size_t, const void*)
{
    time_t now = time(NULL);
    if (now != last_sec)
    {
        if (last_sec != 0) cout << received << "/s" << endl;
        received = 1;
        last_sec = now;
    }
    else ++received;
}

void sync_recv_bench()
{
    multicast_socket ms;
    ms.join("jo:ker");
    for (;;)
    {
        multicast_packet mp = ms.receive();
        handle_packet(mp.from(), mp.size(), mp.data());
    }
}

void async_recv_bench()
{
    async_multicast_socket ms(handle_packet);
    ms.join("jo:ker");
    for (;;)
    {
        sleep(20);
    }
}

int main(int argc, char **argv)
{

    hc_set_default_log_fun(HC_LOG_TRACE_LVL);

    if (argc == 2)
    {
        if (strcmp(argv[1], "--sync_recv_bench") == 0)
        {
            sync_recv_bench();
            return 0;
        }
        else if (strcmp(argv[1], "--async_recv_bench") == 0)
        {
            async_recv_bench();
            return 0;
        }
        else if (strcmp(argv[1], "--send_some") == 0)
        {
            std::string foo = "bar";
            multicast_socket ms;
            cout << "send 1000 packets" << endl;
            for (int i = 0; i < 1000; ++i)
                ms.send("no:one", foo.size(), foo.c_str());
            cout << "done" << endl;
            return 0;
        }
    }

    multicast_socket msock;
    std::vector<uri> groups;
    if (argc > 1 ) {
        cout << "List of groups in arguments:" << endl;
        for (int j=1; j < argc; ++j) {
            uri group (argv[j]);
            if (!group.empty()) {
                cout << "Group[" << j << "] " << group << endl;
                groups.push_back(group);
            }
        }
    }
    if (groups.size() > 0) {
        cout << "Joining groups ..." << endl;
        for (size_t k=0; k< groups.size(); ++k) {
            msock.join(groups[k]);
            sleep(1);
        }
    }
    cout << "Wait ..." << endl << endl;
    sleep(1);
    cout << "List of available interfaces:" << endl << endl;
    std::vector<interface_property> ifs;
    ifs = get_interfaces();
    for (size_t i = 0; i < ifs.size(); ++i)
    {
        cout << "Interface[id = " << ifs[i].id << "]:" << endl
             << "\tname = " << ifs[i].name << endl
             << "\taddress = " << ifs[i].address << endl
             << "\ttechnology = " << ifs[i].technology << endl << endl;
    }

    cout << "List of known groups per Interface:" << endl << endl;
    for (size_t l = 0; l < ifs.size(); ++l) {
        cout << "\tInterface: " << ifs[l].name << endl;
        std::vector<std::pair<uri, boost::uint32_t> > if_groups =
                                                group_set(ifs[l].id);
        cout << "\t - Number of Groups: " << if_groups.size() << endl;
        for (size_t q = 0; q < if_groups.size(); ++q) {
            cout << "\t -- Group: " << if_groups[q].first << endl;
            std::vector<uri> group_parents =
                    parent_set(ifs[l].id, if_groups[q].first);
            for (size_t m = 0; m < group_parents.size(); ++m) {
                cout << "\t --- Parent: " << group_parents[m] << endl;
            }
            std::vector<uri> group_children =
                    children_set(ifs[l].id, if_groups[q].first);
            for (size_t n = 0; n < group_children.size(); ++n) {
                cout << "\t --- Child: " << group_children[n] << endl;
            }
        }
    }
    cout << endl;
    sleep(5);
    if (groups.size() > 0) {
        cout << "Leaving groups ..." << endl;
        for (size_t p=0; p < groups.size(); ++p) {
            msock.leave(groups[p]);
        }
    }
    return 0;
}

