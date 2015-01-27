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

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::set;

using boost::bad_lexical_cast;
using boost::lexical_cast;
using boost::thread;

using namespace hamcast;

namespace {
/* struct to parse long opts */
    int daemonize = 0;
    struct option long_options[] = {
        {"daemonize",       no_argument,        &daemonize, 1},
        {"config-file",     required_argument,  0,  'f'},
        {"group",           required_argument,  0,  'g'},
        {"downstream",      required_argument,  0,  'd'},
        {"upstream",        required_argument,  0,  'u'},
        {"help",            no_argument,        0,  'h'},
        {"list",            no_argument,        0,  'l'},
        {0, 0, 0, 0}
    };

    volatile bool s_run = true;     // cancel receive loop
    volatile bool s_change = false; // change to s_groups;
    interface_id s_upstream_ifid;   // receive from
    std::vector<interface_id> s_downstreams;
    //interface_id s_downstream_ifid; // send to
    uri s_group;                    // group
    set<uri> s_upstream_groups;     // forward upstream to downstream
    set<uri> s_downstream_groups;   // forward downstream to upstream
    boost::mutex s_mutex;           // lock to secure above sets
}

void usage (const std::string& program)
{
        cout << endl;
        cout << "USAGE" << endl;
        cout << "\t" << program << " [-hl] {-f <CONFIG-FILE>|-g <GROUP>} -d <IF> -u <IF>" << endl;
        cout << endl;
}

void help (const std::string& program)
{
    usage(program);
    cout << "OPTION" << endl;
    cout << "\t-h, --help" << endl;
    cout << "\t\tPrint this help screen." << endl;
    cout << "\t-l, --list" << endl;
    cout << "\t\t List all interfaces." << endl;
    cout << "\t-f, --file CONFIG" << endl;
    cout << "\t\tSet CONFIG file with group list." << endl;
    cout << "\t-g, --group GROUP" << endl;
    cout << "\t\tSet multicast GROUP URI." << endl;
    cout << "\t-u, --upstream IF" << endl;
    cout << "\t\tSet upstream interface id IF [single]." << endl;
    cout << "\t-d, --downstream IF" << endl;
    cout << "\t\tSet downstream interface id IF [multiple]." << endl;
    cout << "NOTE" << endl;
    cout << "\tIt is mandatory to either specify a config file (-f) or" << endl;
    cout << "\ta group URI (-g) and interface IDs for upstream and" << endl;
    cout << "\tdownstream." << endl;
    cout << endl;
    cout << "\tFormat of config file is:" << endl;
    cout << "\t\tinterface-ID=Group-URI" << endl;
    cout << endl;
    cout << "AUTHOR" << endl;
    cout << "\tSebastian Meiling <sebastian.meiling (at) haw-hamburg.de" << endl;
    cout << endl;
}

void receive_loop ()
{
    // recv socket
    hamcast::multicast_socket rsock;
    // set if to upstream
    rsock.set_interface(s_upstream_ifid);
    // send socket
    hamcast::multicast_socket ssock;
    // set if to downstream for forwarding
    ssock.set_interfaces(s_downstreams);
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

/**
  * @brief Receive all groups on upstream and forward to downstream
  *
  *
  */

void upstream_loop ()
{
    // recv socket
    hamcast::multicast_socket rsock;
    // set if to upstream
    rsock.set_interface (s_upstream_ifid);
    // send socket
    hamcast::multicast_socket ssock;
    // set if to downstream for forwarding
    ssock.set_interfaces(s_downstreams);

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
            mp = rsock.receive ();
            ssock.send(mp.from(), mp.size(), mp.data());
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
    hamcast::multicast_socket rsock;
    rsock.set_interfaces (s_downstreams);
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
            mp = rsock.receive ();
            ssock.send(mp.from(), mp.size(), mp.data());
        }
    }
}

void read_gfile (const string &f)
{
    cout << "\t... read config file ..." << endl;
    std::ifstream config_file;
    
    config_file.open (f.c_str(), std::ifstream::in);
    string line;
    if (config_file.is_open()) {
        while (config_file.good()) {
            getline (config_file, line);
            if (!line.empty() &&
                    (line.find("#",0) == string::npos) &&
                    (line.find(";",0) == string::npos)) {
                size_t delim = line.find("=",0);
                string iface = line.substr(0,delim);
                string grp = line.substr((delim+1), (line.size()-delim-1));
                uri u (grp);
                if (grp.empty()) {
                    cerr << "Invalid group uri in config file!" << endl;
                    continue;
                }
                try {
                    interface_id iid = lexical_cast<interface_id>(iface);
                    if (iid == s_upstream_ifid) {
                        s_upstream_groups.insert(u);
                    } else  {
                        s_downstream_groups.insert(u);
                    }
                } catch (...) {
                    cerr << "Invalid if-id in config file!" << endl;
                    continue;
                }
            }
        }
    } else {
        cerr << endl;
        cerr << "Unable to open file ..." << endl;
        cerr << endl;
    }
    config_file.close();
}

int get_interface_count ()
{
    std::vector<ipc::interface_property> ifs;
    ifs = ipc::get_interfaces();
    return ifs.size();
}

void list_ifs ()
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

int main (int argc, char** argv)
{
    string program (argv[0]);
    string gfile;
    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hlf:g:u:d:", 
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        unsigned long ul;

        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                cout << "option " << 
                    long_options[option_index].name;
                if (optarg)
                    cout << " with arg " << optarg;
                cout << endl;
                break;

            case 'f':
                gfile = string (optarg);
                break;

            case 'l':
                list_ifs ();
                return EXIT_SUCCESS;
                break;

            case 'g':
                s_group = uri (optarg);
                if (s_group.empty()) {
                    cerr << "Invalid group URI!" << endl;
                    return (EXIT_FAILURE);
                }
                break;

            case 'u':
                try {
                    s_upstream_ifid = lexical_cast<interface_id>(optarg);
                } catch (bad_lexical_cast &e) {
                    cerr << endl;
                    cerr << "Invalid ID for upstream interface!" << endl;
                    cerr << endl;
                    return (EXIT_FAILURE);
                }
                break;

            case 'd':
                try {
                    interface_id downstream = lexical_cast<interface_id>(optarg);
                    s_downstreams.push_back(downstream);
                } catch (bad_lexical_cast &e) {
                    cerr << endl;
                    cerr << "Invalid ID for downstream interface!" << endl;
                    cerr << endl;
                    return (EXIT_FAILURE);
                }
                break;

            case 'h':
                help (program);
                return (EXIT_SUCCESS);
                break;

            default:
                usage (program);
                return (EXIT_FAILURE);
                break;
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
    if (!s_upstream_ifid || s_downstreams.empty()) {
        cerr << endl;
        cerr << "Invalid ID for upstream or downstream interface!" << endl;
        cerr << endl;
        usage (program);
        return (EXIT_FAILURE);
    }
    for (int i=0; i < s_downstreams.size(); ++i) {
        if (s_upstream_ifid == s_downstreams[i]) {
            cerr << endl;
            cerr << "Invalid ID for upstream or downstream interface!" << endl;
            cerr << endl;
            usage (program);
            return (EXIT_FAILURE);
        }
    }

    if (gfile.size() > 0)
        read_gfile (gfile);

    boost::thread t_receiver;
    boost::thread t_upstream;
    boost::thread t_downstream;

    cout << "+++ HAMcast Demo IMG +++" << endl;

    if (!s_group.empty()) {
        cout << " forwarding group: " << s_group.str() << " from if-id: " 
             << s_upstream_ifid << " to if-id: " << s_downstreams[0]
             << " ." << endl;
        ipc::is_img(true);
        t_receiver = thread(receive_loop);
    } else if (!s_upstream_groups.empty() || !s_downstream_groups.empty()) {
        cout << " forwarding from if-id: " << s_upstream_ifid 
             << " to downstreams." << endl;
        for (set<uri>::iterator it = s_upstream_groups.begin(); 
                        it != s_upstream_groups.end(); ++it) {
            cout << "\t" << (*it).str() << endl;
        }
        cout << endl;
        cout << " forwarding from downstreams to if-id: " << s_upstream_ifid << " ." << endl;
        for (set<uri>::iterator it = s_downstream_groups.begin(); 
                        it != s_downstream_groups.end(); ++it) {
           cout << "\t" << (*it).str() << endl;
        }
        cout << endl;
        // set img flag, this is a mockup in middleware :)
        ipc::is_img(true);
        t_upstream = thread (upstream_loop);
        t_downstream = thread (downstream_loop);
    } else {
        cerr << endl;
        cerr << "Missing group URI, specify by -f or -g!" << endl;
        cerr << endl;
        return (-1);
    }
    cout << "++++++++++++++++++++++++" << endl;
    cout << endl;
    t_receiver.join();
    t_upstream.join();
    t_downstream.join();
    ipc::is_img(false);
    cout << "++++++++++++++++++++++++" << endl;
    cout << endl;
    return (EXIT_SUCCESS);
}
