#ifndef PINGPONG_H_
#define PINGPONG_H_

#include "ariba/ariba.h"
#include "PingPongMessage.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/logging/Logging.h"
#include "mcpo/MCPO.h"

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

namespace ariba {
namespace application {
namespace pingpong {

/**
 * The PingPong main class
 * This class implements an example service for demonstration purposes
 * The pingpong class sends and receives messages between two SpoVNet
 * instances
**/
class PingPong :
	public MCPO::ReceiverInterface,
	public NodeListener,
	public StartupInterface,
	public Timer {

	use_logging_h(PingPong);

public:
	PingPong();
	virtual ~PingPong();

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
	// the ariba module and a node
	AribaModule* ariba;
	Node* node;
	string name;
	MCPO* mcpo;

	// the CLIO connector
	//CLIONode* clioNode;

	// flag, whether this node initiates or just joins the spovnet
	bool isInitiator;

	// the current ping id
	unsigned long pingId;
};

// needed for simulation support
ARIBA_SIMULATION_SERVICE(PingPong);

}}} // namespace ariba, application, pingpong

#endif // PINGPONG_H_
