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
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#ifndef __APPLE__
    #include <sys/sysinfo.h>
#endif
#include <set>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>

#include <boost/thread.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/lexical_cast.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"

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
using namespace hamcast;

//extern uint32_t BOB_Hash (uint8_t*, uint16_t, uint32_t);

namespace {
    int f_stat = 0;
/* struct to parse long opts */
    struct option long_options[] = {
        {"stat",        no_argument,        &f_stat, 1},
        {"help",        no_argument,        0,  'h'},
        {"interval",    required_argument,  0,  'i'},
        {"network",     required_argument,  0,  'n'},
        {"mode",        required_argument,  0,  'm'},
        {"length",      required_argument,  0,  'l'},
        {"recv",        required_argument,  0,  'r'},
        {"send",        required_argument,  0,  's'},
        {"time",        required_argument,  0,  't'},
        {"offset",      required_argument,  0,  'o'},
        {"width",       required_argument,  0,  'w'},
        {0, 0, 0, 0}
    };

    enum multicast_mode {MODE_HAMCAST, MODE_NATIVE, MODE_UNICAST, MODE_ASYNC_HAMCAST};

    const int c_debug = 0;

    const string c_group ("239.239.239.239");   // default group, DEPRECATED
    const string c_port ("7580");               // default port, DEPRECATED
    const char c_ttl = 64;                      // default multicast ttl
    const size_t c_payload = 1000;              // default payload in bytes
    const size_t c_bufsize = 65001;
    const size_t c_time = 60;                   // default runtime in sec
    const size_t c_interval = 1000;             // default interval in msec
    multicast_mode mode = MODE_HAMCAST;         // default mode is hamcast
    int s_iface = 0;                            // default interface id
    uint16_t s_offset = 70;                     // hash offset
    uint16_t s_length = 50;                     // hash length
    uint32_t s_pkt_count = 0;                   // global packet counter
    uint8_t* hash_buf = NULL;
}   

void usage (const string& program)
{
        cout << "Usage: " << program << " [-hinmowlt] -{r|s} GROUP" << endl;
        cout << endl;
}

void help (const string& program)
{
    usage(program);
    cout << "Options" << endl;
    cout << "  -h, --help" << endl;
    cout << "    Print this help screen." << endl;
    cout << "  -i, --intervall TIME" << endl;
    cout << "    Set packet interval TIME, in msec (1000)" << endl;
    cout << "  -n, --network-if ID" << endl;
    cout << "    Set interface ID, default is 0 = ALL" << endl;
    cout << "  -m, --mode MODE" << endl;
    cout << "    Multicast MODE: hamcast or ip (hamcast)" << endl;
    cout << "  -o, --offset OFFSET" << endl;
    cout << "    Set payload OFFSET to generate packet hash id from (70)." << endl;
    cout << "  -w, --width LEN" << endl;
    cout << "    Set offset LEN to generate hash id from (50)" << endl;
    cout << "  -l, --length LEN" << endl;
    cout << "    Set packet payload LENgth in Bytes (100)" << endl;
    cout << "  -r, --recv GROUP" << endl;
    cout << "    Receive from multicast GROUP" << endl;
    cout << "  -s, --send GROUP" << endl;
    cout << "    Send to multicast GROUP" << endl;
    cout << "  -t, --time RUNTIME" << endl;
    cout << "    Set RUNTIME of experiment, in sec (60)" << endl;
    cout << endl;
    cout << "Notes" << endl;
    cout << "  You MUST provide either option '-r' or '-s' with a valid multicast group URI." << endl;
    cout << "  On send and receive strictly ensure that: OFFSET < PAYLOAD and LENGTH < OFFSET!" << endl;
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

void send (const hamcast::uri &group, const size_t &payload,
           const size_t &duration, const size_t &interval, const bool &stat)
{
    int ipsock = 0;
    hamcast::multicast_socket *hcsock = NULL;
    struct sockaddr *addr = NULL;
    boost::system_time tout;

    if (mode == MODE_NATIVE) {
        ipsock = socket(AF_INET, SOCK_DGRAM, 0);
        if(ipsock == -1)
            perror("socket");
        struct sockaddr_in addr_in;
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(atoi(group.port().c_str()));
        if (inet_pton(AF_INET, group.host().c_str(), &(addr_in.sin_addr)) != 1)
        {
          perror("inet_pton");
          exit(1);
        }
        addr = reinterpret_cast<struct sockaddr*>(&addr_in);

    }
    else {
        hcsock = new multicast_socket();
        if (s_iface > 0) {
            hcsock->set_interface(s_iface);
        }
        hcsock->set_ttl(c_ttl);
    }

    if (payload < sizeof(uint32_t)) {
        std::cerr << "Payload to small, muss be larger than sizeof(uint32_t)!" << std::endl;
        exit(EXIT_FAILURE);
    }
    char* buf = reinterpret_cast<char*>(malloc (payload));
    uint8_t* hash_buf = reinterpret_cast<uint8_t*>(malloc(payload));
    //bzero(buf, payload);

    uint32_t packet_count = 1;
    uint32_t last_pkt_count = s_pkt_count;
    uint32_t pkts_stat = 0;

    ptime start_time = microsec_clock::local_time();
    ptime cur_time = start_time;
    tout = boost::get_system_time();
    tout += boost::posix_time::milliseconds(interval);
    // specify alphabet for random payload
    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;
    do
    {
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

        // write pkt number at start of payload
        uint32_t tmp = htonl(packet_count);
        memcpy(buf, &tmp, sizeof(packet_count));

        // generate packet HASH_ID from buf as impd4e does
        bzero (hash_buf, payload);
        memcpy (hash_buf, (buf + (payload - s_offset)), s_length);
        uint32_t ret = BOB_Hash(hash_buf, s_length, initval);

        if (stat == 1) {
            ++s_pkt_count;
        }
        else {
            struct timeval tv;
            unsigned long seconds, useconds, timestamp;
            gettimeofday(&tv, NULL);
            seconds = tv.tv_sec;
            useconds = tv.tv_usec;
            timestamp = (seconds * 1000000) + useconds;
            //std::cout << timestamp << "\t" << ret << std::endl;
            std::cout << seconds; printf("%06lu", useconds); std::cout << "\t" << ret << "\t" << payload << std::endl;
        }

        if(mode == MODE_NATIVE) {
            if(sendto(ipsock, buf, payload, 0, addr, sizeof(struct sockaddr) ) == -1)
            {
                perror("sendto error: ");
                exit(1);
            }
        }
        else {
            hcsock->send(group, payload, buf);
        }

        cur_time = microsec_clock::local_time();
        if (stat ==  0) {
            tout = boost::get_system_time();
            tout += boost::posix_time::milliseconds(interval);
            boost::system_time sleep_tout;
            do {
                sleep_tout = boost::get_system_time();
                sleep_tout += boost::posix_time::milliseconds(10);
                boost::this_thread::sleep(sleep_tout);
            } while (sleep_tout < tout);
        }
        else {
            if (cur_time > tout) { // stat timer expired
                pkts_stat = s_pkt_count - last_pkt_count;
                std::cout << interval << "\t" << pkts_stat << std::endl;
                last_pkt_count = s_pkt_count;
                tout = boost::get_system_time();
                tout += boost::posix_time::milliseconds(interval);
            }
        }
        cur_time = microsec_clock::local_time();
        ++packet_count;
    }
    while (((cur_time - start_time).total_seconds()) < duration);
    free(buf);
    free(hash_buf);
}

uint32_t packet_hash (const size_t buflen, const char* buf)
{
    memcpy (hash_buf, (buf + (buflen - s_offset)), s_length);
    return BOB_Hash(hash_buf, s_length, initval);
}

void handle_packet(const uri&, size_t recv_size, const void* buf)
{
    if ((s_offset > recv_size) || (s_length > s_offset)) {
        std::cerr << "# Unable to generate packet HASH ID!" << std::endl;
    }
    else {
        uint32_t ret = packet_hash (recv_size, reinterpret_cast<const char*>(buf));
        struct timeval tv;
        unsigned long seconds, useconds, timestamp;
        gettimeofday(&tv, NULL);
        seconds = tv.tv_sec;
        useconds = tv.tv_usec;
        timestamp = (seconds * 1000000) + useconds;
        std::cout << seconds; printf("%06lu", useconds); std::cout << "\t" << ret << "\t" << recv_size << std::endl;
    }
}

void hc_async_recv (const hamcast::uri &group, const size_t &duration,
                    const size_t &interval, const bool &stat)
{
    // generate packet HASH_ID from buf as impd4e does
    hash_buf = new uint8_t[s_length];
    async_multicast_socket ms(handle_packet);
    ms.join(group);

    ptime start_time = microsec_clock::local_time();
    ptime cur_time = microsec_clock::local_time();
    uint32_t pkts_stat = 0;
    uint32_t last_pkt_count = s_pkt_count;

    while ( (s_pkt_count == 0) || (((cur_time - start_time).total_seconds()) < duration) ) {
        if (stat && (s_pkt_count != 0)) {

        }

    }
    delete hash_buf;
}

void recv (const hamcast::uri &group, const size_t &payload,
           const size_t &duration, const size_t &interval, const bool &stat)
{
    int ipsock = 0;
    hamcast::multicast_socket *hcsock = NULL;
    boost::system_time tout;

    if (mode == MODE_NATIVE) {
        ipsock = socket(AF_INET, SOCK_DGRAM, 0);
        struct sockaddr_in addr_in;
        addr_in.sin_family = AF_INET;
        addr_in.sin_port = htons(atoi(group.port().c_str()));
        addr_in.sin_addr.s_addr = INADDR_ANY;
        if(::bind(ipsock, reinterpret_cast<struct sockaddr*>(&addr_in), sizeof(addr_in)) == -1)
            perror("bind error");

        struct group_req mgroup;
        mgroup.gr_interface = 0; //kernel choose interface
        reinterpret_cast<struct sockaddr_in*>(&mgroup.gr_group)->sin_family = AF_INET;
        if ((inet_pton(AF_INET, group.host().c_str(), &(reinterpret_cast<struct sockaddr_in*>(&mgroup.gr_group)->sin_addr))) != 1)
        {
            std::cout << "group: " << group.host() << std::endl;
            perror("inet_pton");
            exit(1);
        }
        if(setsockopt(ipsock, IPPROTO_IP, MCAST_JOIN_GROUP, &mgroup, sizeof(mgroup)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

    }
    else {
        hcsock = new multicast_socket();
        if (s_iface > 0) {
            hcsock->set_interface(s_iface);
        }
        hcsock->join(group);
    }
    
    char* buf = NULL;
    size_t blen = c_bufsize;
    int recv_size = 0;

    if (blen < payload)
        blen = payload;

    buf = reinterpret_cast<char*>(malloc(blen));
    hash_buf = reinterpret_cast<uint8_t*>(malloc(s_length));

    uint32_t packet_count = 0;
    uint32_t pktno_recv = 0;
    uint32_t pkts_stat = 0;
    uint32_t last_pkt_count = s_pkt_count;
    std::set<uint32_t> pkts_lost;
    std::set<uint32_t>::iterator pit;
    ptime start_time;
    ptime cur_time;

    do{
        bzero(buf, blen);
        if(mode == MODE_NATIVE) {
            recv_size = recv(ipsock, buf, blen, 0);
            if (recv_size == -1)
            {
                perror("recv error: ");
                exit(EXIT_FAILURE);
            }
        }
        else {
            multicast_packet p = hcsock->receive();
            memcpy(buf, p.data(), p.size());
            recv_size = p.size();
        }
        uint32_t tmp;
        memcpy(&tmp, buf, sizeof(tmp));
        uint32_t pktno = ntohl(tmp);
        if (pktno == (pktno_recv+1)) {
            pktno_recv = pktno;
        }
        else if (pktno > (pktno_recv+1)) {
            for (uint32_t i=pktno_recv+1; i < pktno; ++i) {
                pkts_lost.insert(i);
            }
        }
        else if (pktno < pktno_recv) {
            if ( (pit = pkts_lost.find(pktno)) != pkts_lost.end() ) {
                    pkts_lost.erase(pit);
            }
        }
        // stictly start with reception of first packet
        if(start_time.is_not_a_date_time()){
            start_time = microsec_clock::local_time();
            tout = boost::get_system_time();
            tout += boost::posix_time::milliseconds(interval);
        }
        cur_time = microsec_clock::local_time();
        if (stat == 1) {
            ++s_pkt_count;
            if (cur_time > tout) {
                pkts_stat = s_pkt_count - last_pkt_count;
                std::cout << interval << "\t" << pkts_stat << std::endl;
                last_pkt_count = s_pkt_count;
                tout = boost::get_system_time();
                tout += boost::posix_time::milliseconds(interval);
            }
        }
        else {
            if ((s_offset > recv_size) || (s_length > s_offset)) {
                std::cerr << "# Unable to generate packet HASH ID!" << std::endl;
            }
            else {
                // generate packet HASH_ID from buf as impd4e does
                memcpy (hash_buf, (buf + (recv_size - s_offset)), s_length);
                uint32_t ret = BOB_Hash(hash_buf, s_length, initval);

                struct timeval tv;
                unsigned long seconds, useconds, timestamp;
                gettimeofday(&tv, NULL);
                seconds = tv.tv_sec;
                useconds = tv.tv_usec;
                timestamp = (seconds * 1000000) + useconds;
        
                std::cout << seconds; printf("%06lu", useconds); std::cout << "\t" << ret << "\t" << recv_size << std::endl;
            }
        }
        packet_count++;
    }
    while (((cur_time - start_time).total_seconds()) < duration);
    free(buf);
    free(hash_buf);
}

int main (int argc, char **argv)
{
    bool receiver = false;
    bool sender = false;
    size_t pkt_interval = c_interval;
    size_t exp_duration = c_time;
    size_t bandwidth = 0;
    size_t pkt_payload = c_payload;

    std::string group = "ip://" + c_group + ":" + c_port;

    string program (argv[0]);

    if (argc < 2) {
        usage(program);
        return EXIT_FAILURE;
    }


    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hi:n:m:l:r:s:t:o:w:",
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
                try {
                    s_iface = atoi(optarg);
                    if (s_iface < 1)
                        throw 42;
                }
                catch (...) {
                    std::cerr << "Invalid network interface ID!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'm': {
                std::string tmp (optarg);
                if (tmp == "ip")
                    mode = MODE_NATIVE;
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
            case 'r': {
                if (sender) {
                    std::cerr << "Conflicting options -s and -r!" << std::endl;
                    return EXIT_FAILURE;
                } else {
                    receiver = true;
                }
                group = string (optarg);
                if (hamcast::uri(group).empty()) {
                    std::cerr << "Invalid multicast group!" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            }
            case 's': {
                if (receiver) {
                    std::cerr << "Conflicting options -s and -r!" << std::endl;
                    return EXIT_FAILURE;
                } else {
                    sender = true;
                }
                group = string (optarg);
                if (hamcast::uri(group).empty()) {
                    std::cerr << "Invalid multicast group!" << std::endl;
                    return EXIT_FAILURE;
                }
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

    hamcast::uri mcast_group (group);
    std::cout << "### HAMcast Packet Generator #" << std::endl;
    std::cout << "# SETTINGS, run as";
    if (sender)
        std::cout << " sender";
    else 
        std::cout << " receiver";

    std::cout << ", group: " << mcast_group.str() << ", duration: " << exp_duration;

    if (bandwidth > 0) {
        bandwidth /= 100;
        bandwidth *= 100;
        pkt_payload = 1280;
        std::cout << ", bandwidth: " << bandwidth;
    }
    else {
        std::cout << ", interval: " << pkt_interval;
    }

    std::cout << ", payload: " << pkt_payload;

    if (mode == MODE_NATIVE)
        std::cout << ", mode: IP" << std::endl;
    else
        std::cout << ", mode: HAMcast" << std::endl;

    if (sender) {
        send (mcast_group, pkt_payload, exp_duration, pkt_interval, f_stat);
    } 
    else {
        switch (mode) {
//        case MODE_NATIVE:
//            recv (mcast_group, pkt_payload, exp_duration, pkt_interval, f_stat);
//            break;
        case MODE_ASYNC_HAMCAST:
            hc_async_recv (mcast_group, exp_duration, pkt_interval, f_stat);
            break;
        default:
            recv (mcast_group, pkt_payload, exp_duration, pkt_interval, f_stat);
            //std::cerr << "Unknown Mode" << std::endl;
        }
    }
    return EXIT_SUCCESS;
}
