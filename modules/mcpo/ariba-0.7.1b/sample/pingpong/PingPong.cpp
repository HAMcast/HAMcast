#include "PingPong.h"
#include "ariba/utility/configuration/Configuration.h"
#include "ariba/utility/visual/DddVis.h"

using ariba::utility::Configuration;
using namespace ariba;

namespace ariba {
namespace application {
namespace pingpong {

// logging
use_logging_cpp( PingPong );

// the service that the pingpong wants to use
ServiceID PingPong::PINGPONG_SERVICEID = ServiceID( 111 );

// construction
PingPong::PingPong() : pingId( 0 ) {
	Timer::setInterval( 1000 );
}

// destruction
PingPong::~PingPong() {
}

// implementation of the startup interface
void PingPong::startup() {

	// set up logging
	logging_rootlevel_info();
	logging_classlevel_debug(PingPong);

	logging_info( "starting up PingPong service ... " );

	// create ariba module
	logging_debug( "creating ariba underlay module ... " );
	ariba = new AribaModule();

	Name spovnetName("pingpong");
	Name nodeName = Name::UNSPECIFIED;
	this->name = string("<ping>");

	// get settings from configuration object
	if( Configuration::haveConfig() ){
		Configuration& config = Configuration::instance();

		// get node name
		if (config.exists("node.name"))
			nodeName = config.read<string> ("node.name");

		// configure ariba module
		if (config.exists("ariba.endpoints"))
			ariba->setProperty("endpoints", config.read<string>("ariba.endpoints"));
		if (config.exists("ariba.bootstrap.hints"))
			ariba->setProperty("bootstrap.hints", config.read<string>("ariba.bootstrap.hints"));
		if (config.exists("pingpong.name"))
			name = config.read<string>("pingpong.name");

		// get visualization
		if( config.exists("ariba.visual3d.ip") && config.exists("ariba.visual3d.port")){
			string ip = config.read<string>("ariba.visual3d.ip");
			unsigned int port = config.read<unsigned int>("ariba.visual3d.port");
			unsigned int color = config.exists("node.color") ?
					config.read<unsigned int>("node.color") : 0;
			ariba::utility::DddVis::instance().configure(ip, port, color);
		}

	} // if( Configuration::haveConfig() )

	// start ariba module
	ariba->start();

	// create node and join
	node = new Node( *ariba, nodeName );

	// bind communication and node listener
	node->bind( this );                              /*NodeListener*/
	node->bind( this, PingPong::PINGPONG_SERVICEID); /*CommunicationListener*/

	// start node module
	node->start();

	// when initiating, you can define the overlay type, default is Chord [CHORD_OVERLAY]
	SpoVNetProperties params;
	//params.setBaseOverlayType( SpoVNetProperties::ONE_HOP_OVERLAY ); // alternative: OneHop

	// initiate the spovnet
	logging_info("initiating spovnet");
	node->initiate(spovnetName, params);

	// join the spovnet
	logging_info("joining spovnet");
	node->join(spovnetName);

	// ping pong started up...
	logging_info( "pingpong starting up with"
			<< " [spovnetid " << node->getSpoVNetId().toString() << "]"
			<< " and [nodeid " << node->getNodeId().toString() << "]" );
}

// implementation of the startup interface
void PingPong::shutdown() {

	logging_info( "pingpong service starting shutdown sequence ..." );

	// stop timer
	Timer::stop();

	// leave spovnet
	node->leave();

	// unbind communication and node listener
	node->unbind( this );                               /*NodeListener*/
	node->unbind( this, PingPong::PINGPONG_SERVICEID ); /*CommunicationListener*/

	// stop the ariba module
	ariba->stop();

	// delete node and ariba module
	delete node;
	delete ariba;

	// now we are completely shut down
	logging_info( "pingpong service shut down" );
}

// timer event
void PingPong::eventFunction() {

	// we ping all nodes that are known in the overlay structure
	// this can be all nodes (OneHop) overlay or just some neighbors
	// in case of a Chord or Kademlia structure

	// in this sample we use auto-links: we just send out our message
	// to the node and the link is established automatically. for more
	// control we would use the node->establishLink function to create
	// a link and start using the link in the CommunicationListener::onLinkUp
	// function that is implemented further down in PingPong::onLinkUp

	pingId++;
	logging_info( "pinging overlay neighbors with ping id " << pingId );
	PingPongMessage pingmsg( pingId, name );

	//-----------------------------------------------------------------------
	// Option 1: get all neighboring nodes and send the message to each
	//-----------------------------------------------------------------------
	counter++;
	if (counter<0 || counter>4) {
		counter = 0;
		string s;
		for (int i=0; i<names.size();i++) {
			if (i!=0) s+= ", ";
			s = s+names[i];
		}
		logging_info("----> I am " << name << " and I know " << s);
		names.clear();
	}

	vector<NodeID> nodes = node->getNeighborNodes();
	BOOST_FOREACH( NodeID nid, nodes ){
		logging_info( "sending ping message to " << nid.toString() );
		node->sendMessage( pingmsg, nid, PingPong::PINGPONG_SERVICEID );
	}

	//-----------------------------------------------------------------------
	// Option 2: send a "broadcast message" that actually does the same thing
	//           internally, gets all neighboring nodes and sends the message
	//-----------------------------------------------------------------------
	// node->sendBroadcastMessage( pingmsg, PingPong::PINGPONG_SERVICEID );
}

void PingPong::onJoinCompleted( const SpoVNetID& vid ) {
	logging_info( "pingpong node join completed, spovnetid=" << vid.toString() );

	// start the timer to ping every second
	Timer::start();
}

void PingPong::onJoinFailed( const SpoVNetID& vid ) {
	logging_error("pingpong node join failed, spovnetid=" << vid.toString() );
}

void PingPong::onLeaveCompleted( const SpoVNetID& vid ){
	logging_info("pingpong node leave completed, spovnetid=" << vid.toString() );
}

void PingPong::onLeaveFailed( const SpoVNetID& vid ){
	logging_error("pingpong node leave failed, spovnetid=" << vid.toString() );
}

void PingPong::onMessage(const DataMessage& msg, const NodeID& remote, const LinkID& lnk) {
	logging_info("received MEssage size: "<< msg.getSize());
	PingPongMessage* pingmsg = msg.getMessage()->convert<PingPongMessage> ();
	bool found=false;
	for (int i=0;i<names.size(); i++) if (names[i]==pingmsg->getName()) found=true;
	if (!found) names.push_back(pingmsg->getName());
	logging_info( "received ping message on link " << lnk.toString()
			<< " from node " << remote.toString()
			<< ": " << pingmsg->info() );
	delete pingmsg;
}

void PingPong::onLinkUp(const LinkID& lnk, const NodeID& remote){
	logging_info( "received link-up event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

void PingPong::onLinkDown(const LinkID& lnk, const NodeID& remote){
	logging_info( "received link-down event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

void PingPong::onLinkChanged(const LinkID& lnk, const NodeID& remote){
	logging_info( "link-changed event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

bool PingPong::onLinkRequest(const NodeID& remote) {
	logging_info( "node " << remote.toString() << " wants to build up a link with us ... allowing" );
	return true;
}

void PingPong::onLinkFail(const LinkID& lnk, const NodeID& remote){
	logging_info( "received link-failed event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

}}} // namespace ariba, application, pingpong
