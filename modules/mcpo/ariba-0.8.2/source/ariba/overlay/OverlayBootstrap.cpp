// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
// Zirkel 2, 76128 Karlsruhe
// Germany
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the Institute of
// Telematics.
// [License]

#include "OverlayBootstrap.h"

#include "BaseOverlay.h"
#include "ariba/utility/bootstrap/modules/bluetoothsdp/BluetoothSdp.h"

using ariba::utility::BluetoothSdp;

namespace ariba {
namespace overlay {

use_logging_cpp(OverlayBootstrap);
SystemEventType OverlayBootstrapMethodType("OverlayBootstrapMethodType");

OverlayBootstrap::OverlayBootstrap()
	: 	manager( BootstrapManager::instance() ),
		spovnetid( SpoVNetID::UNSPECIFIED ),
		nodeid( NodeID::UNSPECIFIED ),
		overlay( NULL ),
		watchtimer(this),
		haveOverlayConnection(false){

	srand(time(NULL));

	BluetoothSdp::CONNECTION_CHECKER = this;
}

OverlayBootstrap::~OverlayBootstrap(){
}

void OverlayBootstrap::start(BaseOverlay* _overlay,
		const SpoVNetID& _spovnetid, const NodeID& _nodeid,
		vector<pair<BootstrapManager::BootstrapType,string> > modules){
	overlay = _overlay;
	spovnetid = _spovnetid;
	nodeid = _nodeid;

	logging_info("starting overlay bootstrap");

	manager.registerCallback( this );

	typedef pair<BootstrapManager::BootstrapType,string> X;
	BOOST_FOREACH( X i, modules){
		manager.registerModule( i.first, i.second );
	}

	watchtimer.startWatchdog();
}

void OverlayBootstrap::stop(){
	overlay = NULL;
	spovnetid = SpoVNetID::UNSPECIFIED;
	nodeid = NodeID::UNSPECIFIED;

	logging_info("stopping overlay bootstrap");

	manager.unregisterCallback( this );
	manager.unregisterAllModules();

	watchtimer.stopWatchdog();
}

void OverlayBootstrap::handleSystemEvent(const SystemEvent& event){
	JoinData* data = event.getData<JoinData>();

	// announcement for our spovnet
	logging_info( "found bootstrap node for our SpoVNetID " << data->spovnetid.toString()
			<< " on NodeID " << data->nodeid.toString() << " with endpoint " << data->endpoint.toString() );

	// tell the base overlay to join using this endpoint
	assert( overlay != NULL );
	overlay->joinSpoVNet( spovnetid, data->endpoint );

	delete data;
}

void OverlayBootstrap::onBootstrapServiceFound(string name, string info1, string info2, string info3){
	if( overlay == NULL ) return;
	if(name.length() <= 0 || info1.length() <= 0 || info2.length() <= 0 || info3.length() <= 0) return;

	//
	// generate the types
	//

	SpoVNetID sid( info1 );
	NodeID nid( info2 );
	EndpointDescriptor ep( info3 );

	//
	// is this announcement of interest for us?
	//

	// announcement for another spovnet
	if( sid != this->spovnetid ) return;

	// announcement with our nodeid (either our announcement
	// or a node with the same id, any way -> ignore)
	if( nid == this->nodeid ) return;

	//
	// send out the bootstrap information as
	// event to synchronize into the system queue
	//

	JoinData* data = new JoinData();
	data->spovnetid = sid;
	data->nodeid = nid;
	data->endpoint = ep;

	SystemQueue::instance().scheduleEvent(
			SystemEvent( this, OverlayBootstrapMethodType, data), 0 );
}

void OverlayBootstrap::publish(const EndpointDescriptor& _ep){

	ostringstream r;
	r << std::hex << rand();

	randname = r.str();
	manager.publish( randname, spovnetid.toString(), nodeid.toString(), _ep.toString() );
}

void OverlayBootstrap::revoke(){
	manager.revoke( randname );
}

void OverlayBootstrap::recordJoin(const EndpointDescriptor& _ep){
	boost::mutex::scoped_lock lock(lastJoinesMutex);

	JoinData data;
	data.spovnetid = spovnetid;
	data.nodeid = nodeid;
	data.endpoint = _ep;

	logging_info("recording bootstrap information " << data.endpoint.toString());

	lastJoines.push_front(data);
}

bool OverlayBootstrap::haveOverlayConnections(){
	boost::mutex::scoped_lock lock(haveOverlayConnectionMutex);
	return haveOverlayConnection;
}

void OverlayBootstrap::checkOverlayStatus(){

	// if we have no overlay neighbors, try to bootstrap using
	// bootstrap information that we already used

	{	//limit history to 10 endpoints
		boost::mutex::scoped_lock lock(lastJoinesMutex);
		while(lastJoines.size() > 10)
			lastJoines.pop_back();
	}

	{
		boost::mutex::scoped_lock lock(haveOverlayConnectionMutex);
		haveOverlayConnection = overlay->getOverlayNeighbors().size() > 0;

		// we have overlay neighbors -> ok nothing to do
		if(haveOverlayConnection > 0) return;
	}

	// no overlay neighbors, see if we can join using old information
	logging_info("overlay not joined, checking for earlier used bootstrap information");
	EndpointDescriptor joinep = EndpointDescriptor::UNSPECIFIED();

	// no overlay neighbors -> try out already
	// successfully used bootstrap nodes
	JoinData data;
	{
		boost::mutex::scoped_lock lock(lastJoinesMutex);
		JoinStack::iterator i = lastJoines.begin();
		if(i == lastJoines.end()) return;

		// use last used element and then put it into back
		joinep = (*i).endpoint;

		if(lastJoines.size() >= 2)
			swap( *lastJoines.begin(), *(--(lastJoines.end())) );
	}

	logging_info("no overlay conenctivity detected, " <<
					"trying to join using old bootstrap information: " <<
					joinep.toString());

	// try to join using this node, if the join is successfull
	// the endpoint will again be inserted using recordJoin
	overlay->joinSpoVNet( spovnetid, joinep );
}

OverlayBootstrap::WatchdogTimer::WatchdogTimer(OverlayBootstrap* _obj) : obj(_obj) {
}

void OverlayBootstrap::WatchdogTimer::startWatchdog(){
	Timer::setInterval(5000);
	Timer::start();
}

void OverlayBootstrap::WatchdogTimer::stopWatchdog(){
	Timer::stop();
}

void OverlayBootstrap::WatchdogTimer::eventFunction(){
	if(obj == NULL) return;
	obj->checkOverlayStatus();
}

}} // namespace ariba, overlay
