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
#include <map>
#include <string>
#include <vector>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::vector;

using boost::lexical_cast;

using namespace hamcast;

namespace {
/* struct to parse long opts */
    int server = 0;
    struct option long_options[] = {
    //    {"daemonize",       no_argument,        &daemonize, 1},
        {"group",               required_argument,  0,  'g'},
        {"chat-receive-port",   required_argument,  0,  'c'},
        {"chat-send-port",      required_argument,  0,  's'},
        {"nickname",            required_argument,  0,  'n'},
        {"video-port",          required_argument,  0,  'v'},
        {"server",         no_argument,        &server,  1},
        {"help",                no_argument,        0,  'h'},
        {0, 0, 0, 0}
    };

    string group = "ip://239.201.108.1";
    string vport ("7580");
    string crport ("7581");
    string csport ("7582");

    hamcast::uri crgroup;
    hamcast::uri csgroup;
    hamcast::uri vgroup;

    volatile bool s_run = true;
    string nickname;
    int helo_timer = 5000;
    map<string,int> s_members;
    vector<string> s_questions;
    boost::mutex s_members_mutex;
    boost::mutex s_questions_mutex;
    const int max_nohello = 5;
}

void print_usage (int argc, char **argv)
{
    if (argc > 0) {
        cout << "Usage: " << argv[0] << " [-hgcsv] [--server]" << endl;
        cout << endl;
        cout << "\tOptions:" << endl;
        cout << "\t\t-h\t Print this help screen." << endl;
        cout << "\t\t-g\t Set multicast group URI." << endl;
        cout << "\t\t-c\t Set port for chat receive." << endl;
        cout << "\t\t-s\t Set port for chat send." << endl;
        cout << "\t\t-v\t Set port for video." << endl;
        cout << endl;
        cout << "\t\t--server Run in server mode." << endl;
        cout << endl;
     }
}

void parse_msg (const multicast_packet mp)
{
    string tmp;
    const char* msg = reinterpret_cast<const char*>(mp.data());
    std::copy(msg, msg + mp.size(),
                std::back_insert_iterator<std::string>(tmp));
    size_t p_post = tmp.find ("POST",0);
    size_t p_from = tmp.find ("From:", 0);
    bool valid = true;
    if ((p_post != string::npos) && (p_from != string::npos)) {
        size_t p_delim = tmp.find ('\n', p_from);
        string from = tmp.substr ((p_from+6), (p_delim-p_from-6));
        size_t p_hello = tmp.find ("/hello", p_post);
        size_t p_message = tmp.find ("/message", p_post);
        if (p_hello != string::npos) {
            s_members_mutex.lock();
            map<string, int>::iterator it = s_members.find(from);
            if (it != s_members.end()) {
                it->second++;
            } else {
                s_members.insert (std::pair<string, int>(from, max_nohello));
            }
            s_members_mutex.unlock();
        } else if (p_message != string::npos) {
            string message;
            size_t p_size = tmp.find ("Content-Length", 0);
            if (p_size != string::npos) {
                p_delim = tmp.find ('\n', p_size);
                string stmp = tmp.substr (p_size+15,(p_delim-p_size-15));
                unsigned long usize = strtoul (stmp.c_str(), NULL, 10);
                message = tmp.substr(tmp.size()-usize, usize);
                if (server && (mp.from() == csgroup) && (message.size() > 0)) {
                    s_questions_mutex.lock();
                    s_questions.push_back(message);
                    s_questions_mutex.unlock();
                }
            }
            message += '\n';
            cout << from << ": " << message << endl;
        } else {
            valid = false;
        }
    } else {
        valid = false;
    }
}

void hello_loop ()
{
    hamcast::multicast_socket s;
    string helo = "POST /hello HTTP/1.0\n";
    helo += "From: " + nickname + "\n";
    helo += "User-Agent: HAMcastDemo/1.0\n";
    helo += "Content-Length: 0\n";
    helo += "\n";
    while (s_run) {
        if (server)
            s.send (crgroup, helo.size(), helo.c_str());
        else
            s.send (csgroup, helo.size(), helo.c_str());
        boost::system_time tout = boost::get_system_time();
        tout += boost::posix_time::milliseconds(helo_timer);
        boost::system_time sleep_tout;
        do {
            sleep_tout = boost::get_system_time();
            sleep_tout += boost::posix_time::milliseconds(100);
            boost::this_thread::sleep(sleep_tout);
        } while (s_run && (sleep_tout < tout));
        s_members_mutex.lock();
        map<string, int>::iterator it;
        for (it = s_members.begin(); it != s_members.end(); ++it) {
            it->second--;
            if (it->second < 0)
                s_members.erase(it);
        }
        s_members_mutex.unlock();
    }
}

void receive_loop ()
{
    hamcast::multicast_socket rsock;
    hamcast::multicast_socket ssock;
    if (server) {
        rsock.join(csgroup);
    }  else {
        rsock.join(crgroup);
        rsock.join(vgroup);
    }
    hamcast::multicast_packet mp;
    while (s_run)
    {
        if (rsock.try_receive(mp, 50))
        {
            if (server && (mp.from() == csgroup)) {
                ssock.send(crgroup, mp.size(), mp.data());
                parse_msg(mp);
            } else {
                if (mp.from() == crgroup)
                    parse_msg(mp);
            }
        }
        // else: check flag again
    }
}

int main (int argc, char **argv)
{
    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "g:c:s:v:h", 
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

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
            case 'g':
                group = string (optarg);
                break;

            case 'c':
                try {
                    int port = lexical_cast<int>(optarg);
                    if (!(port > 1024) && !(port < 65000))
                        throw 42;
                } catch (...) {
                    cerr << "Invalid port for chat receive!" << endl;
                    return (-1);
                }
                crport = string (optarg);
                break;

            case 's':
                try {
                    int port = lexical_cast<int>(optarg);
                    if (!(port > 1024) && !(port < 65000))
                        throw 42;
                } catch (...) {
                    cerr << "Invalid port for chat send!" << endl;
                    return (-1);
                }
                csport = string (optarg);
                break;

            case 'v':
                try {
                    int port = lexical_cast<int>(optarg);
                    if (!(port > 1024) && !(port < 65000))
                        throw 42;
                } catch (...) {
                    cerr << "Invalid port for video receive!" << endl;
                    return (-1);
                }
                vport = string (optarg);
                break;

            case 'n':
                nickname = string(optarg);
                break;

            case 'h':
                print_usage (argc, argv);
                return 0;
                break;

            default:
                print_usage (argc, argv);
                return (-1);
                break;
        }
    }
    if (crport == vport || csport == vport || csport == crport) {
        cerr << "Error: port numbers must be different!" << endl;
        return (-1);
    }

    // put together chat receive group URI
    crgroup = uri (group + ":" + crport);
    if ( crgroup.empty()) {
        cerr << "Invalid group URI (chat receive)." << endl;
        return (-1);
    }

    // put together chat send group URI
    csgroup = uri (group + ":" + csport);
    if ( csgroup.empty()) {
        cerr << "Invalid group URI (chat send)." << endl;
        return (-1);
    }

    // put together video group URI
    vgroup = uri (group + ":" + vport);
    if (vgroup.empty()) {
        cerr << "Invalid group URI (video)." << endl;
        return (-1);
    }

    while (nickname.empty()) {
        cout << "Enter a nickname: ";
        cout.flush();
        std::getline(cin, nickname);
    }

    boost::thread t_receiver(receive_loop);
    boost::thread t_hello(hello_loop);
    hamcast::multicast_socket ssock;
    string message;
    message += "COMMANDS:\n";
    message += "\thelp: This help screen.\n";
    message += "\tulist: List known users.\n";
    if (server) 
        message += "\tqlist: List questions.\n";
    message += "\tquit: Exit programm.\n";
    message += "----------\n";
    cout << message << endl;
    cout << "CHAT MESSAGES" << endl;
    while (s_run) {
        std::string tmp;
        std::getline(cin, tmp);
        message.clear();
        if (tmp == "quit")
        {
            cout << "goodbye\n";
            cout.flush();
            s_run = false;
            t_receiver.join();
            t_hello.join();
        } else if (tmp == "help") {
            message.clear();
            message += "HELP\n";
            message += "\thelp: This help screen.\n";
            message += "\tulist: List known users.\n";
            if (server) 
                message += "\tqlist: List questions.\n";
            message += "\tquit: Exit programm.\n";
            message += "----------\n";
            cout << message << endl;;
        } else if (tmp == "ulist") {
            message.clear();
            message += "USERS\n";
            message += "\n";
            map<string, int>::iterator it;
            for (it = s_members.begin(); it != s_members.end(); ++it) {
                message += "\t";
                message += it->first;
                message += "\n";
            }
            message += "----------\n";
            cout << message << endl;
        } else if (server && (tmp == "qlist")) {
            message.clear();
            message += "QUESTIONS\n";
            message += "\n";
            for (size_t s=1; s <= s_questions.size(); ++s) {
                message += boost::lexical_cast<string>(s);
                message += ") ";
                message += s_questions[s-1];
                message += "\n";
            }
            message += "----------\n";
            cout << message << endl;
        } else {
            if (tmp.size() > 0) {
                message.clear();
                message  = "POST /message HTTP/1.0\n";
                message += "From: " + nickname + "\n";
                message += "User-Agent: HAMcastDemo/1.0\n";
                message += "Content-Type: text/text\n";
                message += "Content-Length: ";
                message += boost::lexical_cast<string>(tmp.size());
                message += "\n";
                message += "\n";
                message += tmp;
                if (server)
                    ssock.send(crgroup, message.size(), message.c_str());
                else
                    ssock.send(csgroup, message.size(), message.c_str());
            }
        }
    }
    return 0;
}
