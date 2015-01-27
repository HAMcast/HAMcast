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

#include <limits>
#include <vector>
#include <string>
#include <iostream>

#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace hamcast;

namespace {

// number of tests
const size_t num_tests = 10;

}

void test_middleware()
{
    HC_LOG_TRACE("");
    vector<ipc::interface_property> vec;
    vec = ipc::get_interfaces();
    for (size_t i = 0; i < vec.size(); ++i)
    {
        cout << "interface[id = " << vec[i].id << "]:" << endl
             << "\tname = " << vec[i].name << endl
             << "\taddress = " << vec[i].address << endl
             << "\ttechnology = " << vec[i].technology << endl;
    }
}

void do_send(multicast_socket& sock, size_t num, const vector<uri>& uris)
{
    string payload("Ever danced with the devil in the pale moonlight?");
    size_t uri_id = 0;
    for (size_t i = 0; i < num; ++i)
    {
        sock.send(uris[uri_id], payload.size(), payload.data());
        uri_id = (uri_id + 1) % uris.size();
    }
}

void print_usage()
{
    cout << "usage: ./test_sender [payload in bytes]" << endl;
}

int main(int argc, char** argv)
{
    hc_set_default_log_fun(HC_LOG_TRACE_LVL);

    HC_LOG_TRACE("");

    int data_size;// = 100;

    if (argc == 1)
    {
        data_size = 100;
    }
    else if (argc == 2)
    {
        try
        {
            data_size = boost::lexical_cast<int>(argv[1]);
        }
        catch (...)
        {
            print_usage();
            return -2;
        }
    }
    else
    {
        print_usage();
        return -1;
    }

    test_middleware();

    multicast_socket sock;
    vector<uri> uris;
    time_t t0;
    time_t t1;

    vector<uri> all_uris;
    all_uris.push_back("ip://239.0.0.1:1234");
    all_uris.push_back("ip://239.0.0.2:1234");
    all_uris.push_back("ip://239.0.0.3:1234");
    all_uris.push_back("ip://239.0.0.4:1234");
    all_uris.push_back("ip://239.0.0.5:1234");
    /*
    all_uris.push_back("ip://239.0.0.6:1234");
    all_uris.push_back("ip://239.0.0.7:1234");
    all_uris.push_back("ip://239.0.0.8:1234");
    all_uris.push_back("ip://239.0.0.9:1234");
    all_uris.push_back("ip://239.0.1.0:1234");
    all_uris.push_back("ip://239.0.1.1:1234");
    all_uris.push_back("ip://239.0.1.2:1234");
    all_uris.push_back("ip://239.0.1.3:1234");
    all_uris.push_back("ip://239.0.1.4:1234");
    all_uris.push_back("ip://239.0.1.5:1234");
    all_uris.push_back("ip://239.0.1.6:1234");
    all_uris.push_back("ip://239.0.1.7:1234");
    all_uris.push_back("ip://239.0.1.8:1234");
    all_uris.push_back("ip://239.0.1.9:1234");
    all_uris.push_back("ip://239.0.2.0:1234");
    all_uris.push_back("ip://239.0.2.1:1234");
    all_uris.push_back("ip://239.0.2.2:1234");
    all_uris.push_back("ip://239.0.2.3:1234");
    all_uris.push_back("ip://239.0.2.4:1234");
    all_uris.push_back("ip://239.0.2.5:1234");
    all_uris.push_back("ip://239.0.2.6:1234");
    all_uris.push_back("ip://239.0.2.7:1234");
    all_uris.push_back("ip://239.0.2.8:1234");
    all_uris.push_back("ip://239.0.2.9:1234");
    */

    size_t num_messages = 1000000;

    for (vector<uri>::iterator i = all_uris.begin(); i != all_uris.end(); ++i) {
        uris.push_back(*i);
        cout << "send " << num_messages << " messages to "
             << uris.size() << " group (round robin)" << endl;
        t0 = time(0);
        do_send(sock, num_messages, uris);
        t1 = time(0);
        cout << "time: " << (t1 - t0) << "s" << endl;
    }
    uris.clear();
    uris.push_back(all_uris[3]);
    for (int i = 0; i < 3; ++i) {
        t0 = time(0);
        do_send(sock, num_messages, uris);
        t1 = time(0);
        cout << "time: " << (t1 - t0) << "s" << endl;
    }
    return 0;
}

