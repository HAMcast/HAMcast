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
//std::string group="ariba://myuri.de:1234";
hamcast::uri group1("ariba://myuri.de:1234");
hamcast::uri group2("ariba://blublub.de:1234");
hamcast::uri group3("ariba://blublub.de:2345");
//time in seconds
int ival=1;
int lifespan=40;
int payloadsize=100; //byte
const size_t c_bufsize = 65001;
hamcast::uri* mgroup;
hamcast::multicast_socket* sock;
hamcast::async_multicast_socket* asock;

struct option long_options[] = {
{"interval",    required_argument,  0,  'i'},
{"group",       required_argument,  0,  'g'},
{"payload",     required_argument,  0,  'p'},
{"recv",        required_argument,  0,  'r'},
{"send",        required_argument,  0,  's'},
{"time",        required_argument,  0,  't'},
{0, 0, 0, 0}
};

void pingpong(const uri& uri, size_t size, const void* msg_ptr){

    char* msg = reinterpret_cast<char*>(malloc(size));
    memcpy(msg, msg_ptr, size);

    //std::string message(ntohl(msg));
    std::string message(msg);
    if((message.substr(0,3)).compare("ping")){
        std::cout << "received " << message << " from group: "<< uri << " - sending pong" << std::endl;
        sleep(1);
        sock->send(group1,4,"pong");
    }else if ((message.substr(0,3)).compare("pong")){
        std::cout << "received " << message << " - doing nothing" << std::endl;
    }else {
        std::cout << "received unrecognised message: " << message << std::endl;
    }

}


void sendloop(){
    int i=0;
    while(i < lifespan){
        sock->send(group1,5,"ping1");
        sock->send(group2,5,"ping2");
        sock->send(group3,5,"ping3");
        lifespan++;
        sleep(1);
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
            //std::string group=std::string(optarg);
            //group1(group);
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
    std::cout << "group1: "  << group1.str() << std::endl;
    std::cout << "group2: "  << group2.str() << std::endl;
    std::cout << "payload: "  << payloadsize << std::endl;
    std::cout << "time: "   << lifespan << std::endl;

    asock= new hamcast::async_multicast_socket(*pingpong);
    sock = new hamcast::multicast_socket();
    asock->join(group1);
    asock->join(group2);
    asock->join(group3);
    sendloop();
    return 0;
}






