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
#include <getopt.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"
#include "hamcast/ipc/api.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::set;
using std::vector;
using std::map;

using boost::bad_lexical_cast;
using boost::lexical_cast;
using boost::thread;

using namespace hamcast;

namespace {
/* struct to parse long opts */
    int daemonize = 0;
    struct option long_options[] = {
        {"downstream",      required_argument,  0,  'd'},
        {"upstream",        required_argument,  0,  'u'},
        {"help",            no_argument,        0,  'h'},
        {0, 0, 0, 0}
    };

    std::map<interface_id, interface_property> s_ifaces;
    volatile bool   s_run = true;     // cancel receive loop
    volatile bool   us_change = false; // change to s_groups;
    volatile bool   ds_change = false;
    interface_id    s_us_ifid;   // receive from
    interface_id    s_ds_ifid; // send to
    hamcast::multicast_socket us_sock;
    hamcast::multicast_socket ds_sock;
    set<uri> s_upstream_groups;     // forward upstream to downstream
    set<uri> s_downstream_groups;   // forward downstream to upstream
    boost::mutex us_mtx;
    boost::mutex ds_mtx;

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


} // namespace

void usage (const std::string& program)
{
        cout << endl;
        cout << "USAGE" << endl;
        cout << "\t" << program << " [-h] -d <IF> -u <IF>" << endl;
        cout << endl;
}

void help (const std::string& program)
{
    usage(program);
    cout << "OPTION" << endl;
    cout << "\t-h, --help" << endl;
    cout << "\t\tPrint this help screen." << endl;
    cout << "\t-u, --upstream IF" << endl;
    cout << "\t\tSet upstream interface id IF." << endl;
    cout << "\t-d, --downstream IF" << endl;
    cout << "\t\tSet downstream interface id IF." << endl;
    cout << "NOTE" << endl;
    cout << "\tIt is mandatory to specify interface IDs for upstream and downstream." << endl;
    cout << endl;
    cout << "AUTHOR" << endl;
    cout << "\tSebastian Meiling <sebastian.meiling (at) haw-hamburg.de" << endl;
    cout << endl;
}

void get_interface_list()
{
    std::vector<interface_property> vec = ipc::get_interfaces();
    for (std::vector<interface_property>::iterator i = vec.begin();
         i != vec.end();
         ++i)
    {
        s_ifaces.insert(std::make_pair(i->id, *i));
    }
}

void event_callback(const membership_event& event)
{
    cout << "event:\n"
         << "\ttype = " << event_type_name(event.type()) << "\n"
         << "\tinterface = " << event.iface_id() << " [name: " << s_ifaces[event.iface_id()].name << "]\n"
         << "\tgroup = " << event.group().str() << endl;
    if (event.iface_id() == s_ds_ifid) {
        membership_event_type etype = event.type();
        switch (etype) {
            case join_event:{
                boost::mutex::scoped_lock guard(us_mtx);
                s_upstream_groups.insert(event.group());
                us_change = true;
                break;
            }
            case leave_event:{
                boost::mutex::scoped_lock guard(us_mtx);
                s_upstream_groups.erase(event.group());
                us_change = true;
                break;
            }
            default:{
                cerr << "Event Type not supported!" << endl;
                break;
            }
        }
    }
/*
    else if (event.iface_id() == s_ds_ifid) {
        membership_event_type etype = event.type();
        switch (etype) {
            case join_event:{
                boost::mutex::scoped_lock guard(us_mtx);
                s_downstream_groups.insert(event.group());
                ds_change = true;
                break;
            }
            case leave_event:{
                boost::mutex::scoped_lock guard(us_mtx);
                s_downstream_groups.erase(event.group());
                ds_change = true;
                break;
            }
            default:{
                cerr << "Event Type not supported!" << endl;
                break;
            }
        }
    }
*/
    else {
        cerr << " - unused interface id." << endl;
    }
}

/**
  * @brief Receive all groups on upstream and forward to downstream
  *
  *
  */
void upstream_loop ()
{
    // send socket
    hamcast::multicast_socket ssock;
    // set if to downstream for forwarding
    ssock.set_interface(s_ds_ifid);

    while (s_run) {

        set<uri>::iterator it;
        us_mtx.lock();
        for (it = s_upstream_groups.begin(); 
                        it != s_upstream_groups.end(); ++it) {
            us_sock.join(*it);
        }
        // dummy join to establish mapping
        ds_mtx.lock();
        for (it = s_downstream_groups.begin();
                        it != s_downstream_groups.end(); ++it) {
            us_sock.join(*it);
            us_sock.leave(*it);
        }
        us_change =false;
        ds_mtx.unlock();
        us_mtx.unlock();
        hamcast::multicast_packet mp;
        while (s_run && !us_change) {
            if (us_sock.try_receive (mp, 50)) {
                ssock.send(mp.from(),mp.size(),mp.data());
            }
        }
    }
}

/**
  * @brief reverse case of upstream loop
  *
  * Receive all groups on downtream forward to upstream
  */

void downstream_loop ()
{
    hamcast::multicast_socket ssock;
    ssock.set_interface(s_us_ifid);

    while (s_run) {
        set<uri>::iterator it;
        ds_mtx.lock();
        for (it = s_downstream_groups.begin(); 
                        it != s_downstream_groups.end(); ++it) {
            ds_sock.join(*it);
        }
        ds_change = false;
        ds_mtx.unlock();
        hamcast::multicast_packet mp;
        while (s_run && !ds_change) {
            if (ds_sock.try_receive (mp, 50)) {
                ssock.send(mp.from(),mp.size(),mp.data());
            }
        }
    }
}

int get_interface_count ()
{
    std::vector<ipc::interface_property> ifs;
    ifs = ipc::get_interfaces();
    return ifs.size();
}

int main (int argc, char** argv)
{
    string program (argv[0]);

    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hu:d:",
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        unsigned long ul;

        switch (c) {
        case 0:{
            if (long_options[option_index].flag != 0)
                break;
            cout << "option " << long_options[option_index].name;
            if (optarg)
                cout << " with arg " << optarg;
            cout << endl;
            break;
        }
        case 'u':{
            try {
                s_us_ifid = lexical_cast<interface_id>(optarg);
            }
            catch (bad_lexical_cast &e) {
                cerr << endl;
                cerr << "Invalid ID for upstream interface!" << endl;
                cerr << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'd':{
            try {
                s_ds_ifid = lexical_cast<interface_id>(optarg);
            }
            catch (bad_lexical_cast &e) {
                cerr << endl;
                cerr << "Invalid ID for downstream interface!" << endl;
                cerr << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'h':{
            help (program);
            return (EXIT_SUCCESS);
            break;
        }
        default:{
            usage (program);
            return (EXIT_FAILURE);
            break;
        }
        }
    }
    
    if (argc < 2) {
        usage (program);
        return (EXIT_FAILURE);
    }

    // check if more than one interface available
    if (get_interface_count () < 2) {
        cerr << "Not enough interfaces, detected only one or no interface. "
             << "At least two (2) are needed ..." << endl;
        return (EXIT_FAILURE);
    }

    // check if if-ids are set and not equal
    if (!s_us_ifid || !s_ds_ifid ||
            (s_us_ifid == s_ds_ifid)) {
        cerr << endl;
        cerr << "Invalid ID for upstream or downstream interface!" << endl;
        cerr << endl;
        usage (program);
        return (EXIT_FAILURE);
    }

    us_sock.set_interface (s_us_ifid);
    ds_sock.set_interface (s_ds_ifid);

    cout << "+++ HAMcast Demo IMG +++" << endl;

    register_event_callback(event_callback);
    get_interface_list();

    // set img flag, this is a mockup in middleware :)
    ipc::is_img(true);
    boost::thread t_upstream = thread (upstream_loop);
    boost::thread t_downstream = thread (downstream_loop);

    cout << "++++++++++++++++++++++++" << endl;
    cout << endl;
    t_upstream.join();
    t_downstream.join();
    ipc::is_img(false);
    cout << "++++++++++++++++++++++++" << endl;
    cout << endl;
    return (EXIT_SUCCESS);
}
