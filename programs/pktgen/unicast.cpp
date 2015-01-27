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
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#ifndef __APPLE__
    #include <sys/sysinfo.h>
#endif
#include <arpa/inet.h>
#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/thread.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/lexical_cast.hpp>

extern "C" {
#include "bobhash.h"
}

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::vector;

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

//extern uint32_t BOB_Hash (uint8_t*, uint16_t, uint32_t);

namespace {
/* struct to parse long opts */
    struct option long_options[] = {
    //    {"daemonize",       no_argument,        &daemonize, 1},
        {"help",        no_argument,        0,  'h'},
        {"file",        required_argument,  0,  'f'},
        {"interval",    required_argument,  0,  'i'},
        {"network-ifn", required_argument,  0,  'n'},
        {"port",        required_argument,  0,  'p'},
        {"length",      required_argument,  0,  'l'},
        {"send",        required_argument,  0,  's'},
        {"time",        required_argument,  0,  't'},
        {"offset",      required_argument,  0,  'o'},
        {"width",       required_argument,  0,  'w'},
        {0, 0, 0, 0}
    };

    const int c_debug = 0;

    const string c_port ("7580");               // default port
    const size_t c_payload = 1000;              // default payload in bytes
    const size_t c_bufsize = 65001;
    const size_t c_time = 60;                   // default runtime in sec
    const size_t c_interval = 1000;             // default interval in msec
    uint16_t s_offset = 70;                     // hash offset
    uint16_t s_length = 50;                     // hash width
}   

void usage (const string& program)
{
        cout << "Usage: " << program << " [-h] [-f FILE] [-iplt VAL] [-n IFNAME] [-s DST]" << endl;
        cout << endl;
}
void help (const string& program)
{
    usage(program);
    cout << "Options" << endl;
    cout << "  -h, --help" << endl;
    cout << "    Print this help screen." << endl;
    cout << "  -f, --file FILE" << endl;
    cout << "    Send data to nodes given in FILE." << endl;
    cout << "  -s, --send DST" << endl;
    cout << "    Send data to node DST." << endl;
    cout << "  -i, --intervall TIME" << endl;
    cout << "    Set packet interval TIME, in msec (1000)" << endl;
    cout << "  -n, --network-ifn IFNAME" << endl;
    cout << "    Set interface IFNAME, default is ANY" << endl;
    cout << "  -p, --port NUM" << endl;
    cout << "    Set port NUM" << endl;
    cout << "  -l, --length LEN" << endl;
    cout << "    Set packet payload LENgth in Bytes (100)" << endl;
    cout << "  -t, --time RUNTIME" << endl;
    cout << "    Set RUNTIME of experiment, in sec (60)" << endl;
    cout << "  -o, --offset OFFSET" << endl;
    cout << "    Set payload OFFSET to generate packet hash id from (70)." << endl;
    cout << "  -w, --width LEN" << endl;
    cout << "    Set offset LEN to generate hash id from (50)" << endl;
    cout << endl;
    cout << "Notes" << endl;
    cout << "    Default mode is receiver, if no FILE is given with option -f." << endl;
    cout << endl;
    cout << "Report bugs to <sebastian.meiling(at)haw-hamburg.de>" << endl;
    cout << endl;
}

// http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

void send (const std::vector<struct sockaddr> &receivers, const size_t &payload,
                     const size_t &duration, const size_t &interval)
{
    int ipsock = socket(AF_INET, SOCK_DGRAM, 0);
    if(ipsock == -1)
        perror("socket");

    // check length of payload
    if (payload < sizeof(uint32_t)) {
        std::cerr << "Payload to small, muss be larger than sizeof(uint32_t)!" << std::endl;
        exit(EXIT_FAILURE);
    }
    char* buf = reinterpret_cast<char*>(malloc (payload));
    uint8_t* hash_buf = reinterpret_cast<uint8_t*>(malloc(payload));
    uint32_t packet_count = 1;

    ptime start_time = microsec_clock::local_time();
    ptime cur_time = start_time;
    // specify alphabet for random payload
    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;
    do
    {
        boost::system_time tout = boost::get_system_time();
        tout += boost::posix_time::milliseconds(interval);
#ifndef __APPLE__
        struct sysinfo info;
        sysinfo(&info);
        unsigned long seed = mix(info.uptime, time(NULL), getpid());
#else
        unsigned long seed = mix(clock(), time(NULL), getpid());
#endif
        srand ( seed );
        for (size_t i=0; i < payload; ++i)
            buf[i] = abc[((rand()%len)+1)];

        memcpy(buf, &packet_count, sizeof(packet_count));

        // generate packet HASH_ID from buf as impd4e does 
        bzero (hash_buf, payload);
        memcpy (hash_buf, (buf + (payload - s_offset)), s_length);
        uint32_t ret = BOB_Hash(hash_buf, s_length, initval);

        struct timeval tv;
        unsigned long seconds, useconds, timestamp;
        gettimeofday(&tv, NULL);
        seconds = tv.tv_sec;
        useconds = tv.tv_usec;
        std::cout << seconds; printf("%06lu", useconds); std::cout << "\t" << ret << "\t" << payload << std::endl;
        
        // send packet to all receivers
        for (int i=0; i < receivers.size(); ++i) {
            if(sendto(ipsock, buf, payload, 0, &receivers[i], sizeof(struct sockaddr) ) == -1)
            {
                perror("sendto error: ");
                exit(1);
            }
        }

        // sleep intervall time
        boost::system_time sleep_tout;
        do {
            sleep_tout = boost::get_system_time();
            sleep_tout += boost::posix_time::milliseconds(10);
            boost::this_thread::sleep(sleep_tout);
        } while (sleep_tout < tout);

        cur_time = microsec_clock::local_time();
        ++packet_count;
    }
    while (((cur_time - start_time).total_seconds()) < duration);
    free(buf);
    free(hash_buf);
}

void send_dst (const struct sockaddr *dst, const size_t &payload,
                     const size_t &duration, const size_t &interval)
{
    int ipsock = socket(AF_INET, SOCK_DGRAM, 0);
    if(ipsock == -1)
        perror("socket");

    // check length of payload
    if (payload < sizeof(uint32_t)) {
        std::cerr << "Payload to small, muss be larger than sizeof(uint32_t)!" << std::endl;
        exit(EXIT_FAILURE);
    }
    char* buf = reinterpret_cast<char*>(malloc (payload));
    uint8_t* hash_buf = reinterpret_cast<uint8_t*>(malloc(payload));
    uint32_t packet_count = 1;

    ptime start_time = microsec_clock::local_time();
    ptime cur_time = start_time;
    // specify alphabet for random payload
    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;
    do
    {
        boost::system_time tout = boost::get_system_time();
        tout += boost::posix_time::milliseconds(interval);
#ifndef __APPLE__
        struct sysinfo info;
        sysinfo(&info);
        unsigned long seed = mix(info.uptime, time(NULL), getpid());
#else
        unsigned long seed = mix(clock(), time(NULL), getpid());
#endif
        srand ( seed );
        for (size_t i=0; i < payload; ++i)
            buf[i] = abc[((rand()%len)+1)];

        memcpy(buf, &packet_count, sizeof(packet_count));

        // generate packet HASH_ID from buf as impd4e does

        bzero (hash_buf, payload);
        memcpy (hash_buf, (buf + (payload - s_offset)), s_length);
        uint32_t ret = BOB_Hash(hash_buf, s_length, initval);

        struct timeval tv;
        unsigned long seconds, useconds, timestamp;
        gettimeofday(&tv, NULL);
        seconds = tv.tv_sec;
        useconds = tv.tv_usec;
        std::cout << seconds; printf("%06lu", useconds); std::cout << "\t" << ret << "\t" << payload << std::endl;
        
        // send packet to all receivers
        if(sendto(ipsock, buf, payload, 0, dst, sizeof(struct sockaddr) ) == -1)
        {
            perror("sendto error: ");
            exit(1);
        }

        // sleep intervall time
        boost::system_time sleep_tout;
        do {
            sleep_tout = boost::get_system_time();
            sleep_tout += boost::posix_time::milliseconds(10);
            boost::this_thread::sleep(sleep_tout);
        } while (sleep_tout < tout);

        cur_time = microsec_clock::local_time();
        ++packet_count;
    }
    while (((cur_time - start_time).total_seconds()) < duration);
    free(buf);
    free(hash_buf);
}

void recv (const struct sockaddr *b_addr, const size_t &payload, const size_t &duration)
{
    int ipsock = 0;
    ipsock = socket(AF_INET, SOCK_DGRAM, 0);
    // bind
    if(::bind(ipsock, b_addr, sizeof(struct sockaddr)) == -1)
        perror("bind error");

    char* buf = NULL;
    size_t blen = c_bufsize;
    int recv_size = 0;

    if (blen < payload)
        blen = payload;

    buf = reinterpret_cast<char*>(malloc(blen));
    uint8_t* hash_buf = reinterpret_cast<uint8_t*>(malloc(recv_size));

    uint32_t packet_count = 0;
    ptime start_time;
    ptime cur_time;

    do{
        bzero(buf, blen);
        recv_size = recv(ipsock, buf, blen, 0);
        if (recv_size == -1)
        {
            perror("recv error: ");
            exit(EXIT_FAILURE);
        }
        // stictly start with reception of first packet
        if(start_time.is_not_a_date_time()){
            start_time = microsec_clock::local_time();
        }
        cur_time = microsec_clock::local_time();
        if ((s_offset > recv_size) || (s_length > s_offset)) {
            std::cerr << "Unable to generate packet HASH ID!" << std::endl;
            exit (EXIT_FAILURE);
        }
        // generate packet HASH_ID from buf as impd4e does
        bzero (hash_buf, recv_size);
        memcpy (hash_buf, (buf + (recv_size - s_offset)), s_length);
        uint32_t ret = BOB_Hash(hash_buf, s_length, initval);

        struct timeval tv;
        unsigned long seconds, useconds, timestamp;
        gettimeofday(&tv, NULL);
        seconds = tv.tv_sec;
        useconds = tv.tv_usec;
        
        std::cout << seconds; printf("%06lu", useconds); std::cout << "\t" << ret << "\t" << recv_size << std::endl;
        packet_count++;
    }
    while (((cur_time - start_time).total_seconds()) < duration);
    free(buf);
    free(hash_buf);
}

std::vector<struct sockaddr> read_file (const string &f_recv, const int &port)
{  
    std::set<string> recv_ips;
    std::ifstream ifs( f_recv.c_str() );
    string temp;

    while( getline( ifs, temp ) )
        recv_ips.insert( temp );

    std::vector<struct sockaddr> receivers;
    std::set<string>::iterator it;
    for (it = recv_ips.begin(); it != recv_ips.end(); ++it) {
        struct sockaddr addr;
        struct sockaddr_in *in_addr = (struct sockaddr_in *)&addr;
        in_addr->sin_port = htons(port);
        in_addr->sin_family = AF_INET;
        int ret = inet_pton (AF_INET, it->c_str(), &(in_addr->sin_addr));
        if (ret > 0) {
            receivers.push_back(addr);
        }
    }
    return receivers;
}

int main (int argc, char **argv)
{
    bool receiver = true;
    bool sender = false;
    size_t pkt_interval = c_interval;
    size_t exp_duration = c_time;
    size_t pkt_payload = c_payload;
    string interface ("ANY");
    string filename;
    string destination;
    int port = atoi(c_port.c_str ());
    string program (argv[0]);

    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hf:i:n:p:l:s:t:o:w:",
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0: {
                if (long_options[option_index].flag != 0)
                    break;
                cout << "option " << 
                    long_options[option_index].name;
                if (optarg)
                    cout << " with arg " << optarg;
                cout << endl;
                break;
            }
            case 'f': {
                filename = string(optarg);
                sender = true;
                receiver= false;
                break;
            }
            case 'i': {
                try {
                    pkt_interval = atoi(optarg);
                    if (pkt_interval < 1)
                        throw 42;
                }
                catch (...) {
                    std::cerr << "Invalid packet interval TIME!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'n': {
                interface = string(optarg);
                break;
            }
            case 'p': {
            try {
                port = atoi(optarg);
                if ( (port < 1024) || (pkt_payload > 64*1024) )
                    throw 42;
            }
            catch (...) {
                std::cerr << "Invalid port number!" << std::endl;
                return EXIT_FAILURE;
            }
                break;
            }
            case 'l': {
                try {
                    pkt_payload = atoi(optarg);
                    if ( (pkt_payload < 100) || (pkt_payload > 1400) )
                        throw 42;
                } 
                catch (...) {
                    std::cerr << "Invalid payload size!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 's': {
                destination = string(optarg);
                sender = true;
                receiver = false;
                break;
            }
            case 't': {
                try {
                    exp_duration = atoi(optarg);
                    if ( exp_duration < 1 )
                        throw 42;
                    
                }
                catch(...) {
                    std::cerr << "Invalid time duration!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'o': {
                try {
                    s_offset = atoi(optarg);
                    if ( s_offset < 1 )
                        throw 42;

                }
                catch(...) {
                    std::cerr << "Invalid hash offset!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'w': {
                try {
                    s_length = atoi(optarg);
                    if ( s_length < 1 )
                        throw 42;

                }
                catch(...) {
                    std::cerr << "Invalid hash length!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'h': {
                help (program);
                exit(EXIT_SUCCESS);
                break;
            }
            default: {
                usage (program);
                return EXIT_FAILURE;
                break;
            }
        }
    }

    std::cout << "### Unicast Packet Generator ###" << std::endl;
    std::cout << "# SETTINGS, run as";
    if (sender)
        std::cout << " sender";
    else 
        std::cout << " receiver";

    std::cout << ", duration: " << exp_duration;
    std::cout << ", interval: " << pkt_interval;
    std::cout << ", payload: " << pkt_payload <<std::endl;

    std::vector<struct sockaddr> receivers;
    if (!filename.empty ()) {
        receivers = read_file(filename, port);
    }

    if (sender) {
        if (destination.size() > 0) {
            struct sockaddr_in dst_addr;
            dst_addr.sin_port = htons (port);
            dst_addr.sin_family = AF_INET;
            int ret = inet_pton (AF_INET, destination.c_str(), &dst_addr.sin_addr);
            if (ret > 0) {
                send_dst ((struct sockaddr *)&dst_addr, pkt_payload, exp_duration, pkt_interval);
            }
            else {
                perror ("inet_pton");
                exit (EXIT_FAILURE);
            }
        }
        else if (receivers.size () > 0) {
            send (receivers, pkt_payload, exp_duration, pkt_interval);
        }
        else {
            std::cerr << std::endl;
            std::cerr << "ERROR: Missing parameters for send mode!" << std::endl;
            std::cerr << "       Receiver file or destination needed!" << std::endl;
            std::cerr << std::endl;
            exit(1);
        }
    } 
    else {
        struct sockaddr_in addr_in;
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(port);
        addr_in.sin_addr.s_addr = INADDR_ANY;
        struct sockaddr *addr = new struct sockaddr;
        memcpy (addr, &addr_in, sizeof(addr_in));
        recv (addr, pkt_payload, exp_duration);
    }
    return EXIT_SUCCESS;
}
