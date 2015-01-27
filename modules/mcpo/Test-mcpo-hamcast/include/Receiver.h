#ifndef RECEIVERX_H_
#define RECEIVERX_H_

#include "ariba/ariba.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/logging/Logging.h"
#include "mcpo/MCPO.h"

using namespace ariba;
using ariba::utility::StartupInterface;
using ariba::utility::Timer;



namespace ariba {
namespace application {

/**
 * Receiver Main class
 * receiving string Messages and printing them.
**/
class Receiver :
        public ariba::services::mcpo::MCPO::ReceiverInterface,
        public StartupInterface,
        public NodeListener,
        public Timer

{
public:
    Receiver(string nodename, string endpoint, string bootstrap, string spovnetName, uint32_t portnr);
	virtual ~Receiver();
        virtual void startup2(AribaModule* ariba, string nodename, string endpoint, string bootstrap, string spovnetName, uint32_t portnr );
         virtual void join(int group);
protected:
	
	// startup wrapper interface
	virtual void startup();
	virtual void shutdown();


        virtual void onJoinCompleted( const SpoVNetID& vid );
        virtual void onJoinFailed( const SpoVNetID& vid );
        virtual void onLeaveCompleted( const SpoVNetID& vid );
        virtual void onLeaveFailed( const SpoVNetID& vid );

	// receiver interface
	virtual	void receiveData( const DataMessage& msg );
    virtual	void receiveData( const DataMessage& msg , ServiceID sID);
	virtual void serviceIsReady();

	virtual void setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members);
	virtual void setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members);
	virtual void setK(unsigned int k);

    virtual void eventFunction();



private:
	// the ariba module and a node
	AribaModule* ariba;
	Node* node;
	string name;
    string m_endpoint;
    string m_bootstrap;
    string m_spovnetName;
    int i;
        ariba::services::mcpo::MCPO* mcpo;
        ServiceID group;
        uint32_t port;
};



}} // namespace ariba, application

#endif //
