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

#include <arpa/inet.h>
#include <sys/socket.h>


#include "Receiver.h"
#include "ariba/ariba.h"
#include "ariba/utility/system/StartupWrapper.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include "main.h"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

enum multicast_mode {MODE_HAMCAST, MODE_NATIVE};

bool logging=false;

void log(std::string str){
    if(logging) std::cout << str << std::endl;
}

std::string role="receiver";
std::string spovnet="hamcast";
std::string nodename="default-node";
std::string bootstrap=spovnet.append("{ip{141.22.26.252};tcp{5005}};");
std::string endpoints="{ip{141.22.26.251};tcpo{5004}};";
//time in seconds
int ival=1;
int lifespan=10;
int payloadsize=100; //byte
const size_t c_bufsize = 65001;
multicast_mode mode=MODE_HAMCAST;
size_t blen;

ptime temp;
ptime to_end;
ptime cur_time;
char* buf;
std::string native_group="239.1.2.3:1234";
int s_pkt_count=0;

Receiver* r;
uint32_t p_all=0;
uint32_t p_ival=0;
uint32_t pktno_recv=0;
std::set<uint32_t> pkts_lost;
std::vector<uint32_t> pkts_dup;
std::set<uint32_t>::iterator pit;
std::vector<double>packages_per_ival;
std::vector<double>lost_per_ival;
std::vector<long>times;

struct option long_options[] = {
        {"recv",        required_argument,  0,  'r'},
        {"send",        required_argument,  0,  's'},
        {"interval",    required_argument,  0,  'i'},
        {"payload",     required_argument,  0,  'p'},
        {"spovnet",     required_argument,  0,  'n'},
        {"time",        required_argument,  0,  't'},
        {"bootstrap",   required_argument,  0,  'b'},
        {"endpoints",   required_argument,  0,  'e'},
        {"nodename",    required_argument,  0,  'c'},
	  {"mode",    	required_argument,  0,  'm'},
	  {"group",    	required_argument,  0,  'g'},
        {0, 0, 0, 0}
    };



void send(){

    ptime start, cur_time,temp;
    uint32_t packet_count=1;
    uint32_t p_ival;
    start=microsec_clock::local_time();
    cur_time=microsec_clock::local_time();
    to_end=start+seconds(lifespan);
    temp=start+seconds(ival);
	struct sockaddr *addr = NULL;
    char* buf = reinterpret_cast<char*>(malloc(payloadsize));


    int ipsock = 0;
    if (mode == MODE_NATIVE) {

        ipsock = socket(AF_INET, SOCK_DGRAM, 0);
        struct sockaddr_in addr_in;
        addr_in.sin_family = AF_INET;

    std::vector<std::string>strs;
    boost::algorithm::split(strs,native_group, boost::algorithm::is_any_of(":"));
    addr_in.sin_port = htons(atoi(strs.at(1).c_str()));

     if (inet_pton(AF_INET, strs.at(0).c_str(), &(addr_in.sin_addr)) != 1)
       {
            std::cout << "group: " << strs.at(0) << std::endl;
          perror("inet_pton");
          exit(1);
        }
        addr = reinterpret_cast<struct sockaddr*>(&addr_in);
    }
    std::cout << "past initialization..." << std::endl;
    while(!r->join_completed) sleep(1);
    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;
    srand ( time(NULL) );

    for (size_t i=0; i < payloadsize; ++i){
            buf[i] = abc[((rand()%len)+1)];
    }

    buf[payloadsize-1]=0;

    while(cur_time < to_end ){
//    std::cout << "curtime: " << cur_time << " to_end: " << to_end << std::endl;



        // write pkt number at start of payload
        uint32_t tmp = htonl(packet_count);
        memcpy(buf, &tmp, sizeof(packet_count));

    if(mode==MODE_NATIVE){
            if(sendto(ipsock, buf, payloadsize, 0, addr, sizeof(struct sockaddr) ) == -1)
                {

                    perror("sendto error: ");
                    exit(1);
                }

    }else{
        r->send(buf, payloadsize);
    //    usleep(1);
   //     r->send_with_ping(packet_count,buf,payloadsize);
	}

    packet_count++;
    p_ival++;
        if(cur_time >= temp){
            //   uint32_t time=temp.seconds();
            time_duration dur=cur_time-temp;
            double packed_in_ival=(p_ival*1000)/((boost::posix_time::seconds(ival)+dur).total_milliseconds());
            packages_per_ival.push_back(packed_in_ival);

            
            long timestamp = time(0);

            times.push_back(timestamp);

            p_ival=0;
            temp=microsec_clock::local_time();
            temp+=boost::posix_time::seconds(ival);
        }


        cur_time=microsec_clock::local_time();
    }


    std::cout << "#total package count: " << p_all << std::endl;
    std::cout << "#ival\t\ttime\t\t\t\tpackages"<< std::endl;

    for(int i=0;i<packages_per_ival.size()-1;i++){
         std::cout << i <<"\t\t"<< times.at(i)<< "\t\t"<<packages_per_ival.at(i)<< std::endl;
     }
    std::cout << "#sended " << packet_count << " packages" << std::endl;
    free(buf);
    stop_all();

}

void ip_receive(){

    int ipsock = 0;
    long runtime_total_loss=0;


    ipsock = socket(AF_INET, SOCK_DGRAM, 0);

    std::vector<std::string>strs;
    boost::algorithm::split(strs,native_group, boost::algorithm::is_any_of(":"));

    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(atoi(strs.at(1).c_str()));
    addr_in.sin_addr.s_addr = INADDR_ANY;
    if(::bind(ipsock, reinterpret_cast<struct sockaddr*>(&addr_in), sizeof(addr_in)) == -1)
        perror("bind error");

    struct group_req mgroup;
    mgroup.gr_interface = 0; //kernel choose interface
    reinterpret_cast<struct sockaddr_in*>(&mgroup.gr_group)->sin_family = AF_INET;
    if ((inet_pton(AF_INET, strs.at(0).c_str(), &(reinterpret_cast<struct sockaddr_in*>(&mgroup.gr_group)->sin_addr))) != 1)
    {
        std::cout << "group: " <<strs.at(0).c_str() << std::endl;
        perror("inet_pton");
        exit(1);
    }
    if(setsockopt(ipsock, IPPROTO_IP, MCAST_JOIN_GROUP, &mgroup, sizeof(mgroup)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }
    std::cout << "past initialization..." << std::endl;
    //SOCKET INITIALIZATION DONE

    char* buf = NULL;
    size_t blen = c_bufsize;
    int recv_size = 0;

    temp=microsec_clock::local_time();
    to_end=microsec_clock::local_time();
    temp=temp+boost::posix_time::seconds(ival);
    to_end=to_end+seconds(lifespan);
    std::cout << "to end: "<< to_end << std::endl;
    buf = reinterpret_cast<char*>(malloc(blen));
    if (blen < payloadsize)
        blen = payloadsize;

    int last_lost_packages=0;

do{
        recv_size = recv(ipsock, buf, payloadsize, 0);
      //  recv_size = recv(ipsock, buf, blen, 0);
        if (recv_size == -1)
    {
        perror("recv error: ");
        exit(EXIT_FAILURE);
    }

    uint32_t tmp;
    memcpy(&tmp, buf, sizeof(tmp));
    uint32_t pktno = ntohl(tmp);

    if (pktno == (pktno_recv+1)) {
        pktno_recv = pktno;
        p_all++;
        p_ival++;
        //    std::cout << "paketno: " << pktno << std::endl;
    }

    else if (pktno > (pktno_recv+1)) {

        runtime_total_loss+=(pktno-pktno_recv-1);


        pktno_recv = pktno;

        p_all++;
        p_ival++;
    }
    else if (pktno <= pktno_recv) {
       }


    cur_time = microsec_clock::local_time();


    if(cur_time >= temp){
        //   uint32_t time=temp.seconds();
        time_duration dur=cur_time-temp;
        double packed_in_ival=(p_ival*1000)/((boost::posix_time::seconds(ival)+dur).total_milliseconds());
        packages_per_ival.push_back(packed_in_ival);

        uint32_t lost_in_ival=runtime_total_loss-last_lost_packages;
         lost_per_ival.push_back(lost_in_ival);
         last_lost_packages=runtime_total_loss;

        long timestamp=std::time(0);
        times.push_back(timestamp);

        p_ival=0;
        temp=microsec_clock::local_time();
        temp+=boost::posix_time::seconds(ival);
    }

    } while(cur_time < to_end);

    std::cout << "#total package count: " << p_all << std::endl;
    std::cout << "#ival\t\ttime\t\tpackages\t\tpacketloss"<< std::endl;
    double sum_up_lost_packages=0;
     for(int i=0;i<packages_per_ival.size()-1;i++){

         std::cout << i <<"\t\t"<<times.at(i) ;
         std::cout <<"\t"<< packages_per_ival.at(i);
         std::cout << "\t\t"<<lost_per_ival[i] << std::endl;

         sum_up_lost_packages+=lost_per_ival[i];
     }

     std::cout << "#packet loss counted at time: " << runtime_total_loss << "packet loss summed up: "<< sum_up_lost_packages <<" last sendet package: " << pktno_recv<<  std::endl;
     stop_all();
}


int main(int argc, char** argv){

    //parsing arguments
    while(true){
    int option_index=0;
    int c=getopt_long(argc, argv, "sri:p:n:t:b:e:c:m:g:", long_options, &option_index);
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
        case 'n':
            spovnet=std::string(optarg);
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
        case 'b':
            bootstrap=std::string(optarg);
            break;

        case 'e':
            endpoints=std::string(optarg);
            break;
        case 'c':
            nodename=std::string(optarg);
            break;
        case 'm':{
            std::string tmp (optarg);
            if (tmp == "ip")
                mode = MODE_NATIVE;
            break;
    }
        case 'g':
            native_group=std::string(optarg);
            break;
        default:
            std::cout << "unknown argument";
            return EXIT_FAILURE;
    }


    }
    std::cout << "#MODE: "   << mode << std::endl;
    std::cout << "#role"<< role << std::endl;
    std::cout << "#interval: " << ival << std::endl;
    std::cout << "#spovnet: "  << spovnet << std::endl;
    std::cout << "#payload: "  << payloadsize << std::endl;
    std::cout << "#time: "   << lifespan << std::endl;

    if(mode!=MODE_NATIVE){
    ariba::utility::StartupWrapper::startSystem();
    r=new Receiver(ival,lifespan, c_bufsize,role,endpoints,bootstrap,nodename,spovnet, payloadsize);
    std::cout << "started ariba service" << std::endl;
    }
    if(role.compare("sender")==0){
      if(mode==MODE_NATIVE){
        send();
      }else {

        ariba::utility::StartupWrapper::startup(r,false);
        sleep(6);


        send();
      }

}else{
        if(mode!=MODE_NATIVE){

        ariba::utility::StartupWrapper::startup(r,false);
		sleep(lifespan*2);

        } else {
            ip_receive();
        }
      }

    return EXIT_SUCCESS;
}

void stop_all(){
    ariba::utility::StartupWrapper::shutdown(r);
    std::cout << "shutted down" << std::endl;
    ariba::utility::StartupWrapper::stopSystem();
    exit(EXIT_SUCCESS);

}
