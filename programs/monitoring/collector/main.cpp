#include <cstdlib>
#include <iostream>
#include "async_tcp_server.hpp"
#include <hamcast/hamcast.hpp>
#include "http_message.hpp"
#include <boost/asio.hpp>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <boost/asio.hpp>
#include <multicast_module.hpp>
#include <getopt.h>
#include <cstdlib>

using std::string;
using std::cout;
using std::endl;

namespace {
    bool running = false;
    boost::asio::io_service io_service;

    const string c_group = "ip://239.0.0.2:1234";

    struct option long_options[] = {
        {"help",            no_argument,        0,  'h'},
        {"port",            required_argument,  0,  'p'},
        {"group",           required_argument,  0,  'g'},
        {"interface",       required_argument,  0,  'i'},
        {"s_update",        required_argument,  0,  's'},
        {"m_update",        required_argument,  0,  'm'},
        {0, 0, 0, 0}
    };
}

void usage (string& program)
{
    cout << "USAGE: " << program << " -[h] -[pgism <param>]" << endl;
}

void help (string& program)
{
    usage (program);
}

int main(int argc, char* argv[])
{
    int server_port =0;
    int server_updaterate=0;
    string multicast_group;
    int multicast_updaterate=0;
    string program (argv[0]);
    string interface ("eth0");

    if (argc < 1) {
        usage (program);
        return (-1);
    }

    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hp:g:i:s:m:",
                                long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'p':
                server_port = atoi(optarg);
                break;

            case 'g':
                multicast_group = string (optarg);
                break;

            case 'i':
                interface = string (optarg);
                break;

            case 's':
                server_updaterate = atoi (optarg);
                break;

            case 'm':
                multicast_updaterate = atoi (optarg);
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

    if(multicast_group.empty()){
        multicast_group = c_group;
        cout << "group not set, using default : "+c_group << endl;
    }
    if(server_port == 0){
        server_port = 35000;
        cout << "port not set, using default : "+boost::lexical_cast<string>(server_port) << endl;
    }
    if(server_updaterate == 0){
        server_updaterate = 10;
        cout << "server_updaterate not set, using default : "+boost::lexical_cast<string>(server_updaterate) << endl;
    }
    if(multicast_updaterate == 0){
        multicast_updaterate = 30;
        cout << "multicast_updaterate not set, using default : "+boost::lexical_cast<string>(multicast_updaterate) << endl;
    }

    boost::asio::io_service io_service;


    async_tcp_server tcp_server(io_service,server_port, server_updaterate);

//    char ifname[IF_NAMESIZE];
//    if_indextoname(2, ifname);
//    std::cout << ifname << std::endl;
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

    std::string pnum(boost::lexical_cast<std::string>(server_port));
    std::string server_address(addressBuffer);
    server_address+=":"+pnum;
    tcp_server.start();
    multicast_module m(multicast_updaterate,multicast_group,server_address);
    m.start_send();

    return (EXIT_SUCCESS);
}
