#ifndef __PINGPONG_H_
#define __PINGPONG_H_

#include "ariba/ariba.h"
#include "PingPongMessage.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/Timer.h"

#include <vector>

using namespace ariba;
using ariba::utility::StartupInterface;
using ariba::utility::Timer;

namespace ariba {
namespace application {
namespace pingpong {

using namespace std;

/**
 * The PingPong main class
 * This class implements an example service for demonstration purposes
 * The pingpong class sends and receives messages between two SpoVNet
 * instances
 */
class PingPong: public NodeListener,
	public CommunicationListener,
	public StartupInterface,
	public Timer {

	use_logging_h(PingPong);

public:
	PingPong();
	virtual ~PingPong();

protected:
	// communication listener interface
	virtual bool onLinkRequest(const NodeID& remote);
	virtual void onMessage(const DataMessage& msg, const NodeID& remote,
		const LinkID& lnk = LinkID::UNSPECIFIED);
	virtual void onLinkUp(const LinkID& lnk, const NodeID& remote);
	virtual void onLinkDown(const LinkID& lnk, const NodeID& remote);
	virtual void onLinkChanged(const LinkID& lnk, const NodeID& remote);
	virtual void onLinkFail(const LinkID& lnk, const NodeID& remote);

	// node listener interface
	virtual void onJoinCompleted(const SpoVNetID& vid);
	virtual void onJoinFailed(const SpoVNetID& vid);
	virtual void onLeaveCompleted(const SpoVNetID& vid);
	virtual void onLeaveFailed(const SpoVNetID& vid);

	// startup wrapper interface
	virtual void startup();
	virtual void shutdown();

	// timer events
	virtual void eventFunction();

private:
	// the ariba module and a node
	AribaModule* ariba;
	Node* node;
	string name;
	int counter;
	vector<string> names;

	// the ping pong service id
	static ServiceID PINGPONG_SERVICEID;

	// the current ping id
	unsigned long pingId;

};

// needed for simulation support
ARIBA_SIMULATION_SERVICE(PingPong);

}}} // namespace ariba, application, pingpong

#endif // __PINGPONG_H_
