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

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include "hamcast/ipc.hpp"
#include "hamcast/hamcast.hpp"

#include <boost/thread.hpp>

using namespace std;

namespace {

boost::system_time tout;
size_t total_packages = 0;
size_t packages = 0;
volatile bool done = false;

} // namespace <anonymous>

void handle_packet(const hamcast::uri&, size_t, const void*)
{
    ++packages;
    ++total_packages;
    if (boost::get_system_time() > tout)
    {
        std::cout << packages << " / sec" << std::endl;
        packages = 0;
        tout = boost::get_system_time();
        tout += boost::posix_time::seconds(1);
    }
}

void run_blocking(hamcast::uri grp)
{
    hamcast::multicast_socket s;
    s.join(grp);
    while (!done)
    {
        hamcast::multicast_packet mp = s.receive();
        handle_packet(mp.from(), mp.size(), mp.data());
    }
}

void await_quit()
{
    cout << "write 'q' to quit" << endl;
    std::string dummy;
    while (dummy != "q") cin >> dummy;
}

void usage()
{
    cout << "test_receive_performance [-b] [URI]\n"
            "\n"
            "Options:\n"
            "  -b   use blocking receive (default: async)\n"
            "  URI  join this group (default: ip://[FF02::3]:1234)\n"
            "  -h   print his text\n"
         << endl;
}

int main(int argc, char** argv)
{
    hamcast::uri grp = "ip://[FF02::3]:1234";
    bool async = true;
    vector<string> args(argv + 1, argv + argc);
    if (args.empty())
    {
        usage();
    }
    else if (args.size() > 2)
    {
        cout << "too many arguments" << endl;
        usage();
        return -1;
    }
    else
    {
        if (args.front() == "-b")
        {
            async = false;
        }
        if (args.size() > 1 || async)
        {
            grp = args.back();
            if (grp.empty())
            {
                cerr << args.back() << " is not a valid URI" << endl;
                usage();
                return -1;
            }
        }
    }

    hc_set_default_log_fun(HC_LOG_TRACE_LVL);
    HC_LOG_TRACE("");
    tout = boost::get_system_time();
    tout += boost::posix_time::seconds(1);
    if (async)
    {
        hamcast::async_multicast_socket s(handle_packet);
        s.join(grp);
        await_quit();
    }
    else
    {
        //boost::thread t(run_blocking, grp);
hamcast::multicast_socket s;
s.join(grp);
            await_quit();
        done = true;
        //t.join();
    }
    return 0;
}
