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
//using boost::thread;

using namespace hamcast;

namespace {
/* struct to parse long opts */
    struct option long_options[] = {
        {"input-group",     required_argument,  0,  'i'},
        {"output-group",    required_argument,  0,  'o'},
        {"downstream",      required_argument,  0,  'd'},
        {"upstream",        required_argument,  0,  'u'},
        {"help",            no_argument,        0,  'h'},
        {"list",            no_argument,        0,  'l'},
        {0, 0, 0, 0}
    };

    volatile bool s_run = true;     // cancel receive loop
    interface_id s_upstream_ifid;   // receive from
    interface_id s_downstream_ifid; // receive from
    uri input_group;                // group
    uri output_group;
    //volatile bool s_change = false; // change to s_groups;
    //std::vector<interface_id> s_downstreams;
    //interface_id s_downstream_ifid; // send to
    //set<uri> s_upstream_groups;     // forward upstream to downstream
    //set<uri> s_downstream_groups;   // forward downstream to upstream
    //boost::mutex s_mutex;           // lock to secure above sets
}

void usage (const std::string& program)
{
    cout << endl;
    cout << "USAGE: " << program << " [-hl] -i <GRP> -o <GRP> -d <IF> -u <IF>" << endl;
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

    cout << "\t-i, --input-group INGRP" << endl;
    cout << "\t\tSet multicast uri for INGRP receiving from." << endl;

    cout << "\t-i, --output-group OUTGRP" << endl;
    cout << "\t\tSet multicast uri for OUTGRP sending to." << endl;

    cout << "\t-u, --upstream IF" << endl;
    cout << "\t\tSet upstream interface id IF." << endl;

    cout << "\t-d, --downstream IF" << endl;
    cout << "\t\tSet downstream interface id IF." << endl;

    cout << endl;
    cout << "AUTHOR" << endl;
    cout << "\tSebastian Meiling <sebastian.meiling (at) haw-hamburg.de" << endl;
    cout << endl;
}

void receive_loop (const uri& rcv_grp, const uri& snd_grp, interface_id rcv_if, interface_id snd_if)
{
    // recv socket
    hamcast::multicast_socket rsock;
    // set if to upstream
    rsock.set_interface(rcv_if);
    // send socket
    hamcast::multicast_socket ssock;
    // set if to downstream for forwarding
    ssock.set_interface(snd_if);
    rsock.join(rcv_grp);
    while (s_run) {
        hamcast::multicast_packet mp;
        if (rsock.try_receive (mp, 50)) {
            ssock.send(snd_grp,mp.size(),mp.data());
        }
    }
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

void join_leave_dummy(const uri& group)
{
    set<uri>::iterator it;
    hamcast::multicast_socket sock;
    sock.join(group);
    sock.leave(group);
}

int main (int argc, char** argv)
{
    uri recv_group;
    uri send_group;
    interface_id recv_ifid;
    interface_id send_ifid;

    string program (argv[0]);
    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hli:o:u:d:",
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        unsigned long ul;

        switch (c) {
        case 0: {
            if (long_options[option_index].flag != 0)
                break;
            cout << "option " << long_options[option_index].name;
            if (optarg)
                cout << " with arg " << optarg;
            cout << endl;
            break;
        }
        case 'l': {
                list_ifs ();
                return EXIT_SUCCESS;
                break;
        }
        case 'i': {
            recv_group = uri (optarg);
            if (recv_group.empty()) {
                cerr << "Invalid input group URI!" << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'o': {
            send_group = uri (optarg);
            if (send_group.empty()) {
                cerr << "Invalid output group URI!" << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'u': {
            try {
               recv_ifid = lexical_cast<interface_id>(optarg);
            } catch (bad_lexical_cast &e) {
                cerr << endl;
                cerr << "Invalid ID for upstream interface!" << endl;
                cerr << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'd': {
            try {
               send_ifid = lexical_cast<interface_id>(optarg);
            } catch (bad_lexical_cast &e) {
                cerr << endl;
                cerr << "Invalid ID for upstream interface!" << endl;
                cerr << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'h': {
            help (program);
            return (EXIT_SUCCESS);
            break;
        }
        default: {
            usage (program);
            return (EXIT_FAILURE);
            break;
        }

        } // switch
    } // while
    
    if (argc < 4) {
        cerr << endl;
        cerr << "Invalid parameters!" << endl;
        cerr << endl;
        usage (program);
        return (EXIT_FAILURE);
    }

    // check if more than one interface available
    if (get_interface_count () < 2) {
        cerr << endl;
        cerr << "Not enough interfaces, detected only one or no interface. " << endl;
        cerr << "At least two (2) are needed ..." << endl;
        cerr << endl;
        return (EXIT_FAILURE);
    }

    // check group uris
    if (recv_group.empty () || send_group.empty ()) {
        cerr << endl;
        cerr << "Invalid input/output group URIs, or not set!" << endl;
        cerr << endl;
        usage (program);
        return (EXIT_FAILURE);
    }
    // check interface ids
    if (recv_ifid == send_ifid || recv_ifid == 0 || send_ifid == 0) {
        cerr << endl;
        cerr << "Invalid ID for upstream or downstream interface!" << endl;
        cerr << endl;
        usage (program);
        return (EXIT_FAILURE);
    }

    cout << "+++ HC-MAT: HAMcast Multicast Address Translator +++" << endl;
    cout << endl;
    ipc::is_img(true);
    s_run = true;
    cout << " Translate group " << recv_group << " on IF " << recv_ifid
         << " to group " << send_group << " on IF " << send_ifid << endl;
    cout << endl;
    receive_loop (recv_group, send_group, recv_ifid, send_ifid);
    ipc::is_img(false);
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << endl;
    return (EXIT_SUCCESS);
}
