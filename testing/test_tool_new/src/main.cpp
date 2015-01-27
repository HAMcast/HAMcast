#include "hamcast/hamcast.hpp"
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <set>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>


#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace hamcast;

enum multicast_mode {MODE_HAMCAST, MODE_NATIVE};

std::string role="receiver";
std::string group="ariba://myuri.de:1234";
//time in seconds
int ival=1;
int lifespan=10;
int payloadsize=100; //byte
const size_t c_bufsize = 65001;
multicast_mode mode=MODE_HAMCAST;

struct option long_options[] = {
        {"interval",    required_argument,  0,  'i'},
        {"group",       required_argument,  0,  'g'},
        {"payload",     required_argument,  0,  'p'},
        {"recv",        required_argument,  0,  'r'},
        {"send",        required_argument,  0,  's'},
        {"time",        required_argument,  0,  't'},
        {"mode",        required_argument,  0,  'm'},
        {0, 0, 0, 0}
    };



void sender (const hamcast::uri &group, const size_t &payload,
      const size_t &duration, const size_t &interval)
{
    struct sockaddr *addr = NULL;
    hamcast::multicast_socket* hcsock = NULL;
    char* buf = reinterpret_cast<char*>(malloc(payloadsize));
    time_t start, cur_time;
    uint32_t packet_count=1;
    time(&start);
    int ipsock = 0;
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

    }else{
        hcsock=new hamcast::multicast_socket();
    }

    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;

    while(difftime(cur_time,start) < duration){


        srand ( time(NULL) );
        for (size_t i=0; i < payload; ++i){
                buf[i] = abc[((rand()%len)+1)];
        }

        // write pkt number at start of payload
        uint32_t tmp = htonl(packet_count);
        memcpy(buf, &tmp, sizeof(packet_count));

        if(mode == MODE_NATIVE) {
            if(sendto(ipsock, buf, payload, 0, addr, sizeof(struct sockaddr) ) == -1)
                {
                    perror("sendto error: ");
                    exit(1);
                }
            }
            else {
            hcsock->send(group,payloadsize,buf);
        }

        packet_count++;
        time(&cur_time);
    }
    free(buf);
	std::cout << "packete: " << packet_count << std::endl;



}


void receiver(const hamcast::uri &group, const size_t &payload,
              const size_t &duration, const size_t &interval)
{
    int ipsock = 0;
    bool started=false;
    ptime temp, to_end, cur_time;
    multicast_socket* hcsock=NULL;

    uint32_t p_all=0;
    uint32_t p_ival=0;
    uint32_t pktno_recv = 0;

    std::vector<double>packages_per_ival;

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

    hcsock=new multicast_socket();
    hcsock->join(group);
    std::cout << "joined group: "<< group << std::endl;
    }

    char* buf=NULL;
    size_t blen = c_bufsize;
    int recv_size = 0;

    if (blen < payload)
    {
        blen = payload;
    }
    buf = reinterpret_cast<char*>(malloc(blen));

    std::set<uint32_t> pkts_lost;
    std::vector<uint32_t> pkts_dup;
    std::set<uint32_t>::iterator pit;


    while(true){

        if(mode == MODE_NATIVE) {
                    std::cout << "waiting for receive (ip)..." << std::endl;
		    recv_size = recv(ipsock, buf, blen, 0);
                    if (recv_size == -1)
                    {
                        perror("recv error: ");
                        exit(EXIT_FAILURE);
                    }
                }
        else {
	   
           multicast_packet p=hcsock->receive();
           if(!started){
               temp=boost::get_system_time();
               to_end=boost::get_system_time();
               temp=temp+boost::posix_time::seconds(ival);
               to_end=to_end+boost::posix_time::seconds(lifespan);



               started=true;
           }
   //        std::cout << "received a package\n";

           memcpy(buf, p.data(), p.size());
           recv_size = p.size();
        }
        uint32_t tmp;
             memcpy(&tmp, buf, sizeof(tmp));
             uint32_t pktno = ntohl(tmp);
             if (pktno == (pktno_recv+1)) {
                 pktno_recv = pktno;
                // std::cout << "paketno: " << pktno << std::endl;
             }

             else if (pktno > (pktno_recv+1)) {
                 for (uint32_t i=pktno_recv+1; i < pktno; ++i) {
                     pkts_lost.insert(i);
                 }
                 pktno_recv = pktno;
             }
             else if (pktno <= pktno_recv) {
                 if ( (pit = pkts_lost.find(pktno)) != pkts_lost.end() ) {
                         pkts_lost.erase(pit);
                 }
             else {
             pkts_dup.push_back(pktno);
             }
             }

           p_all++;
           p_ival++;

           cur_time = boost::get_system_time();
         //  std::cout << "cur_time: " << cur_time << std::endl;
         //  std::cout << "temp: "<< temp <<std::endl;
           if(cur_time >= temp){
            //   uint32_t time=temp.seconds();
               time_duration dur=cur_time-temp;
               double packed_in_ival=(p_ival*1000)/((boost::posix_time::seconds(ival)+dur).total_milliseconds());
               packages_per_ival.push_back(packed_in_ival);

               p_ival=0;
               temp=boost::get_system_time();
               temp+=boost::posix_time::seconds(ival);

           }

           if(cur_time >= to_end ){
               std::cout << "total package count: " << p_all << std::endl;
               for(int i=0;i<packages_per_ival.size();i++){
                   std::cout << "ival "<< i <<" : "<< packages_per_ival.at(i)<< " packages" << std::endl;
               }
               exit(0);

           }

    }

}

int main(int argc, char** argv){

    //parsing arguments
    while(true){
    int option_index=0;
    int c=getopt_long(argc, argv, "srg:i:t:p:m:", long_options, &option_index);
    if(c<=-1){
        break;
    }

    switch(c){
        case 's':
            role="sender";
            break;
        case 'r':
            role="receiver";
            break;
        case 'g':
            group=std::string(optarg);
            break;
        case 'i':
            ival=atoi(optarg);
            break;
        case 'p':
            payloadsize=atoi(optarg);
            break;
        case 't':
            lifespan=atoi(optarg);
            break;
        case 'm': {
            std::string tmp (optarg);
            if (tmp == "ip")
                mode = MODE_NATIVE;
            break;
        }
        default:
            std::cout << "unknown argument";
            return EXIT_FAILURE;
    }


    }
    std::cout << role << std::endl;
    std::cout << "interval: " << ival << std::endl;
    std::cout << "group: "  << group << std::endl;
    std::cout << "payload: "  << payloadsize << std::endl;
    std::cout << "time: "   << lifespan << std::endl;

    hamcast::uri mgroup(group);

    if(role=="sender"){
        sender(group,payloadsize,lifespan,ival);
    }else{
        receiver(group,payloadsize,lifespan,ival);
    }

    return 0;
}





