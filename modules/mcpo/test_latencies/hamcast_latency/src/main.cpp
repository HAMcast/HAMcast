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
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace hamcast;
using namespace std;

std::string group="ariba://myuri0.de:1234";
//time in seconds
int ival=500;
int lifetime=30;
int payloadsize=100; //byte
const size_t c_bufsize = 65001;

uint32_t  s_pkt_count=0;
uint32_t p_all=0;
uint32_t p_ival=0;
uint32_t pktno_recv=0;
bool is_sender=false;
ptime start, end_time, to_end, cur_time;
hamcast::async_multicast_socket* hcsock_recv = NULL;
hamcast::multicast_socket* hcsock_send = NULL;
struct option long_options[] = {
        {"group",       required_argument,  0,  'g'},
        {"send",        required_argument,  0,  's'},
        {"time",        required_argument,  0,  't'},
        {"interval",    required_argument,  0,  'i'},
        {"payload",     required_argument,  0,  'p'},
        {0, 0, 0, 0}
    };


void out_and_end(){
    end_time=microsec_clock::local_time();
    std::cout << "#start\t\t end \t\t difference"<< std::endl;
    std::cout <<start << "\t\t"<< end_time <<"\t\t" << end_time-start << std::endl;
    std::vector<interface_id> interfaceids=hcsock_send->interfaces();
     std::vector<uri> uris=hamcast::ipc::children_set(interfaceids.at(0),group);
     std::cout << "uris" << std::endl;
     for(int i=0;i<uris.size();i++){
        std::cout << uris.at(i);
     }
     std::cout<< "neighbours= "<<uris.size() << std::endl;
     std::cout << "highest layer= "<<(int)(uris.size()/7)<< std::endl;
      if(hcsock_send != NULL){
        std::cout << "#in leave group"<< std::endl;
        hcsock_send->leave(group);
    }
    delete hcsock_send;
    exit(0);
}


void receive(const uri&, size_t, const void*){
    out_and_end();
}

void join(const hamcast::uri &group){




    start=microsec_clock::local_time();
    //for async socket
   // hcsock_recv->join(group);

    //with sync socket
    hcsock_send->join(group);
    std::cout << "#joined group"<< std::endl;
    hcsock_send->receive();
    out_and_end();

}

void send(hamcast::uri group){
    start=microsec_clock::local_time();
    to_end=start+boost::posix_time::seconds(lifetime);
    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;
    char* buf = reinterpret_cast<char*>(malloc(payloadsize));
    srand ( time(NULL) );
    for (size_t i=0; i < payloadsize; ++i){
            buf[i] = abc[((rand()%len)+1)];
    }


    for(int i=0; i<10;i++){
        string num=boost::lexical_cast<std::string>(i);
        string adress="ariba://myuri"+num+".de:1234";
        //For async socket
        //hcsock_recv->join(hamcast::uri(adress));
        hcsock_send->join(hamcast::uri(adress));
        std::cout << "join group: " << adress << std::endl;
    }


    int i=1;
   // cout << << "to_end: "<<to_end << " cur_time: "
   cur_time=microsec_clock::local_time();
   while(to_end > cur_time){
    // while(true){
      //   std::cout << "in loop"<< std::endl;
         hcsock_send->send(group,payloadsize,buf);
    //       std::cout << "after send"<< endl;
            usleep(ival);
//            std::cout << "sending message: "<<i << std::endl;
  //          i++;
        cur_time=microsec_clock::local_time();
    }
}



int main(int argc, char** argv){

    //parsing arguments
    while(true){
    int option_index=0;
    int c=getopt_long(argc, argv, "sg:i:t:p:", long_options, &option_index);
    if(c<=-1){
        break;
    }

    switch(c){
        case 's':
            is_sender=true;
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
            lifetime=atoi(optarg);
            break;

        default:
            std::cout << "unknown argument";
            return EXIT_FAILURE;
    }


    }
    if(is_sender) {
        std::cout << "#i am sender"<< std::endl;
     }else{
        std::cout << "#i am joiner"<< std::endl;
    }
    std::cout << "#interval: " << ival << std::endl;
    std::cout << "#group: "  << group << std::endl;
    std::cout << "#payload: "  << payloadsize << std::endl;
    std::cout << "#time: "   << lifetime << std::endl;

    hcsock_recv=new hamcast::async_multicast_socket(receive);
        hcsock_send=new hamcast::multicast_socket();

    if(is_sender){
        send(group);

    }else{
        join(group);
      //  sleep(lifetime);
    }
    std::cout << "byebye!"<<std::endl;

    return 0;
}





