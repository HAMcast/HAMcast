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
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

#include "util.hpp"

// some output stuff we use
using std::cout;
using std::cerr;
using std::endl;

// some standard stuff we use
using std::string;
using std::map;
using std::set;
using std::vector;

// some boost stuff we use
using boost::bad_lexical_cast;
using boost::lexical_cast;
using boost::thread;

// primary namespace we use
using namespace hamcast;

// some local variables
namespace {
    int daemonize = 0;
    /* struct to parse long opts */
    struct option long_options[] = {
        {"daemonize",       no_argument,        &daemonize, 1},
        {"config-file",     required_argument,  0,  'f'},
        {"list",            no_argument,        0,  'l'},
        {"upstream",        required_argument,  0,  'u'},
        {"help",            no_argument,        0,  'h'},
        {0, 0, 0, 0}
    };

    interface_id s_upstream_ifid;   // receive from
}

/**
 * @brief Print brief usage
 * @param program
 */
void usage (const std::string& program)
{
        cout << endl;
        cout << "USAGE" << endl;
        cout << "\t" << program << " [-hl] {-f <CONFIG-FILE>|-g <GROUP>} -d <IF> -u <IF>" << endl;
        cout << endl;
}

/**
 * @brief Print help screen
 * @param program
 */
void help (const std::string& program)
{
    usage(program);
    cout << "OPTION" << endl;
    cout << "\t-h, --help" << endl;
    cout << "\t\tPrint this help screen." << endl;
    cout << "\t-l, --list" << endl;
    cout << "\t\t Print a list of available interfaces." << endl;
    cout << "\t-f, --file CONFIG" << endl;
    cout << "\t\tSet CONFIG file with group list." << endl;
    cout << "\t-u, --upstream IF" << endl;
    cout << "\t\tSet default upstream interface id IF." << endl;
    cout << "NOTE" << endl;
    cout << endl;
    cout << "\tFormat of config file is:" << endl;
    cout << "\t\tinterface-ID=Group-URI" << endl;
    cout << endl;
    cout << "AUTHOR" << endl;
    cout << "\tSebastian Meiling <sebastian.meiling (at) haw-hamburg.de" << endl;
    cout << endl;
}

/**
 * @brief Parse IMG configuration from config file
 * @param fname Config file name
 * @return Map of IMG config
 */
map< interface_id, set<uri> > read_config_file (const string &fname)
{
    map<interface_id, set<uri> > result;
    std::ifstream f_config;
    f_config.open(fname.c_str(), std::ifstream::in);
    string line;
    if (f_config.is_open()) { // file open
        while (f_config.good()) { // file readable
            getline (f_config, line); // read a line
            if ((!line.empty()) && (line.find("#")==string::npos) &&
                (line.find(";")==string::npos)) { // line valid?
                size_t delim = line.find("=",0);
                string iface = line.substr(0,delim);
                string grp = line.substr((delim+1), (line.size()-delim-1));
                uri u (grp);
                if (u.empty()) {
                    cerr << "Invalid group uri in config file!" << endl;
                    continue;
                }
                interface_id iid = lexical_cast<interface_id>(iface);
                map<interface_id, set<uri> >::iterator it = result.find(iid);
                if (it != result.end()) {
                    it->second.insert (u);
                }
                else {
                    set<uri> new_set;
                    new_set.insert(u);
                    std::pair<interface_id, set<uri> > tmp (iid, new_set);
                    result.insert(tmp);
                }
            }
        }
    }
    else {
        cerr << endl;
        cerr << "Unable to open file ..." << endl;
        cerr << endl;
    }
    f_config.close();
    return result;
}

/**
 * @brief main
 * @param argc
 * @param argv
 * @return Exit code
 */
int main (int argc, char** argv)
{
    string program (argv[0]);
    string gfile;
    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "hlf:u:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 0: {
            if (long_options[option_index].flag != 0)
                break;
            cout << "option " <<  long_options[option_index].name;
            if (optarg)
                cout << " with arg " << optarg;
            cout << endl;
            break;
        }
        case 'f': {
            gfile = string (optarg);
            break;
        }
        case 'l': {
            list_interfaces ();
            return EXIT_SUCCESS;
            break;
        }
        case 'u': {
            try {
                s_upstream_ifid = lexical_cast<interface_id>(optarg);
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

    map<interface_id, set<uri> > img_config;
    if (gfile.size() > 0) {
        img_config = read_config_file(gfile);
    }

    vector<ipc::interface_property> ifs = ipc::get_interfaces();
    cout << "+++ HAMcast Demo IMG +++" << endl;

    return (EXIT_SUCCESS);
}
