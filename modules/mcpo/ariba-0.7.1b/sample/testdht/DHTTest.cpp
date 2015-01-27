#include "DHTTest.h"
#include "ariba/utility/configuration/Configuration.h"

using ariba::utility::Configuration;
using namespace ariba;

namespace ariba {
namespace application {
namespace dhttest {

// logging
use_logging_cpp( DHTTest );

// the service that the dhttest wants to use
ServiceID DHTTest::DHTTEST_SERVICEID = ServiceID( 111 );

// construction
DHTTest::DHTTest() {
	Timer::setInterval( 10000 );
}

// destruction
DHTTest::~DHTTest() {
}

// implementation of the startup interface
void DHTTest::startup() {

	logging_info( "starting up DHTTest service ... " );

	// create ariba module
	logging_debug( "creating ariba underlay module ... " );
	ariba = new AribaModule();

	Name spovnetName("dhttest");
	Name nodeName = Name::UNSPECIFIED;
	this->name = string("<dhttest>");

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
		if (config.exists("dhttest.name"))
			name = config.read<string>("dhttest.name");

		// configure test parameteres
		if (config.exists("dhttest.key"))
			key = config.read<string>("dhttest.key");
		if (config.exists("dhttest.data"))
			data = config.read<string>("dhttest.data");

	} // if( Configuration::haveConfig() )

	// start ariba module
	ariba->start();

	// create node and join
	node = new Node( *ariba, nodeName );

	// bind communication and node listener
	node->bind( this );                              /*NodeListener*/
	node->bind( this, DHTTest::DHTTEST_SERVICEID); /*CommunicationListener*/

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

	// dht test started up...
	logging_info( "dhttest starting up with"
			<< " [spovnetid " << node->getSpoVNetId().toString() << "]"
			<< " and [nodeid " << node->getNodeId().toString() << "]" );
}

// implementation of the startup interface
void DHTTest::shutdown() {

	logging_info( "dhttest service starting shutdown sequence ..." );

	// stop timer
	Timer::stop();

	// leave spovnet
	node->leave();

	// unbind communication and node listener
	node->unbind( this );                               /*NodeListener*/
	//node->unbind( this, DHTTest::DHTTEST_SERVICEID ); /*CommunicationListener*/

	// stop the ariba module
	ariba->stop();

	// delete node and ariba module
	delete node;
	delete ariba;

	// now we are completely shut down
	logging_info( "dhttest service shut down" );
}

// timer event
void DHTTest::eventFunction() {

	switch(counter) {
	case 1:
		logging_info( "Putting '" << key << "'->'" << data << "' in the dht.");

		node->put(stod(key), stod(data), 3600);
		break;
	default:
		logging_info( "Trying to get '" << key << "' from the DHT. This is try number " << counter-1 <<".");

		node->get(stod(key), DHTTEST_SERVICEID);
		break;
	}

	counter++;

}

void DHTTest::onJoinCompleted( const SpoVNetID& vid ) {
	logging_info( "dhttest node join completed, spovnetid=" << vid.toString() );

	// key is set -> this node will be testing
	if(!key.empty()) {
		counter = 0;

		// start the timer to ping every second
		Timer::start();
	}

}

void DHTTest::onJoinFailed( const SpoVNetID& vid ) {
	logging_error("dhttest node join failed, spovnetid=" << vid.toString() );
}

void DHTTest::onLeaveCompleted( const SpoVNetID& vid ){
	logging_info("dhttest node leave completed, spovnetid=" << vid.toString() );
}

void DHTTest::onLeaveFailed( const SpoVNetID& vid ){
	logging_error("dhttest node leave failed, spovnetid=" << vid.toString() );
}


bool DHTTest::onLinkRequest(const NodeID& remote) {
	logging_info( "node " << remote.toString() << " wants to build up a link with us ... allowing" );
	return true;
}

void DHTTest::onLinkUp(const LinkID& lnk, const NodeID& remote){
	logging_info( "received link-up event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

void DHTTest::onLinkDown(const LinkID& lnk, const NodeID& remote){
	logging_info( "received link-down event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

void DHTTest::onLinkChanged(const LinkID& lnk, const NodeID& remote){
	logging_info( "link-changed event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

void DHTTest::onLinkFail(const LinkID& lnk, const NodeID& remote){
	logging_info( "received link-failed event for link " << lnk.toString()
			<< " and node " << remote.toString() );
}

void DHTTest::onKeyValue( const Data& key, const vector<Data>& value ) {
	if (value.size() == 0) {
		logging_info( "Received DHT answer for '" << dtos(key)
				<< "': no values stored! ");
		return;
	}
	logging_info( "Received DHT answer for '" << dtos(key) << "' "
			<< " with value='" << dtos(value.front()) << "'.") ;
	// TODO: implement
}

Data DHTTest::stod(string s) {
	return Data((uint8_t*)s.data(), s.length()*8).clone();
}

string DHTTest::dtos(Data d) {
	return string((char*)d.getBuffer(), d.getLength()/8);
}

}}} // namespace ariba, application, dhttest
