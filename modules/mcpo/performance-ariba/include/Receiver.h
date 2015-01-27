#ifndef RECEIVERX_H_
#define RECEIVERX_H_

#include <set>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "ariba/DataMessage.h"

#include "ariba/ariba.h"
#include "ariba/utility/system/StartupInterface.h"
//#include "ariba/utility/system/Timer.h"
//#include "ariba/utility/configuration/Configuration.h"
//#include "ariba/utility/logging/Logging.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread.hpp>
#include "PingPongMessage.h"
#include "ariba/utility/system/Timer.h"
#include "main.h"
//using ariba::utility::Configuration;
using namespace ariba;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using ariba::utility::StartupInterface;
using ariba::utility::Timer;





/**
 * Receiver Main class
 * receiving string Messages and printing them.
**/
class Receiver :
        public StartupInterface,
        public NodeListener,
        public CommunicationListener,
        public Timer

{
public:
    Receiver(uint32_t ival, uint32_t tout, uint32_t b_len, std::string role, std::string endpoints, std::string bootstrap, std::string nodename,std::string spovnet, uint32_t payloadsize);
    ~Receiver();
        virtual void send(char* buf, uint32_t payloadsize);
    virtual void send_with_ping(uint32_t nr, char* buf, uint32_t payloadsize);
    bool join_completed;

protected:
	
	// startup wrapper interface
	virtual void startup();
	virtual void shutdown();

        virtual void onJoinCompleted( const SpoVNetID& vid );
        virtual void onJoinFailed( const SpoVNetID& vid );
        virtual void onLeaveCompleted( const SpoVNetID& vid );
        virtual void onLeaveFailed( const SpoVNetID& vid );
        virtual void onMessage(const DataMessage& msg, const NodeID& remote,const LinkID& lnk = LinkID::UNSPECIFIED);

	virtual void setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members);
	virtual void setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members);

    virtual void log(const char* str);


    virtual void eventFunction();

private:
	// the ariba module and a node
    AribaModule* aribamodule;
	Node* node;
    NodeID* nodeID;

    int i;
        uint32_t ival;
        uint32_t tout;
        uint32_t payloadsize;
	uint32_t runtime_total_loss;
        std::set<uint32_t> pkts_lost;
        std::vector<uint32_t> pkts_dup;
        std::set<uint32_t>::iterator pit;


        std::vector<double>packages_per_ival;
        std::vector<double>lost_per_ival;
        std::vector<long>times;

        uint32_t p_all;
        uint32_t p_ival;
        uint32_t pktno_recv;
        size_t blen;
        ptime temp;
        ptime to_end;
        ptime cur_time;
        char* buf;
        bool started;
        std::string role;
        std::string endpoints;
        std::string bootstrap;
        ariba::Name nodename;
        ariba::Name spovnet;
        bool logging;
        ServiceID sid;
        double last_lost_packages;
        uint32_t packet_count;
};



#endif //
