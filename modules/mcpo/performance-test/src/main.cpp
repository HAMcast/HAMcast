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

std::string role="receiver";
std::string group="ariba://myuri.de:1234";
//time in seconds
int ival=1;
int lifespan=10;
int payloadsize=100; //byte
const size_t c_bufsize = 65001;

uint32_t  s_pkt_count=0;

ptime temp;
ptime to_end;
ptime cur_time;
uint32_t p_all=0;
uint32_t p_ival=0;
uint32_t pktno_recv=0;
std::set<uint32_t> pkts_lost;
std::vector<uint32_t> pkts_dup;
std::set<uint32_t>::iterator pit;
std::vector<double>packages_per_ival;
std::vector<double>lost_per_ival;
std::vector<long>times;
int last_lost_packages=0;

struct option long_options[] = {
        {"interval",    required_argument,  0,  'i'},
        {"group",       required_argument,  0,  'g'},
        {"payload",     required_argument,  0,  'p'},
        {"recv",        required_argument,  0,  'r'},
        {"send",        required_argument,  0,  's'},
        {"time",        required_argument,  0,  't'},
        {0, 0, 0, 0}
    };



void sender (const hamcast::uri &group, const size_t &payload)
{

    ptime start, cur_time,temp;
    uint32_t packet_count=1;
    uint32_t p_ival;
    start=microsec_clock::local_time();
    cur_time=microsec_clock::local_time();
    to_end=start+seconds(lifespan);
    temp=start+seconds(ival);
    hamcast::multicast_socket* hcsock = NULL;
    char* buf = reinterpret_cast<char*>(malloc(payloadsize));
    packet_count=1;

    hcsock=new hamcast::multicast_socket();


    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;

    srand ( time(NULL) );
    for (size_t i=0; i < payload; ++i){
            buf[i] = abc[((rand()%len)+1)];
    }

    // write pkt number at start of payload
    uint32_t tmp = htonl(packet_count);
    memcpy(buf, &tmp, sizeof(packet_count));

    while((ipc::neighbor_set(hcsock->interfaces().at(0))).size()==0){
        sleep(1);
    }
    while(cur_time < to_end ){




        hcsock->send(group,payloadsize,buf);


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

        packet_count++;
        cur_time=microsec_clock::local_time();
    }
    std::cout << "#total package count: " << p_all << std::endl;
    std::cout << "#ival\t\ttime\t\t\t\tpackages"<< std::endl;

    for(int i=0;i<packages_per_ival.size();i++){
         std::cout << i <<"\t\t"<< times.at(i)<< "\t\t"<<packages_per_ival.at(i)<< std::endl;
     }
    std::cout << "#sended " << packet_count << " packages" << std::endl;
    free(buf);



}


void receiver(const hamcast::uri &group, const size_t &payload)
{
    bool started=false;
    ptime temp, to_end, cur_time;
    multicast_socket* hcsock=NULL;

    uint32_t p_all=0;
    uint32_t p_ival=0;
    uint32_t pktno_recv = 0;
    uint32_t runtime_total_loss=0;
    uint32_t wrong_order=0;
    std::vector<double>packages_per_ival;

    hcsock=new multicast_socket();
    hcsock->join(group);



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

           multicast_packet p=hcsock->receive();
           if(!started){
               temp=microsec_clock::local_time();
               to_end=microsec_clock::local_time();
               temp=temp+boost::posix_time::seconds(ival);
               to_end=to_end+boost::posix_time::seconds(lifespan);



               started=true;
           }

           memcpy(buf, p.data(), p.size());
           recv_size = p.size();

        uint32_t tmp;
             memcpy(&tmp, buf, sizeof(tmp));
             uint32_t pktno = ntohl(tmp);
             if (pktno == (pktno_recv+1)) {
                 pktno_recv = pktno;
                 std::cout << "paketno: " << pktno << std::endl;
             }

             else if (pktno > (pktno_recv+1)) {

                  runtime_total_loss+=(pktno-pktno_recv-1);
                 pktno_recv = pktno;
             }
             else if (pktno <= pktno_recv) {
                 runtime_total_loss--;
                 wrong_order++;
             }
             }

           p_all++;
           p_ival++;

           cur_time = microsec_clock::local_time();

           if(cur_time >= temp){
               time_duration dur=cur_time-temp;
               double packed_in_ival=(p_ival*1000)/((boost::posix_time::seconds(ival)+dur).total_milliseconds());
               packages_per_ival.push_back(packed_in_ival);

               p_ival=0;
               temp=microsec_clock::local_time();
               temp+=boost::posix_time::seconds(ival);

               lost_per_ival.push_back(last_lost_packages-pkts_lost.size());
               last_lost_packages=pkts_lost.size()-last_lost_packages;

               /*struct timeval tv;
               unsigned long seconds, useconds, timestamp;
               gettimeofday(&tv, NULL);
               seconds = tv.tv_sec;
               useconds = tv.tv_usec;
               timestamp = (seconds * 1000000) + useconds;*/
               time_t timestamp=std::time(0);
               times.push_back(timestamp);

               p_ival=0;
               temp=microsec_clock::local_time();
               temp+=boost::posix_time::seconds(ival);

           }

           if(cur_time >= to_end ){
               std::cout << "#total package count: " << p_all << std::endl;
               std::cout << "#ival\t\ttime\t\tpackages\t\tpacketloss"<< std::endl;
               double sum_up_lost_packages=0;
                for(int i=0;i<packages_per_ival.size();i++){


                    std::cout << i <<"\t\t"<< times.at(i)<< "\t\t"<<packages_per_ival.at(i)<< "\t\t"<<lost_per_ival[i] << std::endl;

                    sum_up_lost_packages+=lost_per_ival[i];
                }

                std::cout << "#packet loss counted in runtime: " << runtime_total_loss << "packet loss summed up: "<< sum_up_lost_packages << " wrong order: "<< wrong_order <<  std::endl;
               exit(0);

           }

}



int main(int argc, char** argv){

    //parsing arguments
    while(true){
    int option_index=0;
    int c=getopt_long(argc, argv, "srg:i:t:p:", long_options, &option_index);
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
        sender(group,payloadsize);
    }else{
        receiver(group,payloadsize);
    }

    return 0;
}






