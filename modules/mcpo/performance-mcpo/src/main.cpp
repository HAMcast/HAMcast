#include <getopt.h>
#include "ariba/ariba.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/StartupWrapper.h"
#include "Instance.h"
#include <string>

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using std::string;


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
{"group",    	required_argument,  0,  'g'},
{0, 0, 0, 0}
};

ariba::Node* node;
ariba::NodeID* nodeID;
ariba::ServiceID sid1(666);
ariba::ServiceID sid2(777) ; 


std::string role="receiver";
std::string spovnet="hamcast";
std::string nodename="default-node";
std::string bootstrap=spovnet.append("{ip{141.22.26.251};tcp{5005}};");
std::string endpoints="{ip{141.22.26.252};tcp{5005}};";
//time in seconds
size_t ival=1;
size_t lifespan=15;
int payloadsize=1000; //byte
const size_t c_bufsize = 65001;
size_t blen;

ptime temp;
ptime to_end;
ptime cur_time;
char* buf;
std::string native_group="239.1.2.3:1234";
int s_pkt_count=0;

Instance* r;
uint32_t p_all=0;
uint32_t p_ival=0;
uint32_t pktno_recv=0;
std::set<uint32_t> pkts_lost;
std::vector<uint32_t> pkts_dup;
std::set<uint32_t>::iterator pit;
std::vector<double>packages_per_ival;
std::vector<double>lost_per_ival;
std::vector<long>times;



void send(Instance* r){


    ptime start, cur_time,temp;
    uint32_t packet_count=1;
    uint32_t p_ival;

    char* buf = reinterpret_cast<char*>(malloc(payloadsize));

    std::cout << "past initialization..." << std::endl;

    std::string abc = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t len = abc.size() - 1;
    srand ( time(NULL) );
    for (uint32_t i=0; i < payloadsize; i++){
            buf[i] = abc[((rand()%len)+1)];

    }
   //memset(buf,0xff,payloadsize);
   buf[payloadsize-1]=0;



    while(!(r->join_completed)){
        std::cout <<"waiting... a second "<< std::endl;
        sleep(1);
    }
    sleep(7);

    //initialze clocks
    start=microsec_clock::local_time();
    cur_time=microsec_clock::local_time();
    to_end=start+seconds(lifespan);
    temp=start+seconds(ival);
    
    while(cur_time < to_end ){

        // write pkt number at start of payload

        uint32_t tmp = htonl(packet_count);
        memcpy(buf, &tmp, sizeof(packet_count));
        r->sendToGroup(buf, payloadsize,1234);

        packet_count++;
        p_ival++;
        if(cur_time >= temp){

            time_duration dur=cur_time-temp;
            //+0.5 zum runden
            long packed_in_ival=((p_ival*1000)/((boost::posix_time::seconds(ival)+dur).total_milliseconds()))+0.5;
            packages_per_ival.push_back(packed_in_ival);

/*            struct timeval tv;
            unsigned long seconds, useconds, timestamp;
            gettimeofday(&tv, NULL);
            seconds = tv.tv_sec;
            useconds = tv.tv_usec;
            timestamp = (seconds * 1000000) + useconds;


            //times.push_back(timestamp);*/
            time_t time=std::time(0);
            times.push_back(time);
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
    std::cout << "# mcpo sended " << packet_count << " packages" << std::endl;
    std::cout << "# ariba sended " << r->getSendetPackagesFromAriba() << " packages" << std::endl;
    free(buf);


}



int main( int argc, char** argv ) {



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

        case 'g':
            native_group=std::string(optarg);
            break;
        default:
            std::cout << "unknown argument";
            return EXIT_FAILURE;
        }


    }
    std::cout << "#role"<< role << std::endl;
    std::cout << "#interval: " << ival << std::endl;
    std::cout << "#spovnet: "  << spovnet << std::endl;
    std::cout << "#payload: "  << payloadsize << std::endl;
    std::cout << "#time: "   << lifespan << std::endl;

    bool twoInstance(false);


    ariba::utility::StartupWrapper::startSystem();
    std::cout << "started system \n";
    AribaModule* ariba= new AribaModule();
    std::cout << "initiated ariba \n";
    Instance* mcpotester1= new Instance(ariba,nodename, spovnet, sid1,payloadsize, lifespan, ival);
    std::cout << "created instance1 \n";


    //Initialize ariba module
    ariba->setProperty("endpoints", endpoints);
    ariba->setProperty("bootstrap.hints", bootstrap);
    ariba->start();
    std::cout << "started ariba" << std::endl;



    
    
    //ariba::utility::StartupWrapper::startup(mcpotester1,true);
    // this will run blocking until <enter> is hit
    if(role=="sender"){
        ariba::utility::StartupWrapper::startup(mcpotester1,false);
        send(mcpotester1);
    }else {
    ariba::utility::StartupWrapper::startup(mcpotester1,false);
    sleep(lifespan*2);
	}


    ariba::utility::StartupWrapper::shutdown(mcpotester1);
    ariba::utility::StartupWrapper::stopSystem();
    delete ariba;
    return 0;
}

