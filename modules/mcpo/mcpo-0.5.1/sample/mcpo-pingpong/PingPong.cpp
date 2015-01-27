#include "PingPong.h"
#include "ariba/utility/configuration/Configuration.h"
#include "ariba/utility/visual/DddVis.h"

using ariba::services::mcpo::MCPO;
using ariba::utility::Configuration;
using namespace ariba;
 
namespace ariba {
namespace application {
namespace pingpong {

// logging
use_logging_cpp( PingPong );

// construction
PingPong::PingPong() : pingId( 0 ) {
	Timer::setInterval( 5000 );
}

// destruction
PingPong::~PingPong() {
}

// implementation of the startup interface
void PingPong::startup() {

	logging_info( "starting up PingPong service ... " );

	// create ariba module
	logging_debug( "creating ariba underlay module ... " );
	ariba = new AribaModule();

	Name spovnetName("pingpong");
	Name nodeName = Name::UNSPECIFIED;
	this->name = string("<ping>");

	int callbackPort = 0;
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

		// configure CLIO
		/*if (config.exists("clio.callbackport")) {
			logging_debug("Setting callback port");
			callbackPort = stoi(config.read<string> ("clio.callbackport"));
		}*/

		// get visualization
		if( config.exists("ariba.visual3d.ip") && config.exists("ariba.visual3d.port")){
			string ip = config.read<string>("ariba.visual3d.ip");
			unsigned int port = config.read<unsigned int>("ariba.visual3d.port");
			unsigned int color = config.exists("node.color") ?
					config.read<unsigned int>("node.color") : 0;
			ariba::utility::DddVis::instance().configure(ip, port, color);
		}

	} // if( Configuration::haveConfig() )

	// set the global log level to warning
	logging_rootlevel_warn();
	logging_classlevel_debug("MCPO");

	// start ariba module
	ariba->start();

	// create node and join
	node = new Node( *ariba, nodeName );

	// bind NodeListener
	node->bind( this );

	// start node module
	node->start();

	// when initiating, you can define the overlay type, default is Chord [CHORD_OVERLAY]
	SpoVNetProperties params;
	//params.setBaseOverlayType( SpoVNetProperties::ONE_HOP_OVERLAY ); // alternative: OneHop

	// initiate the spovnet
	logging_info("initiating spovnet");
	node->initiate(spovnetName, params);

	// initiate a CLIONode
	/*clioNode = new CLIONode(node, ariba);
	if (callbackPort != 0)
		clioNode->init(callbackPort);
	else
		clioNode->init();*/

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

	// unbind node listener
	node->unbind( this );

	// deleting mcpo instance
	if( mcpo != NULL) delete mcpo;

	// stop the ariba module
	ariba->stop();

	// delete node and ariba module
	delete node;
	delete ariba;

	// now we are completely shut down
	logging_info( "pingpong service shut down" );
}

/*
 * Timer interface
 */
void PingPong::eventFunction() {

	// we ping all nodes using a MCPO broadcast
	//logging_info( "pinging overlay neighbors with ping id " << ++pingId );

	PingPongMessage pingmsg( pingId );
	mcpo->sendToAll( pingmsg );
}

/*
 * NodeListener interface
 */
void PingPong::onJoinCompleted( const SpoVNetID& vid ) {
	logging_info( "pingpong node join completed, spovnetid=" << vid.toString() );

	// initialize the MCPO service
	//mcpo = new MCPO( this, ariba, node, clioNode );
	mcpo = new MCPO( this, ariba, node );
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

/*
 * MCPO::ReceiverInterface interface
 */
void PingPong::receiveData( const DataMessage& msg ) {
	PingPongMessage* pingmsg = msg.getMessage()->convert<PingPongMessage>();
	logging_info("receiveData " << pingmsg->info() );
	delete pingmsg;
}

void PingPong::receiveData( const DataMessage& msg, ServiceID sid ){
 	receiveData(msg);
}


void PingPong::serviceIsReady() {
	logging_info("serviceIsReady()");
	Timer::start();
}

void PingPong::setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members) {

}

void PingPong::setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members) {

}

void PingPong::setK(unsigned int k) {

}

}}} // namespace ariba, application, pingpong
