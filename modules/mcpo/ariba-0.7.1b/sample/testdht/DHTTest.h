#ifndef __PINGPONG_H_
#define __PINGPONG_H_

#include "ariba/ariba.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/Timer.h"

#include <vector>

using namespace ariba;
using ariba::utility::StartupInterface;
using ariba::utility::Timer;

namespace ariba {
namespace application {
namespace dhttest {

using namespace std;

/*
 * DHTTest main class
 */
class DHTTest: public NodeListener,
	public CommunicationListener,
	public StartupInterface,
	public Timer {

	use_logging_h(DHTTest);

public:
	DHTTest();
	virtual ~DHTTest();

protected:
	// communication listener interface
	virtual bool onLinkRequest(const NodeID& remote);
	virtual void onLinkUp(const LinkID& lnk, const NodeID& remote);
	virtual void onLinkDown(const LinkID& lnk, const NodeID& remote);
	virtual void onLinkChanged(const LinkID& lnk, const NodeID& remote);
	virtual void onLinkFail(const LinkID& lnk, const NodeID& remote);

	// the dht message handler (for answers to get() requests)
	virtual void onKeyValue( const Data& key, const vector<Data>& value );

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

	// helper functions to convert from string to Data and visa versa
	Data stod(string s);
	string dtos(Data d);

private:
	// the ariba module and a node
	AribaModule* ariba;
	Node* node;
	string name;

	// this is for the specific dht test
	string key;
	string data;
	int counter;

	// the dht test service id
	static ServiceID DHTTEST_SERVICEID;

};

// needed for simulation support
ARIBA_SIMULATION_SERVICE(DHTTest);

}}} // namespace ariba, application, pingpong

#endif // __DHTTEST_H_
