#include <getopt.h>
#include <ifaddrs.h>
#include <netinet/in.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

#include "multicast_module.hpp"
#include "tcp_client_connection.hpp"

using std::cout;
using std::endl;
using std::string;
using hamcast::interface_id;

namespace {
    bool running = false;
    boost::asio::io_service io_service;

    const string c_group = "ip://239.0.0.2:1234";
    static interface_id ifid = 0;

    struct option long_options[] = {
        {"help",            no_argument,        0,  'h'},
        {"daemon-id",       required_argument,  0,  'd'},
        {"group",           required_argument,  0,  'g'},
        {"interface",       required_argument,  0,  'i'},
        {"config-file",     required_argument,  0,  'f'},
        {0, 0, 0, 0}
    };
}

void usage (string& program)
{
    cout << "USAGE: " << program << " -[h] -[dgif <param>]" << endl;
}

void help (string& program)
{
    usage (program);
}

int main(int argc,char **argv)
{
    string configfile;
    string interface ("eth0");
    string group = c_group;
    string daemonid;
    string program (argv[0]);

    if (argc < 1) {
        usage (program);
        return (-1);
    }

    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hd:g:i:f:",
                                long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'd':
                daemonid = string (optarg);
                break;

            case 'g':
                group = string (optarg);
                break;

            case 'i':{
                interface = string (optarg);
                std::vector<hamcast::ipc::interface_property> ifs;
                ifs = hamcast::ipc::get_interfaces();
                for (size_t i = 0; i < ifs.size(); ++i) {
                    if (interface.compare(ifs[i].name) == 0) {
                        ifid = ifs[i].id;
                    }
                }
                break;
            }
            case 'f':
                configfile = string (optarg);
                break;
            case 'h':
                help (program);
                return (0);
                break;
            default:
                usage (program);
                return (-1);
                break;
        }
    }

    if (daemonid.size() > 0) {
        cout << "Daemon id was set to: " << daemonid << endl;
    }
    else {
        cout << "Daemon id was not set." << endl;
//        char ifname[IF_NAMESIZE];
//        if_indextoname(2, ifname);
//        std::cout << ifname << std::endl;
        struct ifaddrs * ifAddrStruct=NULL;
        struct ifaddrs * ifa=NULL;
        void * tmpAddrPtr=NULL;
        char addressBuffer[INET_ADDRSTRLEN];
        getifaddrs(&ifAddrStruct);
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa ->ifa_addr->sa_family==AF_INET && std::string(ifa->ifa_name).compare(interface)==0) {
                tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                break;
            }
        }
        daemonid = string(addressBuffer);
        cout << "Auto generated daemon id: " << daemonid << endl;
    }


    int up = 1975;

    multicast_module mod(io_service,up,group,daemonid,ifid);
    mod.start_receive();
    return EXIT_SUCCESS;
}
