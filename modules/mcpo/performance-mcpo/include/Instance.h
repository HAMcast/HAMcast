#include "ariba/ariba.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/logging/Logging.h"
#include "mcpo/MCPO.h"
#include "string.h"
#include <vector>
#include <map>

using namespace std;
using namespace ariba;
using ariba::utility::StartupInterface;
using ariba::utility::Timer;
using ariba::services::mcpo::MCPO;

/**
 * CLIO includes
 */
//#include "clio/CLIO.h"

/**
 * CLIO using
 */
//using spovnet::clio::CLIONode;
//using spovnet::clio::CLIOOrder;
//using spovnet::clio::CLIOResult;
//using spovnet::clio::AppListener;
//using spovnet::clio::OrderID;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

class Instance :
    public MCPO::ReceiverInterface,
    public NodeListener,
    public StartupInterface,
    public Timer {



public:
    Instance(AribaModule* ariba, string nodename, string spovnet, ServiceID sID, size_t blen,  size_t tout, size_t ival);
    virtual ~Instance();
    virtual void sendToGroup(const void* message, uint32_t length, uint32_t group);
  //  virtual void sendToAll(char* message, uint32_t length);
    bool join_completed;
    uint32_t getSendetPackagesFromAriba();



protected:
    // node listener interface
    virtual void onJoinCompleted( const SpoVNetID& vid );
    virtual void onJoinFailed( const SpoVNetID& vid );
    virtual void onLeaveCompleted( const SpoVNetID& vid );
    virtual void onLeaveFailed( const SpoVNetID& vid );

    // startup wrapper interface
    virtual void startup();
    virtual void shutdown();

    // timer events
    virtual void eventFunction();

    // receiver interface
    virtual	void receiveData( const DataMessage& msg );
    virtual	void receiveData( const DataMessage& msg, ServiceID sid );
    virtual void serviceIsReady();

    virtual void setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members);
    virtual void setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members);
    virtual void setK(unsigned int k);


private:
    ptime temp;
    ptime to_end;
    ptime cur_time;

    int i;
        uint32_t first_package;
        uint32_t ival;
        uint32_t tout;
        uint32_t runtime_total_loss;
        std::set<uint32_t> pkts_too_late;
        std::map<uint32_t,uint32_t> pkts_lost;
        std::vector<uint32_t> pkts_dup;
        std::set<uint32_t>::iterator pit;


        std::vector<long>packages_per_ival;
        std::vector<long>lost_per_ival;
        std::vector<long>times;


        uint32_t p_all;
        uint32_t p_ival;
        uint32_t pktno_recv;
        size_t blen;
        char* buf;
        bool started;
        bool logging;
        ServiceID sid;


    // the ariba module and a node
    AribaModule* ariba;
    Node* node;
    ariba::Name name;
    ariba::Name spovnet;
    MCPO* mcpo;
	NodeID* nodeID;

    int last_lost_packages;

};

