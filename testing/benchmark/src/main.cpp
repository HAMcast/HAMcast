//Last modified: 04/04/11 12:35:55(CEST) by Fabian Holler

#include "hamcast/hamcast.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <boost/scoped_ptr.hpp>
#include "getusage.c"
#include "pidof.c"

using namespace hamcast;
using namespace boost;
using namespace std;
using namespace boost::gregorian;
using namespace boost::posix_time;

enum mode {HAMCAST_ASYNC, HAMCAST, NATIVE, MODE_UNSET};
enum action {SEND, RECV, ACTION_UNSET};

uint32_t cur_packet_nr = 0;
uint32_t last_packet_nr = 0;
uint32_t packet_count = 0;
uint32_t duplo3_count = 0;
uint32_t s_num_groups = 1;
string s_base_ip("239.0.0.1");

char* buf;

ptime start_time;
ptime last_output;

void handle_buffer(const char* buffer, const size_t buflen)
{
    last_packet_nr = cur_packet_nr;
    if (buflen < sizeof(cur_packet_nr)) {
        cerr << "Packet to small!" << endl;
        return;
    }
    uint32_t tmp_packet_nr;
    memcpy(&tmp_packet_nr, buffer, sizeof(tmp_packet_nr));
    if (tmp_packet_nr > cur_packet_nr) {
        cur_packet_nr = tmp_packet_nr;
    }
    else {
        duplo3_count++;
    }
    packet_count++;
}

//void handle_packet(const multicast_packet& mp)
void handle_packet(const hamcast::uri&, size_t buflen, const void* buf)
{
    if(start_time.is_not_a_date_time()){
        start_time = microsec_clock::local_time();
        last_output = microsec_clock::local_time();
    }
    handle_buffer ((char*)buf, buflen);
}

vector<string> generate_group_ips (const string ip, const uint32_t l_num_groups)
{
    vector<string> group_ips;
    uint32_t num_groups = l_num_groups;
    if (num_groups > 1) {
        group_ips.push_back (s_base_ip);
        uint32_t base_ip;
        struct in_addr buf;
        int ret = inet_pton (AF_INET, s_base_ip.c_str (), &buf);
        if (ret < 1) {
            if (ret == 0)
                fprintf(stderr, "Not in presentation format");
            else
                perror("inet_pton");
            exit(EXIT_FAILURE);
        }
        base_ip = ntohl(buf.s_addr);
        num_groups += (num_groups/255);
        num_groups += (num_groups/256);
        for (uint32_t i=1; i < num_groups; i++) {
            if (i>1) {
                uint32_t m255 = (i+2)%256;
                uint32_t m256 = (i+1)%256;
                //cout << "m255: " << m255 << ", m256: " << m256 << endl;
                if ((m255 == 0) || (m256 == 0))
                    continue;
            }
            char next_ip[INET_ADDRSTRLEN];
            buf.s_addr = htonl(base_ip + i);
            if (inet_ntop(AF_INET, (const void*)&buf, next_ip, INET_ADDRSTRLEN) == NULL) {
                std::cerr << "Failed to convert IPv4 address to string!" << std::endl;
            }
            else {
                group_ips.push_back (string(next_ip));
            }
            //cout << "i: " << i << ", ip: " << next_ip << endl;
        }
    }
    else {
        group_ips.push_back (ip);
    }
    for (uint32_t j=1; j <= group_ips.size (); j++) {
        cout << "Group " << j << " : " << group_ips[j-1] << endl;
    }
    return group_ips;
}

void recv(const mode mode, const string ip, const int port, const unsigned int buf_size, const int duration){
    int maxfd=0;
    fd_set readset;
    fd_set readset_org;
    FD_ZERO(&readset);
    FD_ZERO(&readset_org);
    hamcast::multicast_socket* hamcast_socket;
    hamcast::async_multicast_socket* async_hamcast_socket;
    uri hamcast_uri;

    vector<string> group_ips;
    group_ips = generate_group_ips (ip, s_num_groups);
    vector<int> msocks;
    //init sockets
    if(mode == NATIVE) {
        for (uint32_t k=0; k < group_ips.size (); k++) {
            cout << "IP: " << group_ips[k] << endl;
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port+k);
            //addr.sin_addr.s_addr = INADDR_ANY;
            if ((inet_pton(AF_INET, group_ips[k].c_str(), &(reinterpret_cast<struct sockaddr_in*>(&addr.sin_addr)->sin_addr))) != 1) {
                perror("inet_pton");
                exit(1);
            }
            if(::bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1)
                perror("bind error");

            struct group_req mgroup;
            mgroup.gr_interface = 0; //kernel choose interface
            reinterpret_cast<struct sockaddr_in*>(&mgroup.gr_group)->sin_family = AF_INET;
            if ((inet_pton(AF_INET, group_ips[k].c_str(), &(reinterpret_cast<struct sockaddr_in*>(&mgroup.gr_group)->sin_addr))) != 1) {
                perror("inet_pton");
                exit(1);
            }
            if(setsockopt(sockfd, IPPROTO_IP, MCAST_JOIN_GROUP, &mgroup, sizeof(mgroup)) == -1) {
                perror("setsockopt");
                exit(1);
            }
            msocks.push_back (sockfd);
            FD_SET(sockfd, &readset_org);
            if (sockfd > maxfd) {
                maxfd=sockfd;
            }
        }
    }
    else if(mode == HAMCAST) {
        hamcast_socket = new hamcast::multicast_socket();
        for (uint32_t k=0; k<s_num_groups; k++) {
            hamcast_uri = uri("ip://" + group_ips[k] + ":" + lexical_cast<string>(port));
            hamcast_socket->join(hamcast_uri);
        }
    }
    else if (mode == HAMCAST_ASYNC) {
        async_hamcast_socket = new hamcast::async_multicast_socket(handle_packet);
        for (uint32_t k=0; k<s_num_groups; k++) {
            hamcast_uri = uri("ip://" + group_ips[k] + ":" + lexical_cast<string>(port));
            async_hamcast_socket->join(hamcast_uri);
        }
    }
    else {
        cerr << "Receive Mode unset!" << endl;
        exit(1);
    }
    buf = (char*) malloc (buf_size);

    cur_packet_nr = 0;
    last_packet_nr = 0;
    packet_count = 0;

    pid_t mypid = getpid();

    //get initial cpu usage
    struct pstat cur_cpu_app;
    if(get_usage(mypid, &cur_cpu_app) == -1)
        cerr << "getusage error" << endl;
    struct pstat last_cpu_app;

    int middleware_pid = 0;
    struct pstat cur_cpu_middleware, last_cpu_middleware;
    if((mode == HAMCAST) || (mode == HAMCAST_ASYNC)){
        middleware_pid = pidof("middleware");
        if(middleware_pid == -1)
        {
            cerr << "couldn't obtain middleware pid" << endl;
        }
        if(get_usage(middleware_pid, &cur_cpu_middleware) == -1)
            cerr << "getusage error" << endl;
    }
    uint32_t current_packet_count = 0;
    uint32_t current_packet_loss = 0;
    uint32_t last_packet_count = 0;
    uint32_t last_packet_loss = 0;
    ptime cur_time;

    printf("%-4s\t%-7s\t%s\t%s\t%s\t%s\t%s\n", "#runtime(milliseconds)", "packets", "loss", "app_user_cpu_usage", "app_kernel_cpu_usage", "middleware_user_cpu_usage", "middleware_kernel_cpu_usage");
    do {
        bzero(buf, buf_size);
        if(mode == NATIVE) {
            memcpy(&readset, &readset_org, sizeof(readset_org));
            if (select(maxfd + 1, &readset, 0, 0, 0) < 0) {
                // must not happen
                HC_LOG_FATAL("select() call failed");
                return;
            }
            for (int s=0; s < maxfd+1; ++s) {
                if (FD_ISSET(s, &readset)) {
                    int rtval = recv(s, buf, buf_size, 0);
                    if (rtval == -1) {
                        perror("recv error: ");
                        exit(1);
                    }
                    handle_buffer (buf,buf_size);
                }
            }
        }
        else if(mode == HAMCAST) {
            multicast_packet p = hamcast_socket->receive();
            memcpy (buf, p.data(), buf_size);
            handle_buffer (buf, buf_size);
        }
        else if (mode == HAMCAST_ASYNC) {
            usleep (1000);
        }
        else {
            cerr << "How did you get here? This was not supposed to happen!" << endl;
            exit(1);
        }

        //count start-time after first recv()
        if(start_time.is_not_a_date_time()) {
            // this is a hack for the new async multicast sockets
            if (mode == HAMCAST_ASYNC) {
                continue;
            }
            else {
                start_time = microsec_clock::local_time();
                last_output = microsec_clock::local_time();
            }
        }

        cur_time = microsec_clock::local_time();
        long time_diff = (cur_time - last_output).total_milliseconds();

        if(time_diff >= 1000) {
            current_packet_count = packet_count;
            current_packet_loss = cur_packet_nr - packet_count;

            float ucpu_usage_app, scpu_usage_app = 0;
            float scpu_usage_middleware = 0;
            float ucpu_usage_middleware = 0;
            last_cpu_app = cur_cpu_app;
            last_cpu_middleware = cur_cpu_middleware;
            if(get_usage(mypid, &cur_cpu_app) == -1) {
                cerr << "getusage error" << endl;
            }
            calc_cpu_usage(&cur_cpu_app, &last_cpu_app, &ucpu_usage_app, &scpu_usage_app);

            if((mode == HAMCAST) || (mode == HAMCAST_ASYNC)) {
                if(get_usage(middleware_pid, &cur_cpu_middleware) == -1) {
                    cerr << "getusage error" << endl;
                }
                calc_cpu_usage(&cur_cpu_middleware, &last_cpu_middleware, &ucpu_usage_middleware, &scpu_usage_middleware);
            }
            else {
                //2tes mal app CPU usage berechnen, damit selbe Anzahl v. Rechenoperationen wie bei HAMCAST erzeugt werden
                if(get_usage(mypid, &cur_cpu_app) == -1) {
                    cerr << "getusage error" << endl;
                }
                calc_cpu_usage(&cur_cpu_app, &last_cpu_app, &ucpu_usage_app, &scpu_usage_app);
            }

#ifdef __x86_64
            printf("%-4lld\t%-7u\t%-6u\t%-2.3f\t%-2.3f\t%-2.3f\t%-2.3f\n", (cur_time - start_time).total_milliseconds(), (current_packet_count - last_packet_count), (current_packet_loss - last_packet_loss), ucpu_usage_app, scpu_usage_app, ucpu_usage_middleware, scpu_usage_middleware); //64bit Arch
#else
            printf("%-4lld\t%-7u\t%-6u\t%-2.3f\t%-2.3f\t%-2.3f\t%-2.3f\n", (cur_time - start_time).total_milliseconds(), (current_packet_count - last_packet_count), (current_packet_loss - last_packet_loss), ucpu_usage_app, scpu_usage_app, ucpu_usage_middleware, scpu_usage_middleware); //32bit Arch
#endif
            last_output = microsec_clock::local_time();
            last_packet_loss = current_packet_loss;
            last_packet_count = current_packet_count;
        }
    } while ((((cur_time - start_time).total_seconds()) <= duration) || (packet_count == 0));
    cerr << " - recv: time is up!" << endl;
    cerr << " - stat: pkts_recv = " << packet_count << ", pkts_lost = " << (cur_packet_nr - packet_count) << ", pkts_dup = " << duplo3_count << endl;
    // cleanup pointer if necessary
    if (mode == HAMCAST) {
        delete hamcast_socket;
    }
    if (mode == HAMCAST_ASYNC) {
        delete async_hamcast_socket;
    }
}


void inline native_send(int& sockfd, char* buf, int buf_size, struct sockaddr* addr, int addr_size)
{
    if(sendto(sockfd, buf, buf_size, 0, addr, addr_size) == -1) {
        perror("sendto error: ");
        exit(1);
    }
}

void send(const mode mode, const string ip, const int port, const unsigned int buf_size, const int duration)
{
    int native_socket;
    hamcast::multicast_socket *hamcast_socket;
    uri hamcast_uri;
    struct sockaddr* addr;

    vector<string> group_ips = generate_group_ips (ip, s_num_groups);
    vector<struct sockaddr_in> ipas;
    vector<uri> uris;
    //init sockets
    if(mode == NATIVE) {
        native_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if(native_socket == -1)
            perror("socket");
        struct sockaddr_in addr_in;
        addr_in.sin_family = AF_INET;
        for (size_t k=0; k < group_ips.size (); k++) {
            addr_in.sin_port = htons(port+k);
            if (inet_pton(AF_INET, group_ips[k].c_str(), &(addr_in.sin_addr)) != 1) {
                perror("inet_pton");
                exit(1);
            }
            ipas.push_back (addr_in);
            addr = reinterpret_cast<struct sockaddr*>(&addr_in);
        }
    }
    else if((mode == HAMCAST) || (mode == HAMCAST_ASYNC)) {
        hamcast_socket = new hamcast::multicast_socket();
        for (size_t k=0; k < group_ips.size (); k++) {
            hamcast_uri = uri("ip://" + group_ips[k] + ":" + lexical_cast<string>(port));
            uris.push_back (hamcast_uri);
        }
    }

    char *buf = new char[buf_size];
    bzero(buf, buf_size);

    uint32_t packet_nr = 1;
    unsigned long packet_count = 0;
    ptime start_time = microsec_clock::local_time();
    ptime last_output = microsec_clock::local_time();
    ptime cur_time;
    pid_t mypid = getpid();
    int middleware_pid = 0;

    struct pstat cur_cpu_app;
    struct pstat last_cpu_app;
    struct pstat cur_cpu_middleware, last_cpu_middleware;

    if(get_usage(mypid, &cur_cpu_app) == -1) {
        cerr << "getusage error" << endl;
    }

    if((mode == HAMCAST) || (mode == HAMCAST_ASYNC)){
        middleware_pid = pidof("middleware");
        if(middleware_pid == -1) {
            cerr << "couldn't obtain middleware pid" << endl;
        }

        if(get_usage(middleware_pid, &cur_cpu_middleware) == -1) {
            cerr << "getusage error" << endl;
        }
    }

    printf("%-4s\t%-7s\t%s\t%s\t%s\t%s\t%s\n", "#runtime(milliseconds)", "packets", "loss", "app_user_cpu_usage", "app_kernel_cpu_usage", "middleware_user_cpu_usage", "middleware_kernel_cpu_usage");
    do {
        if(mode == NATIVE) {
            for (size_t k=0; k < ipas.size (); k++) {
                memcpy(buf, &packet_nr, sizeof(packet_nr));
                struct sockaddr_in sai = ipas[k];
                addr = reinterpret_cast<struct sockaddr*>(&sai);
                if(sendto(native_socket, buf, buf_size, 0, addr, sizeof(struct sockaddr)) == -1) {
                    perror("sendto error: ");
                    exit(1);
                }
                packet_nr++;
                packet_count++;
            }
        }
        else {
            for (size_t k=0; k < uris.size (); k++) {
                memcpy(buf, &packet_nr, sizeof(packet_nr));
                hamcast_socket->send (uris[k], buf_size, buf);
                packet_nr++;
                packet_count++;
            }
        }
        cur_time = microsec_clock::local_time();

        long time_diff = (cur_time - last_output).total_milliseconds();
        if(time_diff >= 1000){
            float ucpu_usage_app, scpu_usage_app = 0;
            float scpu_usage_middleware = 0;
            float ucpu_usage_middleware = 0;
            last_cpu_app = cur_cpu_app;
            last_cpu_middleware = cur_cpu_middleware;
            if(get_usage(mypid, &cur_cpu_app) == -1)
                cerr << "getusage error" << endl;
            calc_cpu_usage(&cur_cpu_app, &last_cpu_app, &ucpu_usage_app, &scpu_usage_app);

            if((mode == HAMCAST) || (mode == HAMCAST_ASYNC)) {
                if(get_usage(middleware_pid, &cur_cpu_middleware) == -1)
                    cerr << "getusage error" << endl;
                calc_cpu_usage(&cur_cpu_middleware, &last_cpu_middleware, &ucpu_usage_middleware, &scpu_usage_middleware);
            }
            else {
                //2tes mal app CPU usage berechnen, damit selbe Anzahl v. Rechenoperationen wie bei HAMCAST erzeugt werden
                if(get_usage(mypid, &cur_cpu_app) == -1)
                    cerr << "getusage error" << endl;
                calc_cpu_usage(&cur_cpu_app, &last_cpu_app, &ucpu_usage_app, &scpu_usage_app);
            }

#ifdef __x86_64
            printf("%-4lld\t%-7lu\t%-6lu%-2.3f\t%-2.3f\t%-2.3f\t%-2.3f\n", (cur_time - start_time).total_milliseconds(), packet_count, 0L, ucpu_usage_app, scpu_usage_app, ucpu_usage_middleware, scpu_usage_middleware);
#else
            printf("%-4lld\t%-7lu\t%-6lu%-2.3f\t%-2.3f\t%-2.3f\t%-2.3f\n", (cur_time - start_time).total_milliseconds(), packet_count, 0L, ucpu_usage_app, scpu_usage_app, ucpu_usage_middleware, scpu_usage_middleware);
#endif
            last_output = cur_time;
            packet_count=0;
        }

    }
    while (((cur_time - start_time).total_seconds()) <= duration);
    cerr << " - send: time is up!" << endl;
    cerr << " - stat: pkts_send = " << packet_nr << endl;
    // cleanup pointer if necessary
    if ((mode == HAMCAST) || (mode == HAMCAST_ASYNC)) {
        delete hamcast_socket;
    }
    delete buf;
}


int main(int argc, char *argv[])
{
    unsigned int payload_size = 512;
    int duration = 60; //seconds
    string group_ip = "239.0.0.1";
    unsigned int port = 1234;
    mode mode = MODE_UNSET;
    action action = ACTION_UNSET;

    int opt;

    if(argc == 1) {
        cout<<"usage:"<<endl;
        cout <<"-a\t\tmode hamcast_async"<<endl;
        cout <<"-h\t\tmode hamcast"<<endl;
        cout <<"-n\t\tmode native"<<endl;
        cout <<"-s\t\taction send"<<endl;
        cout <<"-r\t\taction receive"<<endl;
        cout <<"-b\t\tpayload size (default 512)"<<endl;
        cout <<"-d\t\tduration in seconds (default 60s)"<<endl;
        cout <<"-i\t\tgroup IP-addr (default 239.0.0.1)"<<endl;
        cout <<"-p\t\tport number (default 1234)"<<endl;
        exit(1);
    }
    else {
        while((opt = getopt(argc, argv, "hanrsd:p:i:d:b:g:")) != -1) {
            switch(opt) {
                case 'a':
                    mode=HAMCAST_ASYNC;
                    break;
                case 'i':
                    group_ip = optarg;
                    break;
                case 'h':
                    mode=HAMCAST;
                    break;
                case 'n':
                    mode=NATIVE;
                    break;
                case 'b':
                    payload_size = atoi(optarg);
                    break;
                case 'p':
                    port = atoi(optarg);
                    break;
                case 's':
                    action = SEND;
                    break;
                case 'r':
                    action = RECV;
                    break;
                case 'd':
                    duration = atoi(optarg);
                    break;
                case 'g':
                    s_num_groups = atoi (optarg);
                    break;
            default:
                cerr << "Unknown parameter!" << endl;
                break;
            }
        }
        //generate_group_ips (group_ip, s_num_groups);
        // check parameters
        if(action == ACTION_UNSET) {
            cerr << "missing argument -r or -s" << endl;
            exit(1);
        }
        if(mode == MODE_UNSET) {
            cerr << "missing argument -a, -h or -n" << endl;
            exit(1);
        }

        if(action == SEND) {
            send(mode, group_ip, port, payload_size, duration);
            cerr << "*** SEND DONE ***" << endl;
        }
        else if(action == RECV) {
            recv(mode, group_ip, port, payload_size, duration);
            cerr << "*** RECV DONE ***" << endl;
        }
        else {
            cerr << "How did you get here? This was not supposed to happen!" << endl;
        }
    }
    free(buf);
}
