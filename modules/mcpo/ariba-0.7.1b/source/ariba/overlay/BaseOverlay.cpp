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

#include "BaseOverlay.h"

#include <sstream>
#include <iostream>
#include <string>
#include <boost/foreach.hpp>

#include "ariba/NodeListener.h"
#include "ariba/CommunicationListener.h"
#include "ariba/SideportListener.h"

#include "ariba/overlay/LinkDescriptor.h"

#include "ariba/overlay/messages/OverlayMsg.h"
#include "ariba/overlay/messages/DHTMessage.h"
#include "ariba/overlay/messages/JoinRequest.h"
#include "ariba/overlay/messages/JoinReply.h"

#include "ariba/utility/visual/OvlVis.h"
#include "ariba/utility/visual/DddVis.h"
#include "ariba/utility/visual/ServerVis.h"

namespace ariba {
namespace overlay {

#define visualInstance 		ariba::utility::DddVis::instance()
#define visualIdOverlay 	ariba::utility::ServerVis::NETWORK_ID_BASE_OVERLAY
#define visualIdBase		ariba::utility::ServerVis::NETWORK_ID_BASE_COMMUNICATION

class ValueEntry {
public:
	ValueEntry( const Data& value ) : ttl(0), last_update(time(NULL)),
		last_change(time(NULL)), value(value.clone()) {
	}

	ValueEntry( const ValueEntry& value ) :
		ttl(value.ttl), last_update(value.last_update),
		last_change(value.last_change), value(value.value.clone()) {
	}

	~ValueEntry()  {
		value.release();
	}

	void refresh() {
		last_update = time(NULL);
	}

	void set_value( const Data& value ) {
		this->value.release();
		this->value = value.clone();
		this->last_change = time(NULL);
		this->last_update = time(NULL);
	}

	Data get_value() const {
		return value;
	}

	uint16_t get_ttl() const {
		return ttl;
	}

	void set_ttl( uint16_t ttl ) {
		this->ttl = ttl;
	}

	bool is_ttl_elapsed() const {
		// is persistent? yes-> always return false
		if (ttl==0) return false;
		// return true, if ttl is elapsed
		return ( difftime( time(NULL), this->last_update ) > ttl );
	}

private:
	uint16_t ttl;
	time_t last_update;
	time_t last_change;
	Data value;
};

class DHTEntry {
public:
	Data key;
	vector<ValueEntry> values;

	vector<Data> get_values() {
		vector<Data> vect;
		BOOST_FOREACH( ValueEntry& e, values )
			vect.push_back( e.get_value() );
		return vect;
	}

	void erase_expired_entries() {
		for (vector<ValueEntry>::iterator i = values.begin();
				i != values.end(); i++ )
			if (i->is_ttl_elapsed())
				i = values.erase(i)-1;
	}
};

class DHT {
public:
	typedef vector<DHTEntry> Entries;
	typedef vector<ValueEntry> Values;
	Entries entries;
	static const bool verbose = false;

	static bool equals( const Data& lhs, const Data& rhs ) {
		if (rhs.getLength()!=lhs.getLength()) return false;
		for (size_t i=0; i<lhs.getLength()/8; i++)
			if (lhs.getBuffer()[i] != rhs.getBuffer()[i]) return false;
		return true;
	}

	void put( const Data& key, const Data& value, uint16_t ttl = 0 ) {
		cleanup();

		// find entry
		for (size_t i=0; i<entries.size(); i++) {
			DHTEntry& entry = entries.at(i);

			// check if key is already known
			if ( equals(entry.key, key) ) {

				// check if value is already in values list
				for (size_t j=0; j<entry.values.size(); j++) {
					// found value already? yes-> refresh ttl
					if ( equals(entry.values[j].get_value(), value) ) {
						entry.values[j].refresh();
						if (verbose)
							std::cout << "DHT: Republished value. Refreshing value timestamp."
								<< std::endl;
						return;
					}
				}

				// new value-> add to entry
				if (verbose)
					std::cout << "DHT: Added value to "
						<< " key=" << key << " with value=" << value << std::endl;
				entry.values.push_back( ValueEntry( value ) );
				entry.values.back().set_ttl(ttl);
				return;
			}
		}

		// key is unknown-> add key value pair
		if (verbose)
			std::cout << "DHT: New key value pair "
				<< " key=" << key << " with value=" << value << std::endl;

		// add new entry
		entries.push_back( DHTEntry() );
		DHTEntry& entry = entries.back();
		entry.key = key.clone();
		entry.values.push_back( ValueEntry(value) );
		entry.values.back().set_ttl(ttl);
	}

	vector<Data> get( const Data& key ) {
		cleanup();
		// find entry
		for (size_t i=0; i<entries.size(); i++) {
			DHTEntry& entry = entries.at(i);
			if ( equals(entry.key,key) )
				return entry.get_values();
		}
		return vector<Data>();
	}

	bool remove( const Data& key ) {
		cleanup();

		// find entry
		for (Entries::iterator i = entries.begin(); i != entries.end(); i++) {
			DHTEntry& entry = *i;

			// found? yes-> delete entry
			if ( equals(entry.key, key) ) {
				entries.erase(i);
				return true;
			}
		}
		return false;
	}

	bool remove( const Data& key, const Data& value ) {
		cleanup();
		// find entry
		for (Entries::iterator i = entries.begin(); i != entries.end(); i++) {
			DHTEntry& entry = *i;

			// found? yes-> try to find value
			if ( equals(entry.key, key) ) {
				for (Values::iterator j = entry.values.begin();
						j != entry.values.end(); j++) {

					// value found? yes-> delete
					if (equals(j->get_value(), value)) {
						j = entry.values.erase(j)-1;
						return true;
					}
				}
			}
		}
		return false;
	}

	void cleanup() {
		for (Entries::iterator i = entries.begin(); i != entries.end(); i++) {
			DHTEntry& entry = *i;
			entry.erase_expired_entries();
			if (entry.values.size()==0)
				i = entries.erase(i)-1;
		}
	}
};

// ----------------------------------------------------------------------------

/* *****************************************************************************
 * PREREQUESITES
 * ****************************************************************************/

CommunicationListener* BaseOverlay::getListener( const ServiceID& service ) {
	if( !communicationListeners.contains( service ) ) {
		logging_info( "No listener found for service " << service.toString() );
		return NULL;
	}
	CommunicationListener* listener = communicationListeners.get( service );
	assert( listener != NULL );
	return listener;
}

// link descriptor handling ----------------------------------------------------

LinkDescriptor* BaseOverlay::getDescriptor( const LinkID& link, bool communication ) {
	BOOST_FOREACH( LinkDescriptor* lp, links )
				if ((communication ? lp->communicationId : lp->overlayId) == link)
					return lp;
	return NULL;
}

const LinkDescriptor* BaseOverlay::getDescriptor( const LinkID& link, bool communication ) const {
	BOOST_FOREACH( const LinkDescriptor* lp, links )
				if ((communication ? lp->communicationId : lp->overlayId) == link)
					return lp;
	return NULL;
}

/// erases a link descriptor
void BaseOverlay::eraseDescriptor( const LinkID& link, bool communication ) {
	for ( vector<LinkDescriptor*>::iterator i = links.begin(); i!= links.end(); i++) {
		LinkDescriptor* ld = *i;
		if ((communication ? ld->communicationId : ld->overlayId) == link) {
			delete ld;
			links.erase(i);
			break;
		}
	}
}

/// adds a link descriptor
LinkDescriptor* BaseOverlay::addDescriptor( const LinkID& link ) {
	LinkDescriptor* desc = getDescriptor( link );
	if ( desc == NULL ) {
		desc = new LinkDescriptor();
		if (!link.isUnspecified()) desc->overlayId = link;
		links.push_back(desc);
	}
	return desc;
}

/// returns a auto-link descriptor
LinkDescriptor* BaseOverlay::getAutoDescriptor( const NodeID& node, const ServiceID& service ) {
	// search for a descriptor that is already up
	BOOST_FOREACH( LinkDescriptor* lp, links )
				if (lp->autolink && lp->remoteNode == node && lp->service == service && lp->up && lp->keepAliveMissed == 0)
					return lp;
	// if not found, search for one that is about to come up...
	BOOST_FOREACH( LinkDescriptor* lp, links )
	if (lp->autolink && lp->remoteNode == node && lp->service == service && lp->keepAliveMissed == 0 )
		return lp;
	return NULL;
}

/// stabilizes link information
void BaseOverlay::stabilizeLinks() {
	// send keep-alive messages over established links
	BOOST_FOREACH( LinkDescriptor* ld, links ) {
		if (!ld->up) continue;
		OverlayMsg msg( OverlayMsg::typeLinkAlive,
				OverlayInterface::OVERLAY_SERVICE_ID, nodeId, ld->remoteNode );
		if (ld->relayed) msg.setRouteRecord(true);
		send_link( &msg, ld->overlayId );
	}

	// iterate over all links and check for time boundaries
	vector<LinkDescriptor*> oldlinks;
	time_t now = time(NULL);
	BOOST_FOREACH( LinkDescriptor* ld, links ) {

		// keep alives and not up? yes-> link connection request is stale!
		if ( !ld->up && difftime( now, ld->keepAliveTime ) >= 2 ) {

			// increase counter
			ld->keepAliveMissed++;

			// missed more than four keep-alive messages (10 sec)? -> drop link
			if (ld->keepAliveMissed > 4) {
				logging_info( "Link connection request is stale, closing: " << ld );
				oldlinks.push_back( ld );
				continue;
			}
		}

		if (!ld->up) continue;

		// check if link is relayed and retry connecting directly
		if ( ld->relayed && !ld->communicationUp && ld->retryCounter > 0) {
			ld->retryCounter--;
			ld->communicationId = bc->establishLink( ld->endpoint );
		}

		// remote used as relay flag
		if ( ld->relaying && difftime( now, ld->timeRelaying ) > 10)
			ld->relaying = false;

		// drop links that are dropped and not used as relay
		if (ld->dropAfterRelaying && !ld->relaying && !ld->autolink) {
			oldlinks.push_back( ld );
			continue;
		}

		// auto-link time exceeded?
		if ( ld->autolink && difftime( now, ld->lastuse ) > 30 ) {
			oldlinks.push_back( ld );
			continue;
		}

		// keep alives missed? yes->
		if ( difftime( now, ld->keepAliveTime ) > 4 ) {

			// increase counter
			ld->keepAliveMissed++;

			// missed more than four keep-alive messages (4 sec)? -> drop link
			if (ld->keepAliveMissed >= 2) {
				logging_info( "Link is stale, closing: " << ld );
				oldlinks.push_back( ld );
				continue;
			}
		}
	}

	// drop links
	BOOST_FOREACH( LinkDescriptor* ld, oldlinks ) {
		logging_info( "Link timed out. Dropping " << ld );
		ld->relaying = false;
		dropLink( ld->overlayId );
	}

	// show link state
	counter++;
	if (counter>=4) showLinks();
	if (counter>=4 || counter<0) counter = 0;
}


std::string BaseOverlay::getLinkHTMLInfo() {
	std::ostringstream s;
	vector<NodeID> nodes;
	if (links.size()==0) {
		s << "<h2 style=\"color=#606060\">No links established!</h2>";
	} else {
		s << "<h2 style=\"color=#606060\">Links</h2>";
		s << "<table width=\"100%\" cellpadding=\"0\" border=\"0\" cellspacing=\"0\">";
		s << "<tr style=\"background-color=#ffe0e0\">";
		s << "<td><b>Link ID</b></td><td><b>Remote ID</b></td><td><b>Relay path</b></td>";
		s << "</tr>";

		int i=0;
		BOOST_FOREACH( LinkDescriptor* ld, links ) {
			if (!ld->isVital() || ld->service != OverlayInterface::OVERLAY_SERVICE_ID) continue;
			bool found = false;
			BOOST_FOREACH(NodeID& id, nodes)
			if (id  == ld->remoteNode) found = true;
			if (found) continue;
			i++;
			nodes.push_back(ld->remoteNode);
			if ((i%1) == 1) s << "<tr style=\"background-color=#f0f0f0;\">";
			else s << "<tr>";
			s << "<td>" << ld->overlayId.toString().substr(0,4) << "..</td>";
			s << "<td>" << ld->remoteNode.toString().substr(0,4) << "..</td>";
			s << "<td>";
			if (ld->routeRecord.size()>1 && ld->relayed) {
				for (size_t i=1; i<ld->routeRecord.size(); i++)
					s << ld->routeRecord[ld->routeRecord.size()-i-1].toString().substr(0,4) << ".. ";
			} else {
				s << "Direct";
			}
			s << "</td>";
			s << "</tr>";
		}
		s << "</table>";
	}
	return s.str();
}

/// shows the current link state
void BaseOverlay::showLinks() {
	int i=0;
	logging_info("--- link state -------------------------------");
	BOOST_FOREACH( LinkDescriptor* ld, links ) {
		string epd = "";
		if (ld->isDirectVital())
			epd = getEndpointDescriptor(ld->remoteNode).toString();

		logging_info("LINK_STATE: " << i << ": " << ld << " " << epd);
		i++;
	}
	logging_info("----------------------------------------------");
}

/// compares two arbitrary links to the same node
int BaseOverlay::compare( const LinkID& lhs, const LinkID& rhs ) {
	LinkDescriptor* lhsld = getDescriptor(lhs);
	LinkDescriptor* rhsld = getDescriptor(rhs);
	if (lhsld==NULL || rhsld==NULL
			|| !lhsld->up || !rhsld->up
			|| lhsld->remoteNode != rhsld->remoteNode) return -1;

	if ((lhsld->remoteLink^lhsld->overlayId)<(rhsld->remoteLink^lhsld->overlayId)  )
		return -1;

	return 1;
}


// internal message delivery ---------------------------------------------------

/// routes a message to its destination node
void BaseOverlay::route( OverlayMsg* message ) {

	// exceeded time-to-live? yes-> drop message
	if (message->getNumHops() > message->getTimeToLive()) {
		logging_warn("Message exceeded TTL. Dropping message and relay routes"
				"for recovery.");
		removeRelayNode(message->getDestinationNode());
		return;
	}

	// no-> forward message
	else {
		// destinastion myself? yes-> handle message
		if (message->getDestinationNode() == nodeId) {
			logging_warn("Usually I should not route messages to myself!");
			Message msg;
			msg.encapsulate(message);
			handleMessage( &msg, NULL );
		} else {
			// no->send message to next hop
			send( message, message->getDestinationNode() );
		}
	}
}

/// sends a message to another node, delivers it to the base overlay class
seqnum_t BaseOverlay::send( OverlayMsg* message, const NodeID& destination ) {
	LinkDescriptor* next_link = NULL;

	// drop messages to unspecified destinations
	if (destination.isUnspecified()) return -1;

	// send messages to myself -> handle message and drop warning!
	if (destination == nodeId) {
		logging_warn("Sent message to myself. Handling message.")
		Message msg;
		msg.encapsulate(message);
		handleMessage( &msg, NULL );
		return -1;
	}

	// use relay path?
	if (message->isRelayed()) {
		next_link = getRelayLinkTo( destination );
		if (next_link != NULL) {
			next_link->setRelaying();
			return bc->sendMessage(next_link->communicationId, message);
		} else {
			logging_warn("Could not send message. No relay hop found to "
					<< destination << " -- trying to route over overlay paths ...")
//			logging_error("ERROR: " << debugInformation() );
		//			return -1;
		}
	}

	// last resort -> route over overlay path
	LinkID next_id = overlayInterface->getNextLinkId( destination );
	if (next_id.isUnspecified()) {
		logging_warn("Could not send message. No next hop found to " <<
				destination );
		logging_error("ERROR: " << debugInformation() );
		return -1;
	}

	// get link descriptor, up and running? yes-> send message
	next_link = getDescriptor(next_id);
	if (next_link != NULL && next_link->up) {
		// send message over relayed link
		return send(message, next_link);
	}

	// no-> error, dropping message
	else {
		logging_warn("Could not send message. Link not known or up");
		logging_error("ERROR: " << debugInformation() );
		return -1;
	}

	// not reached-> fail
	return -1;
}

/// send a message using a link descriptor, delivers it to the base overlay class
seqnum_t BaseOverlay::send( OverlayMsg* message, LinkDescriptor* ldr, bool ignore_down ) {
	// check if null
	if (ldr == NULL) {
		logging_error("Can not send message to " << message->getDestinationAddress());
		return -1;
	}

	// check if up
	if (!ldr->up && !ignore_down) {
		logging_error("Can not send message. Link not up:" << ldr );
		logging_error("DEBUG_INFO: " << debugInformation() );
		return -1;
	}
	LinkDescriptor* ld = NULL;

	// handle relayed link
	if (ldr->relayed) {
		logging_debug("Resolving direct link for relayed link to "
				<< ldr->remoteNode);
		ld = getRelayLinkTo( ldr->remoteNode );
		if (ld==NULL) {
			logging_error("No relay path found to link " << ldr );
			logging_error("DEBUG_INFO: " << debugInformation() );
			return -1;
		}
		ld->setRelaying();
		message->setRelayed(true);
	} else
		ld = ldr;

	// handle direct link
	if (ld->communicationUp) {
		logging_debug("send(): Sending message over direct link.");
		return bc->sendMessage( ld->communicationId, message );
	} else {
		logging_error("send(): Could not send message. "
				"Not a relayed link and direct link is not up.");
		return -1;
	}
	return -1;
}

seqnum_t BaseOverlay::send_node( OverlayMsg* message, const NodeID& remote,
		const ServiceID& service) {
	message->setSourceNode(nodeId);
	message->setDestinationNode(remote);
	message->setService(service);
	return send( message, remote );
}

seqnum_t BaseOverlay::send_link( OverlayMsg* message, const LinkID& link,bool ignore_down ) {
	LinkDescriptor* ld = getDescriptor(link);
	if (ld==NULL) {
		logging_error("Cannot find descriptor to link id=" << link.toString());
		return -1;
	}
	message->setSourceNode(nodeId);
	message->setDestinationNode(ld->remoteNode);

	message->setSourceLink(ld->overlayId);
	message->setDestinationLink(ld->remoteLink);

	message->setService(ld->service);
	message->setRelayed(ld->relayed);
	return send( message, ld, ignore_down );
}

// relay route management ------------------------------------------------------

/// stabilize relay information
void BaseOverlay::stabilizeRelays() {
	vector<relay_route>::iterator i = relay_routes.begin();
	while (i!=relay_routes.end() ) {
		relay_route& route = *i;
		LinkDescriptor* ld = getDescriptor(route.link);

		// relay link still used and alive?
		if (ld==NULL
				|| !ld->isDirectVital()
				|| difftime(route.used, time(NULL)) > 8) {
			logging_info("Forgetting relay information to node "
					<< route.node.toString() );
			i = relay_routes.erase(i);
		} else
			i++;
	}
}

void BaseOverlay::removeRelayLink( const LinkID& link ) {
	vector<relay_route>::iterator i = relay_routes.begin();
	while (i!=relay_routes.end() ) {
		relay_route& route = *i;
		if (route.link == link ) i = relay_routes.erase(i); else i++;
	}
}

void BaseOverlay::removeRelayNode( const NodeID& remote ) {
	vector<relay_route>::iterator i = relay_routes.begin();
	while (i!=relay_routes.end() ) {
		relay_route& route = *i;
		if (route.node == remote ) i = relay_routes.erase(i); else i++;
	}
}

/// refreshes relay information
void BaseOverlay::refreshRelayInformation( const OverlayMsg* message, LinkDescriptor* ld ) {

	// handle relayed messages from real links only
	if (ld == NULL
			|| ld->relayed
			|| message->getSourceNode()==nodeId ) return;

	// update usage information
	if (message->isRelayed()) {
		// try to find source node
		BOOST_FOREACH( relay_route& route, relay_routes ) {
			// relay route found? yes->
			if ( route.node == message->getDestinationNode() ) {
				ld->setRelaying();
				route.used = time(NULL);
			}
		}

	}

	// register relay path
	if (message->isRegisterRelay()) {
		// set relaying
		ld->setRelaying();

		// try to find source node
		BOOST_FOREACH( relay_route& route, relay_routes ) {

			// relay route found? yes->
			if ( route.node == message->getSourceNode() ) {

				// refresh timer
				route.used = time(NULL);
				LinkDescriptor* rld = getDescriptor(route.link);

				// route has a shorter hop count or old link is dead? yes-> replace
				if (route.hops > message->getNumHops()
						|| rld == NULL
						|| !rld->isDirectVital()) {
					logging_info("Updating relay information to node "
							<< route.node.toString()
							<< " reducing to " << message->getNumHops() << " hops.");
					route.hops = message->getNumHops();
					route.link = ld->overlayId;
				}
				return;
			}
		}

		// not found-> add new entry
		relay_route route;
		route.hops = message->getNumHops();
		route.link = ld->overlayId;
		route.node = message->getSourceNode();
		route.used = time(NULL);
		logging_info("Remembering relay information to node "
				<< route.node.toString());
		relay_routes.push_back(route);
	}
}

/// returns a known "vital" relay link which is up and running
LinkDescriptor* BaseOverlay::getRelayLinkTo( const NodeID& remote ) {
	// try to find source node
	BOOST_FOREACH( relay_route& route, relay_routes ) {
		if (route.node == remote ) {
			LinkDescriptor* ld = getDescriptor( route.link );
			if (ld==NULL || !ld->isDirectVital()) return NULL; else {
				route.used = time(NULL);
				return ld;
			}
		}
	}
	return NULL;
}

/* *****************************************************************************
 * PUBLIC MEMBERS
 * ****************************************************************************/

use_logging_cpp(BaseOverlay);

// ----------------------------------------------------------------------------

BaseOverlay::BaseOverlay() :
			started(false),state(BaseOverlayStateInvalid),
			bc(NULL),
			nodeId(NodeID::UNSPECIFIED), spovnetId(SpoVNetID::UNSPECIFIED),
			sideport(&SideportListener::DEFAULT), overlayInterface(NULL),
			counter(0) {
	initDHT();
}

BaseOverlay::~BaseOverlay() {
	destroyDHT();
}

// ----------------------------------------------------------------------------

void BaseOverlay::start( BaseCommunication& _basecomm, const NodeID& _nodeid ) {
	logging_info("Starting...");

	// set parameters
	bc = &_basecomm;
	nodeId = _nodeid;

	// register at base communication
	bc->registerMessageReceiver( this );
	bc->registerEventListener( this );

	// timer for auto link management
	Timer::setInterval( 1000 );
	Timer::start();

	started = true;
	state = BaseOverlayStateInvalid;
}

void BaseOverlay::stop() {
	logging_info("Stopping...");

	// stop timer
	Timer::stop();

	// delete oberlay interface
	if(overlayInterface != NULL) {
		delete overlayInterface;
		overlayInterface = NULL;
	}

	// unregister at base communication
	bc->unregisterMessageReceiver( this );
	bc->unregisterEventListener( this );

	started = false;
	state = BaseOverlayStateInvalid;
}

bool BaseOverlay::isStarted(){
	return started;
}

// ----------------------------------------------------------------------------

void BaseOverlay::joinSpoVNet(const SpoVNetID& id,
		const EndpointDescriptor& bootstrapEp) {

	if(id != spovnetId){
		logging_error("attempt to join against invalid spovnet, call initiate first");
		return;
	}

	//ovl.visShowNodeBubble ( ovlId, nodeId, "joining..." );
	logging_info( "Starting to join spovnet " << id.toString() <<
			" with nodeid " << nodeId.toString());

	if(bootstrapEp.isUnspecified() && state == BaseOverlayStateInvalid){

		//** FIRST STEP - MANDATORY */

		// bootstrap against ourselfs
		logging_info("joining spovnet locally");

		overlayInterface->joinOverlay();
		state = BaseOverlayStateCompleted;
		BOOST_FOREACH( NodeListener* i, nodeListeners )
			i->onJoinCompleted( spovnetId );

		//ovl.visChangeNodeIcon ( ovlId, nodeId, OvlVis::ICON_ID_CAMERA );
		//ovl.visChangeNodeColor( ovlId, nodeId, OvlVis::NODE_COLORS_GREEN );

	} else {

		//** SECOND STEP - OPTIONAL */

		// bootstrap against another node
		logging_info("joining spovnet remotely against " << bootstrapEp.toString());

		const LinkID& lnk = bc->establishLink( bootstrapEp );
		bootstrapLinks.push_back(lnk);
		logging_info("join process initiated for " << id.toString() << "...");
	}
}


void BaseOverlay::startBootstrapModules(vector<pair<BootstrapManager::BootstrapType,string> > modules){
	logging_debug("starting overlay bootstrap module");
	overlayBootstrap.start(this, spovnetId, nodeId, modules);
	overlayBootstrap.publish(bc->getEndpointDescriptor());
}

void BaseOverlay::stopBootstrapModules(){
	logging_debug("stopping overlay bootstrap module");
	overlayBootstrap.stop();
	overlayBootstrap.revoke();
}

void BaseOverlay::leaveSpoVNet() {

	logging_info( "Leaving spovnet " << spovnetId );
	bool ret = ( state != this->BaseOverlayStateInvalid );

	logging_debug( "Dropping all auto-links" );

	// gather all service links
	vector<LinkID> servicelinks;
	BOOST_FOREACH( LinkDescriptor* ld, links ) {
		if( ld->service != OverlayInterface::OVERLAY_SERVICE_ID )
			servicelinks.push_back( ld->overlayId );
	}

	// drop all service links
	BOOST_FOREACH( LinkID lnk, servicelinks )
	dropLink( lnk );

	// let the node leave the spovnet overlay interface
	logging_debug( "Leaving overlay" );
	if( overlayInterface != NULL )
		overlayInterface->leaveOverlay();

	// drop still open bootstrap links
	BOOST_FOREACH( LinkID lnk, bootstrapLinks )
	bc->dropLink( lnk );

	// change to inalid state
	state = BaseOverlayStateInvalid;
	//ovl.visShutdown( ovlId, nodeId, string("") );

	visualInstance.visShutdown(visualIdOverlay, nodeId, "");
	visualInstance.visShutdown(visualIdBase, nodeId, "");

	// inform all registered services of the event
	BOOST_FOREACH( NodeListener* i, nodeListeners ) {
		if( ret ) i->onLeaveCompleted( spovnetId );
		else i->onLeaveFailed( spovnetId );
	}
}

void BaseOverlay::createSpoVNet(const SpoVNetID& id,
		const OverlayParameterSet& param,
		const SecurityParameterSet& sec,
		const QoSParameterSet& qos) {

	// set the state that we are an initiator, this way incoming messages are
	// handled correctly
	logging_info( "creating spovnet " + id.toString() <<
			" with nodeid " << nodeId.toString() );

	spovnetId = id;

	overlayInterface = OverlayFactory::create( *this, param, nodeId, this );
	if( overlayInterface == NULL ) {
		logging_fatal( "overlay structure not supported" );
		state = BaseOverlayStateInvalid;

		BOOST_FOREACH( NodeListener* i, nodeListeners )
		i->onJoinFailed( spovnetId );

		return;
	}

	visualInstance.visCreate(visualIdBase, nodeId, "", "");
	visualInstance.visCreate(visualIdOverlay, nodeId, "", "");
}

// ----------------------------------------------------------------------------

const LinkID BaseOverlay::establishLink( const EndpointDescriptor& remoteEp,
		const NodeID& remoteId, const ServiceID& service ) {

	// establish link via overlay
	if (!remoteId.isUnspecified())
		return establishLink( remoteId, service );
	else
		return establishDirectLink(remoteEp, service );
}

/// call base communication's establish link and add link mapping
const LinkID BaseOverlay::establishDirectLink( const EndpointDescriptor& ep,
		const ServiceID& service ) {

	/// find a service listener
	if( !communicationListeners.contains( service ) ) {
		logging_error( "No listener registered for service id=" << service.toString() );
		return LinkID::UNSPECIFIED;
	}
	CommunicationListener* listener = communicationListeners.get( service );
	assert( listener != NULL );

	// create descriptor
	LinkDescriptor* ld = addDescriptor();
	ld->relayed = false;
	ld->listener = listener;
	ld->service = service;
	ld->communicationId = bc->establishLink( ep );

	/// establish link and add mapping
	logging_info("Establishing direct link " << ld->communicationId.toString()
			<< " using " << ep.toString());

	return ld->communicationId;
}

/// establishes a link between two arbitrary nodes
const LinkID BaseOverlay::establishLink( const NodeID& remote,
		const ServiceID& service ) {

	// do not establish a link to myself!
	if (remote == nodeId) return LinkID::UNSPECIFIED;

	// create a link descriptor
	LinkDescriptor* ld = addDescriptor();
	ld->relayed = true;
	ld->remoteNode = remote;
	ld->service = service;
	ld->listener = getListener(ld->service);

	// create link request message
	OverlayMsg msg(OverlayMsg::typeLinkRequest, service, nodeId, remote );
	msg.setSourceLink(ld->overlayId);

	// send over relayed link
	msg.setRelayed(true);
	msg.setRegisterRelay(true);

	// debug message
	logging_info(
			"Sending link request with"
			<< " link=" << ld->overlayId.toString()
			<< " node=" << ld->remoteNode.toString()
			<< " serv=" << ld->service.toString()
	);

	// sending message to node
	send_node( &msg, ld->remoteNode, ld->service );

	return ld->overlayId;
}

/// drops an established link
void BaseOverlay::dropLink(const LinkID& link) {
	logging_info( "Dropping link (initiated locally):" << link.toString() );

	// find the link item to drop
	LinkDescriptor* ld = getDescriptor(link);
	if( ld == NULL ) {
		logging_warn( "Can't drop link, link is unknown!");
		return;
	}

	// delete all queued messages
	if( ld->messageQueue.size() > 0 ) {
		logging_warn( "Dropping link " << ld->overlayId.toString() << " that has "
				<< ld->messageQueue.size() << " waiting messages" );
		ld->flushQueue();
	}

	// inform sideport and listener
	if(ld->listener != NULL)
		ld->listener->onLinkDown( ld->overlayId, ld->remoteNode );
	sideport->onLinkDown(ld->overlayId, this->nodeId, ld->remoteNode, this->spovnetId );

	// do not drop relay links
	if (!ld->relaying) {
		// drop the link in base communication
		if (ld->communicationUp) bc->dropLink( ld->communicationId );

		// erase descriptor
		eraseDescriptor( ld->overlayId );
	} else {
		ld->dropAfterRelaying = true;
	}
}

// ----------------------------------------------------------------------------

/// internal send message, always use this functions to send messages over links
seqnum_t BaseOverlay::sendMessage( const Message* message, const LinkID& link ) {
	logging_debug( "Sending data message on link " << link.toString() );

	// get the mapping for this link
	LinkDescriptor* ld = getDescriptor(link);
	if( ld == NULL ) {
		logging_error("Could not send message. "
				<< "Link not found id=" << link.toString());
		return -1;
	}

	// check if the link is up yet, if its an auto link queue message
	if( !ld->up ) {
		ld->setAutoUsed();
		if( ld->autolink ) {
			logging_info("Auto-link " << link.toString() << " not up, queue message");
			Data data = data_serialize( message );
			const_cast<Message*>(message)->dropPayload();
			ld->messageQueue.push_back( new Message(data) );
		} else {
			logging_error("Link " << link.toString() << " not up, drop message");
		}
		return -1;
	}

	// compile overlay message (has service and node id)
	OverlayMsg overmsg( OverlayMsg::typeData );
	overmsg.encapsulate( const_cast<Message*>(message) );

	// send message over relay/direct/overlay
	return send_link( &overmsg, ld->overlayId );
}

seqnum_t BaseOverlay::sendMessage(const Message* message,
		const NodeID& node, const ServiceID& service) {

	// find link for node and service
	LinkDescriptor* ld = getAutoDescriptor( node, service );

	// if we found no link, create an auto link
	if( ld == NULL ) {

		// debug output
		logging_info( "No link to send message to node "
				<< node.toString() << " found for service "
				<< service.toString() << ". Creating auto link ..."
		);

		// call base overlay to create a link
		LinkID link = establishLink( node, service );
		ld = getDescriptor( link );
		if( ld == NULL ) {
			logging_error( "Failed to establish auto-link.");
			return -1;
		}
		ld->autolink = true;

		logging_debug( "Auto-link establishment in progress to node "
				<< node.toString() << " with link id=" << link.toString() );
	}
	assert(ld != NULL);

	// mark the link as used, as we now send a message through it
	ld->setAutoUsed();

	// send / queue message
	return sendMessage( message, ld->overlayId );
}

// ----------------------------------------------------------------------------

const EndpointDescriptor& BaseOverlay::getEndpointDescriptor(
		const LinkID& link) const {

	// return own end-point descriptor
	if( link.isUnspecified() )
		return bc->getEndpointDescriptor();

	// find link descriptor. not found -> return unspecified
	const LinkDescriptor* ld = getDescriptor(link);
	if (ld==NULL) return EndpointDescriptor::UNSPECIFIED();

	// return endpoint-descriptor from base communication
	return bc->getEndpointDescriptor( ld->communicationId );
}

const EndpointDescriptor& BaseOverlay::getEndpointDescriptor(
		const NodeID& node) const {

	// return own end-point descriptor
	if( node == nodeId || node.isUnspecified() ) {
		//logging_info("getEndpointDescriptor: returning self.");
		return bc->getEndpointDescriptor();
	}

	// no joined and request remote descriptor? -> fail!
	if( overlayInterface == NULL ) {
		logging_error( "Overlay interface not set, cannot resolve end-point." );
		return EndpointDescriptor::UNSPECIFIED();
	}

//	// resolve end-point descriptor from the base-overlay routing table
//	const EndpointDescriptor& ep = overlayInterface->resolveNode( node );
//	if(ep.toString() != "") return ep;

	// see if we can find the node in our own table
	BOOST_FOREACH(const LinkDescriptor* ld, links){
		if(ld->remoteNode != node) continue;
		if(!ld->communicationUp) continue;
		const EndpointDescriptor& ep =
				bc->getEndpointDescriptor(ld->communicationId);
		if(ep != EndpointDescriptor::UNSPECIFIED()) {
			//logging_info("getEndpointDescriptor: using " << ld->to_string());
			return ep;
		}
	}

	logging_warn( "No EndpointDescriptor found for node " << node );
	logging_warn( const_cast<BaseOverlay*>(this)->debugInformation() );

	return EndpointDescriptor::UNSPECIFIED();
}

// ----------------------------------------------------------------------------

bool BaseOverlay::registerSidePort(SideportListener* _sideport) {
	sideport = _sideport;
	_sideport->configure( this );
	return true;
}

bool BaseOverlay::unregisterSidePort(SideportListener* _sideport) {
	sideport = &SideportListener::DEFAULT;
	return true;
}

// ----------------------------------------------------------------------------

bool BaseOverlay::bind(CommunicationListener* listener, const ServiceID& sid) {
	logging_debug( "binding communication listener " << listener
			<< " on serviceid " << sid.toString() );

	if( communicationListeners.contains( sid ) ) {
		logging_error( "some listener already registered for service id "
				<< sid.toString() );
		return false;
	}

	communicationListeners.registerItem( listener, sid );
	return true;
}


bool BaseOverlay::unbind(CommunicationListener* listener, const ServiceID& sid) {
	logging_debug( "unbinding listener " << listener << " from serviceid " << sid.toString() );

	if( !communicationListeners.contains( sid ) ) {
		logging_warn( "cannot unbind listener. no listener registered on service id " << sid.toString() );
		return false;
	}

	if( communicationListeners.get(sid) != listener ) {
		logging_warn( "listener bound to service id " << sid.toString()
				<< " is different than listener trying to unbind" );
		return false;
	}

	communicationListeners.unregisterItem( sid );
	return true;
}

// ----------------------------------------------------------------------------

bool BaseOverlay::bind(NodeListener* listener) {
	logging_debug( "Binding node listener " << listener );

	// already bound? yes-> warning
	NodeListenerVector::iterator i =
			find( nodeListeners.begin(), nodeListeners.end(), listener );
	if( i != nodeListeners.end() ) {
		logging_warn("Node listener " << listener << " is already bound!" );
		return false;
	}

	// no-> add
	nodeListeners.push_back( listener );
	return true;
}

bool BaseOverlay::unbind(NodeListener* listener) {
	logging_debug( "Unbinding node listener " << listener );

	// already unbound? yes-> warning
	NodeListenerVector::iterator i = find( nodeListeners.begin(), nodeListeners.end(), listener );
	if( i == nodeListeners.end() ) {
		logging_warn( "Node listener " << listener << " is not bound!" );
		return false;
	}

	// no-> remove
	nodeListeners.erase( i );
	return true;
}

// ----------------------------------------------------------------------------

void BaseOverlay::onLinkUp(const LinkID& id,
		const address_v* local, const address_v* remote) {
	logging_debug( "Link up with base communication link id=" << id );

	// get descriptor for link
	LinkDescriptor* ld = getDescriptor(id, true);

	// handle bootstrap link we initiated
	if( std::find(bootstrapLinks.begin(), bootstrapLinks.end(), id) != bootstrapLinks.end() ){
		logging_info(
				"Join has been initiated by me and the link is now up. " <<
				"Sending out join request for SpoVNet " << spovnetId.toString()
		);

		// send join request message
		OverlayMsg overlayMsg( OverlayMsg::typeJoinRequest,
				OverlayInterface::OVERLAY_SERVICE_ID, nodeId );
		JoinRequest joinRequest( spovnetId, nodeId );
		overlayMsg.encapsulate( &joinRequest );
		bc->sendMessage( id, &overlayMsg );
		return;
	}

	// no link found? -> link establishment from remote, add one!
	if (ld == NULL) {
		ld = addDescriptor( id );
		logging_info( "onLinkUp (remote request) descriptor: " << ld );

		// update descriptor
		ld->fromRemote = true;
		ld->communicationId = id;
		ld->communicationUp = true;
		ld->setAutoUsed();
		ld->setAlive();

		// in this case, do not inform listener, since service it unknown
		// -> wait for update message!

		// link mapping found? -> send update message with node-id and service id
	} else {
		logging_info( "onLinkUp descriptor (initiated locally):" << ld );

		// update descriptor
		ld->setAutoUsed();
		ld->setAlive();
		ld->communicationUp = true;
		ld->fromRemote = false;

		// if link is a relayed link->convert to direct link
		if (ld->relayed) {
			logging_info( "Converting to direct link: " << ld );
			ld->up = true;
			ld->relayed = false;
			OverlayMsg overMsg( OverlayMsg::typeLinkDirect );
			overMsg.setSourceLink( ld->overlayId );
			overMsg.setDestinationLink( ld->remoteLink );
			send_link( &overMsg, ld->overlayId );
		} else {
			// note: necessary to validate the link on the remote side!
			logging_info( "Sending out update" <<
					" for service " << ld->service.toString() <<
					" with local node id " << nodeId.toString() <<
					" on link " << ld->overlayId.toString() );

			// compile and send update message
			OverlayMsg overlayMsg( OverlayMsg::typeLinkUpdate );
			overlayMsg.setSourceLink(ld->overlayId);
			overlayMsg.setAutoLink( ld->autolink );
			send_link( &overlayMsg, ld->overlayId, true );
		}
	}
}

void BaseOverlay::onLinkDown(const LinkID& id,
		const address_v* local, const address_v* remote) {

	// erase bootstrap links
	vector<LinkID>::iterator it = std::find( bootstrapLinks.begin(), bootstrapLinks.end(), id );
	if( it != bootstrapLinks.end() ) bootstrapLinks.erase( it );

	// get descriptor for link
	LinkDescriptor* ld = getDescriptor(id, true);
	if ( ld == NULL ) return; // not found? ->ignore!
	logging_info( "onLinkDown descriptor: " << ld );

	// removing relay link information
	removeRelayLink(ld->overlayId);

	// inform listeners about link down
	ld->communicationUp = false;
	if (!ld->service.isUnspecified()) {
		CommunicationListener* lst = getListener(ld->service);
		if(lst != NULL) lst->onLinkDown( ld->overlayId, ld->remoteNode );
		sideport->onLinkDown( id, this->nodeId, ld->remoteNode, this->spovnetId );
	}

	// delete all queued messages (auto links)
	if( ld->messageQueue.size() > 0 ) {
		logging_warn( "Dropping link " << id.toString() << " that has "
				<< ld->messageQueue.size() << " waiting messages" );
		ld->flushQueue();
	}

	// erase mapping
	eraseDescriptor(ld->overlayId);
}

void BaseOverlay::onLinkChanged(const LinkID& id,
		const address_v* oldlocal, const address_v* newlocal,
		const address_v* oldremote, const address_v* newremote) {

	// get descriptor for link
	LinkDescriptor* ld = getDescriptor(id, true);
	if ( ld == NULL ) return; // not found? ->ignore!
	logging_debug( "onLinkChanged descriptor: " << ld );

	// inform listeners
	ld->listener->onLinkChanged( ld->overlayId, ld->remoteNode );
	sideport->onLinkChanged( id, this->nodeId, ld->remoteNode, this->spovnetId );

	// autolinks: refresh timestamp
	ld->setAutoUsed();
}

void BaseOverlay::onLinkFail(const LinkID& id,
		const address_v* local, const address_v* remote) {
	logging_debug( "Link fail with base communication link id=" << id );

	// erase bootstrap links
	vector<LinkID>::iterator it = std::find( bootstrapLinks.begin(), bootstrapLinks.end(), id );
	if( it != bootstrapLinks.end() ) bootstrapLinks.erase( it );

	// get descriptor for link
	LinkDescriptor* ld = getDescriptor(id, true);
	if ( ld == NULL ) return; // not found? ->ignore!
	logging_debug( "Link failed id=" << ld->overlayId.toString() );

	// inform listeners
	ld->listener->onLinkFail( ld->overlayId, ld->remoteNode );
	sideport->onLinkFail( id, this->nodeId, ld->remoteNode, this->spovnetId );
}

void BaseOverlay::onLinkQoSChanged(const LinkID& id, const address_v* local,
		const address_v* remote, const QoSParameterSet& qos) {
	logging_debug( "Link quality changed with base communication link id=" << id );

	// get descriptor for link
	LinkDescriptor* ld = getDescriptor(id, true);
	if ( ld == NULL ) return; // not found? ->ignore!
	logging_debug( "Link quality changed id=" << ld->overlayId.toString() );
}

bool BaseOverlay::onLinkRequest( const LinkID& id, const address_v* local,
		const address_v* remote ) {
	logging_debug("Accepting link request from " << remote->to_string() );
	return true;
}

/// handles a message from base communication
bool BaseOverlay::receiveMessage(const Message* message,
		const LinkID& link, const NodeID& ) {
	// get descriptor for link
	LinkDescriptor* ld = getDescriptor( link, true );
	return handleMessage( message, ld, link );
}

// ----------------------------------------------------------------------------

/// Handle spovnet instance join requests
bool BaseOverlay::handleJoinRequest( OverlayMsg* overlayMsg, const LinkID& bcLink ) {

	// decapsulate message
	JoinRequest* joinReq = overlayMsg->decapsulate<JoinRequest>();
	logging_info( "Received join request for spovnet " <<
			joinReq->getSpoVNetID().toString() );

	// check spovnet id
	if( joinReq->getSpoVNetID() != spovnetId ) {
		logging_error(
				"Received join request for spovnet we don't handle " <<
				joinReq->getSpoVNetID().toString() );
		delete joinReq;
		return false;
	}

	// TODO: here you can implement mechanisms to deny joining of a node
	bool allow = true;
	logging_info( "Sending join reply for spovnet " <<
			spovnetId.toString() << " to node " <<
			overlayMsg->getSourceNode().toString() <<
			". Result: " << (allow ? "allowed" : "denied") );
	joiningNodes.push_back( overlayMsg->getSourceNode() );

	// return overlay parameters
	assert( overlayInterface != NULL );
	logging_debug( "Using bootstrap end-point "
			<< getEndpointDescriptor().toString() )
	OverlayParameterSet parameters = overlayInterface->getParameters();
	OverlayMsg retmsg( OverlayMsg::typeJoinReply,
			OverlayInterface::OVERLAY_SERVICE_ID, nodeId );
	JoinReply replyMsg( spovnetId, parameters,
			allow, getEndpointDescriptor() );
	retmsg.encapsulate(&replyMsg);
	bc->sendMessage( bcLink, &retmsg );

	delete joinReq;
	return true;
}

/// Handle replies to spovnet instance join requests
bool BaseOverlay::handleJoinReply( OverlayMsg* overlayMsg, const LinkID& bcLink ) {
	// decapsulate message
	logging_debug("received join reply message");
	JoinReply* replyMsg = overlayMsg->decapsulate<JoinReply>();

	// correct spovnet?
	if( replyMsg->getSpoVNetID() != spovnetId ) { // no-> fail
		logging_error( "Received SpoVNet join reply for " <<
				replyMsg->getSpoVNetID().toString() <<
				" != " << spovnetId.toString() );
		delete replyMsg;
		return false;
	}

	// access granted? no -> fail
	if( !replyMsg->getJoinAllowed() ) {
		logging_error( "Our join request has been denied" );

		// drop initiator link
		if( !bcLink.isUnspecified() ){
			bc->dropLink( bcLink );

			vector<LinkID>::iterator it = std::find(
					bootstrapLinks.begin(), bootstrapLinks.end(), bcLink);
			if( it != bootstrapLinks.end() )
				bootstrapLinks.erase(it);
		}

		// inform all registered services of the event
		BOOST_FOREACH( NodeListener* i, nodeListeners )
		i->onJoinFailed( spovnetId );

		delete replyMsg;
		return true;
	}

	// access has been granted -> continue!
	logging_info("Join request has been accepted for spovnet " <<
			spovnetId.toString() );

	logging_debug( "Using bootstrap end-point "
			<< replyMsg->getBootstrapEndpoint().toString() );

	// create overlay structure from spovnet parameter set
	// if we have not boostrapped yet against some other node
	if( overlayInterface == NULL ){

		logging_debug("first-time bootstrapping");

		overlayInterface = OverlayFactory::create(
				*this, replyMsg->getParam(), nodeId, this );

		// overlay structure supported? no-> fail!
		if( overlayInterface == NULL ) {
			logging_error( "overlay structure not supported" );

			if( !bcLink.isUnspecified() ){
				bc->dropLink( bcLink );

				vector<LinkID>::iterator it = std::find(
						bootstrapLinks.begin(), bootstrapLinks.end(), bcLink);
				if( it != bootstrapLinks.end() )
					bootstrapLinks.erase(it);
			}

			// inform all registered services of the event
			BOOST_FOREACH( NodeListener* i, nodeListeners )
			i->onJoinFailed( spovnetId );

			delete replyMsg;
			return true;
		}

		// everything ok-> join the overlay!
		state = BaseOverlayStateCompleted;
		overlayInterface->createOverlay();

		overlayInterface->joinOverlay( replyMsg->getBootstrapEndpoint() );
		overlayBootstrap.recordJoin( replyMsg->getBootstrapEndpoint() );

		// update ovlvis
		//ovl.visChangeNodeColor( ovlId, nodeId, OvlVis::NODE_COLORS_GREEN);

		// inform all registered services of the event
		BOOST_FOREACH( NodeListener* i, nodeListeners )
		i->onJoinCompleted( spovnetId );

		delete replyMsg;

	} else {

		// this is not the first bootstrap, just join the additional node
		logging_debug("not first-time bootstrapping");
		overlayInterface->joinOverlay( replyMsg->getBootstrapEndpoint() );
		overlayBootstrap.recordJoin( replyMsg->getBootstrapEndpoint() );

		delete replyMsg;

	} // if( overlayInterface == NULL )

	return true;
}


bool BaseOverlay::handleData( OverlayMsg* overlayMsg, LinkDescriptor* ld ) {
	// get service
	const ServiceID& service = overlayMsg->getService();
	logging_debug( "Received data for service " << service.toString()
			<< " on link " << overlayMsg->getDestinationLink().toString() );

	// delegate data message
	CommunicationListener* lst = getListener(service);
	if(lst != NULL){
		lst->onMessage(
				overlayMsg,
				overlayMsg->getSourceNode(),
				overlayMsg->getDestinationLink()
		);
	}

	return true;
}


bool BaseOverlay::handleLinkUpdate( OverlayMsg* overlayMsg, LinkDescriptor* ld ) {

	if( ld == NULL ) {
		logging_warn( "received overlay update message for link for "
				<< "which we have no mapping" );
		return false;
	}
	logging_info("Received type update message on link " << ld );

	// update our link mapping information for this link
	bool changed =
			( ld->remoteNode != overlayMsg->getSourceNode() )
			|| ( ld->service != overlayMsg->getService() );

	// set parameters
	ld->up         = true;
	ld->remoteNode = overlayMsg->getSourceNode();
	ld->remoteLink = overlayMsg->getSourceLink();
	ld->service    = overlayMsg->getService();
	ld->autolink   = overlayMsg->isAutoLink();

	// if our link information changed, we send out an update, too
	if( changed ) {
		overlayMsg->swapRoles();
		overlayMsg->setSourceNode(nodeId);
		overlayMsg->setSourceLink(ld->overlayId);
		overlayMsg->setService(ld->service);
		send( overlayMsg, ld );
	}

	// service registered? no-> error!
	if( !communicationListeners.contains( ld->service ) ) {
		logging_warn( "Link up: event listener has not been registered" );
		return false;
	}

	// default or no service registered?
	CommunicationListener* listener = communicationListeners.get( ld->service );
	if( listener == NULL || listener == &CommunicationListener::DEFAULT ) {
		logging_warn("Link up: event listener is default or null!" );
		return true;
	}

	// update descriptor
	ld->listener = listener;
	ld->setAutoUsed();
	ld->setAlive();

	// ask the service whether it wants to accept this link
	if( !listener->onLinkRequest(ld->remoteNode) ) {

		logging_debug("Link id=" << ld->overlayId.toString() <<
				" has been denied by service " << ld->service.toString() << ", dropping link");

		// prevent onLinkDown calls to the service
		ld->listener = &CommunicationListener::DEFAULT;

		// drop the link
		dropLink( ld->overlayId );
		return true;
	}

	// set link up
	ld->up = true;
	logging_info( "Link has been accepted by service and is up: " << ld );

	// auto links: link has been accepted -> send queued messages
	if( ld->messageQueue.size() > 0 ) {
		logging_info( "Sending out queued messages on link " << ld );
		BOOST_FOREACH( Message* msg, ld->messageQueue ) {
			sendMessage( msg, ld->overlayId );
			delete msg;
		}
		ld->messageQueue.clear();
	}

	// call the notification functions
	listener->onLinkUp( ld->overlayId, ld->remoteNode );
	sideport->onLinkUp( ld->overlayId, nodeId, ld->remoteNode, this->spovnetId );

	return true;
}

/// handle a link request and reply
bool BaseOverlay::handleLinkRequest( OverlayMsg* overlayMsg, LinkDescriptor* ld ) {
	logging_info( "Link request received from node id=" << overlayMsg->getSourceNode() );

	//TODO: Check if a request has already been sent using getSourceLink() ...

	// create link descriptor
	LinkDescriptor* ldn = addDescriptor();

	// flags
	ldn->up = true;
	ldn->fromRemote = true;
	ldn->relayed = true;

	// parameters
	ldn->service = overlayMsg->getService();
	ldn->listener = getListener(ldn->service);
	ldn->remoteNode = overlayMsg->getSourceNode();
	ldn->remoteLink = overlayMsg->getSourceLink();

	// update time-stamps
	ldn->setAlive();
	ldn->setAutoUsed();

	// create reply message and send back!
	overlayMsg->swapRoles(); // swap source/destination
	overlayMsg->setType(OverlayMsg::typeLinkReply);
	overlayMsg->setSourceLink(ldn->overlayId);
	overlayMsg->setSourceEndpoint( bc->getEndpointDescriptor() );
	overlayMsg->setRelayed(true);
	send( overlayMsg, ld ); // send back to link

	// inform listener
	if(ldn != NULL && ldn->listener != NULL)
		ldn->listener->onLinkUp( ldn->overlayId, ldn->remoteNode );

	return true;
}

bool BaseOverlay::handleLinkReply( OverlayMsg* overlayMsg, LinkDescriptor* ld ) {

	// find link request
	LinkDescriptor* ldn = getDescriptor(overlayMsg->getDestinationLink());

	// not found? yes-> drop with error!
	if (ldn == NULL) {
		logging_error( "No link request pending for "
				<< overlayMsg->getDestinationLink().toString() );
		return false;
	}
	logging_debug("Handling link reply for " << ldn )

	// check if already up
	if (ldn->up) {
		logging_warn( "Link already up: " << ldn );
		return true;
	}

	// debug message
	logging_debug( "Link request reply received. Establishing link"
			<< " for service " << overlayMsg->getService().toString()
			<< " with local id=" << overlayMsg->getDestinationLink()
			<< " and remote link id=" << overlayMsg->getSourceLink()
			<< " to " << overlayMsg->getSourceEndpoint().toString()
	);

	// set local link descriptor data
	ldn->up = true;
	ldn->relayed = true;
	ldn->service = overlayMsg->getService();
	ldn->listener = getListener(ldn->service);
	ldn->remoteLink = overlayMsg->getSourceLink();
	ldn->remoteNode = overlayMsg->getSourceNode();

	// update timestamps
	ldn->setAlive();
	ldn->setAutoUsed();

	// auto links: link has been accepted -> send queued messages
	if( ldn->messageQueue.size() > 0 ) {
		logging_info( "Sending out queued messages on link " <<
				ldn->overlayId.toString() );
		BOOST_FOREACH( Message* msg, ldn->messageQueue ) {
			sendMessage( msg, ldn->overlayId );
			delete msg;
		}
		ldn->messageQueue.clear();
	}

	// inform listeners about new link
	ldn->listener->onLinkUp( ldn->overlayId, ldn->remoteNode );

	// try to replace relay link with direct link
	ldn->retryCounter = 3;
	ldn->endpoint = overlayMsg->getSourceEndpoint();
	ldn->communicationId =	bc->establishLink( ldn->endpoint );

	return true;
}

/// handle a keep-alive message for a link
bool BaseOverlay::handleLinkAlive( OverlayMsg* overlayMsg, LinkDescriptor* ld ) {
	LinkDescriptor* rld = getDescriptor(overlayMsg->getDestinationLink());
	if ( rld != NULL ) {
		logging_debug("Keep-Alive for " <<
				overlayMsg->getDestinationLink() );
		if (overlayMsg->isRouteRecord())
			rld->routeRecord = overlayMsg->getRouteRecord();
		rld->setAlive();
		return true;
	} else {
		logging_error("Keep-Alive for "
				<< overlayMsg->getDestinationLink() << ": link unknown." );
		return false;
	}
}

/// handle a direct link message
bool BaseOverlay::handleLinkDirect( OverlayMsg* overlayMsg, LinkDescriptor* ld ) {
	logging_debug( "Received direct link replacement request" );

	/// get destination overlay link
	LinkDescriptor* rld = getDescriptor( overlayMsg->getDestinationLink() );
	if (rld == NULL || ld == NULL) {
		logging_error("Direct link replacement: Link "
				<< overlayMsg->getDestinationLink() << "not found error." );
		return false;
	}
	logging_info( "Received direct link convert notification for " << rld );

	// update information
	rld->communicationId = ld->communicationId;
	rld->communicationUp = true;
	rld->relayed = false;

	// mark used and alive!
	rld->setAlive();
	rld->setAutoUsed();

	// erase the original descriptor
	eraseDescriptor(ld->overlayId);
	return true;
}

/// handles an incoming message
bool BaseOverlay::handleMessage( const Message* message, LinkDescriptor* ld,
		const LinkID bcLink ) {
	logging_debug( "Handling message: " << message->toString());

	// decapsulate overlay message
	OverlayMsg* overlayMsg =
			const_cast<Message*>(message)->decapsulate<OverlayMsg>();
	if( overlayMsg == NULL ) return false;

	// increase number of hops
	overlayMsg->increaseNumHops();

	// refresh relay information
	refreshRelayInformation( overlayMsg, ld );

	// update route record
	overlayMsg->addRouteRecord(nodeId);

	// handle dht messages (do not route)
	if (overlayMsg->isDHTMessage())
		return handleDHTMessage(overlayMsg);

	// handle signaling messages (do not route!)
	if (overlayMsg->getType()>=OverlayMsg::typeSignalingStart &&
			overlayMsg->getType()<=OverlayMsg::typeSignalingEnd ) {
		overlayInterface->onMessage(overlayMsg, NodeID::UNSPECIFIED, LinkID::UNSPECIFIED);
		delete overlayMsg;
		return true;
	}

	// message for reached destination? no-> route message
	if (!overlayMsg->getDestinationNode().isUnspecified() &&
			overlayMsg->getDestinationNode() != nodeId ) {
		logging_debug("Routing message "
				<< " from " << overlayMsg->getSourceNode()
				<< " to " << overlayMsg->getDestinationNode()
		);
		route( overlayMsg );
		delete overlayMsg;
		return true;
	}

	// handle DHT response messages
	if (overlayMsg->hasTypeMask( OverlayMsg::maskDHTResponse )) {
		bool ret = handleDHTMessage(overlayMsg);
		delete overlayMsg;
		return ret;
	}

	// handle base overlay message
	bool ret = false; // return value
	switch ( overlayMsg->getType() ) {

	// data transport messages
	case OverlayMsg::typeData:
		ret = handleData(overlayMsg, ld); 			break;

		// overlay setup messages
	case OverlayMsg::typeJoinRequest:
		ret = handleJoinRequest(overlayMsg, bcLink ); 	break;
	case OverlayMsg::typeJoinReply:
		ret = handleJoinReply(overlayMsg, bcLink ); 	break;

		// link specific messages
	case OverlayMsg::typeLinkRequest:
		ret = handleLinkRequest(overlayMsg, ld ); 	break;
	case OverlayMsg::typeLinkReply:
		ret = handleLinkReply(overlayMsg, ld ); 	break;
	case OverlayMsg::typeLinkUpdate:
		ret = handleLinkUpdate(overlayMsg, ld );  	break;
	case OverlayMsg::typeLinkAlive:
		ret = handleLinkAlive(overlayMsg, ld );   	break;
	case OverlayMsg::typeLinkDirect:
		ret = handleLinkDirect(overlayMsg, ld );  	break;

		// handle unknown message type
	default: {
		logging_error( "received message in invalid state! don't know " <<
				"what to do with this message of type " << overlayMsg->getType() );
		ret = false;
		break;
	}
	}

	// free overlay message and return value
	delete overlayMsg;
	return ret;
}

// ----------------------------------------------------------------------------

void BaseOverlay::broadcastMessage(Message* message, const ServiceID& service) {

	logging_debug( "broadcasting message to all known nodes " <<
			"in the overlay from service " + service.toString() );

	if(message == NULL) return;
	message->setReleasePayload(false);

	OverlayInterface::NodeList nodes = overlayInterface->getKnownNodes(true);
	for(size_t i=0; i<nodes.size(); i++){
		NodeID& id = nodes.at(i);
		if(id == this->nodeId) continue; // don't send to ourselfs
		if(i+1 == nodes.size()) message->setReleasePayload(true); // release payload on last send
		sendMessage( message, id, service );
	}
}

/// return the overlay neighbors
vector<NodeID> BaseOverlay::getOverlayNeighbors(bool deep) const {
	// the known nodes _can_ also include our node, so we remove ourself
	vector<NodeID> nodes = overlayInterface->getKnownNodes(deep);
	vector<NodeID>::iterator i = find( nodes.begin(), nodes.end(), this->nodeId );
	if( i != nodes.end() ) nodes.erase( i );
	return nodes;
}

const NodeID& BaseOverlay::getNodeID(const LinkID& lid) const {
	if( lid == LinkID::UNSPECIFIED ) return nodeId;
	const LinkDescriptor* ld = getDescriptor(lid);
	if( ld == NULL ) return NodeID::UNSPECIFIED;
	else return ld->remoteNode;
}

vector<LinkID> BaseOverlay::getLinkIDs( const NodeID& nid ) const {
	vector<LinkID> linkvector;
	BOOST_FOREACH( LinkDescriptor* ld, links ) {
		if( ld->remoteNode == nid || nid == NodeID::UNSPECIFIED ) {
			linkvector.push_back( ld->overlayId );
		}
	}
	return linkvector;
}


void BaseOverlay::onNodeJoin(const NodeID& node) {
	JoiningNodes::iterator i = std::find( joiningNodes.begin(), joiningNodes.end(), node );
	if( i == joiningNodes.end() ) return;

	logging_info( "node has successfully joined baseoverlay and overlay structure "
			<< node.toString() );

	joiningNodes.erase( i );
}

void BaseOverlay::eventFunction() {
	stabilizeRelays();
	stabilizeLinks();
	stabilizeDHT();
	updateVisual();
}

void BaseOverlay::updateVisual(){

	//
	// update base overlay structure
	//

	static NodeID pre = NodeID::UNSPECIFIED;
	static NodeID suc = NodeID::UNSPECIFIED;

	vector<NodeID> nodes = this->getOverlayNeighbors(false);

	if(nodes.size() == 0){

		if(pre != NodeID::UNSPECIFIED){
			visualInstance.visDisconnect(visualIdOverlay, this->nodeId, pre, "");
			pre = NodeID::UNSPECIFIED;
		}
		if(suc != NodeID::UNSPECIFIED){
			visualInstance.visDisconnect(visualIdOverlay, this->nodeId, suc, "");
			suc = NodeID::UNSPECIFIED;
		}

	} // if(nodes.size() == 0)

	if(nodes.size() == 1){
		// only one node, make this pre and succ
		// and then go into the node.size()==2 case
		//nodes.push_back(nodes.at(0));

		if(pre != nodes.at(0)){
			pre = nodes.at(0);
			if(pre != NodeID::UNSPECIFIED)
				visualInstance.visConnect(visualIdOverlay, this->nodeId, pre, "");
		}
	}

	if(nodes.size() == 2){

		// old finger
		if(nodes.at(0) != pre){
			if(pre != NodeID::UNSPECIFIED)
				visualInstance.visDisconnect(visualIdOverlay, this->nodeId, pre, "");
			pre = NodeID::UNSPECIFIED;
		}
		if(nodes.at(1) != suc){
			if(suc != NodeID::UNSPECIFIED)
				visualInstance.visDisconnect(visualIdOverlay, this->nodeId, suc, "");
			suc = NodeID::UNSPECIFIED;
		}

		// connect with fingers
		if(pre == NodeID::UNSPECIFIED){
			pre = nodes.at(0);
			if(pre != NodeID::UNSPECIFIED)
				visualInstance.visConnect(visualIdOverlay, this->nodeId, pre, "");
		}
		if(suc == NodeID::UNSPECIFIED){
			suc = nodes.at(1);
			if(suc != NodeID::UNSPECIFIED)
				visualInstance.visConnect(visualIdOverlay, this->nodeId, suc, "");
		}

	} //if(nodes.size() == 2)

//	{
//		logging_error("================================");
//		logging_error("my nodeid " << nodeId.get(MAX_KEYLENGTH-16, 16));
//		logging_error("================================");
//		if(nodes.size()>= 1){
//			logging_error("real pre " << nodes.at(0).toString());
//			logging_error("real pre " << nodes.at(0).get(MAX_KEYLENGTH-16, 16));
//		}
//		if(nodes.size()>= 2){
//			logging_error("real suc " << nodes.at(1).toString());
//			logging_error("real suc " << nodes.at(1).get(MAX_KEYLENGTH-16, 16));
//		}
//		logging_error("================================");
//		if(pre == NodeID::UNSPECIFIED){
//			logging_error("pre: unspecified");
//		}else{
//			unsigned int prei = pre.get(MAX_KEYLENGTH-16, 16);
//			logging_error("pre: " << prei);
//		}
//		if(suc == NodeID::UNSPECIFIED){
//			logging_error("suc: unspecified");
//		}else{
//			unsigned int suci = suc.get(MAX_KEYLENGTH-16, 16);
//			logging_error("suc: " << suci);
//		}
//		logging_error("================================");
//	}

	//
	// update base communication links
	//

	static set<NodeID> linkset;
	set<NodeID> remotenodes;
	BOOST_FOREACH( LinkDescriptor* ld, links ) {
		if (!ld->isVital() || ld->service != OverlayInterface::OVERLAY_SERVICE_ID)
			continue;

		if (ld->routeRecord.size()>1 && ld->relayed) {
			for (size_t i=1; i<ld->routeRecord.size(); i++)
				remotenodes.insert( ld->routeRecord[ld->routeRecord.size()-i-1] );
		} else {
			remotenodes.insert(ld->remoteNode);
		}
	}

	// which links are old and need deletion?
	bool changed = false;

	do{
		changed = false;
		BOOST_FOREACH(NodeID n, linkset){
			if(remotenodes.find(n) == remotenodes.end()){
				visualInstance.visDisconnect(visualIdBase, this->nodeId, n, "");
				linkset.erase(n);
				changed = true;
				break;
			}
		}
	}while(changed);

	// which links are new and need creation?
	do{
		changed = false;
		BOOST_FOREACH(NodeID n, remotenodes){
			if(linkset.find(n) == linkset.end()){
				visualInstance.visConnect(visualIdBase, this->nodeId, n, "");
				linkset.insert(n);
				changed = true;
				break;
			}
		}
	}while(changed);

}

// ----------------------------------------------------------------------------

void BaseOverlay::initDHT() {
	dht = new DHT();
	localDHT = new DHT();
	republishCounter = 0;
}

void BaseOverlay::destroyDHT() {
	delete dht;
	delete localDHT;
}

/// stabilize DHT state
void BaseOverlay::stabilizeDHT() {

	// do refresh every 2 seconds
	if (republishCounter < 2) {
		republishCounter++;
		return;
	}
	republishCounter = 0;

	// remove old values from DHT
	BOOST_FOREACH( DHTEntry& entry, dht->entries ) {
		// erase old entries
		entry.erase_expired_entries();
	}

	// re-publish values-> do not refresh locally stored values
	BOOST_FOREACH( DHTEntry& entry, localDHT->entries ) {
		BOOST_FOREACH( ValueEntry& value, entry.values )
			dhtPut(entry.key, value.get_value(), value.get_ttl(), false, true );
	}
}

// handle DHT messages
bool BaseOverlay::handleDHTMessage( OverlayMsg* msg ) {

	// de-capsulate message
	logging_debug("Received DHT message");
	DHTMessage* dhtMsg = msg->decapsulate<DHTMessage>();

	// handle DHT data message
	if (msg->getType()==OverlayMsg::typeDHTData) {
		const ServiceID& service = msg->getService();
		logging_info( "Received DHT data for service " << service.toString() );

		// delegate data message
		CommunicationListener* lst = getListener(service);
		if(lst != NULL) lst->onKeyValue(dhtMsg->getKey(), dhtMsg->getValues() );
		delete dhtMsg;
		return true;
	}

	// route message to closest node
	if (!overlayInterface->isClosestNodeTo(msg->getDestinationNode())) {
		logging_debug("Routing DHT message to closest node "
			<< " from " << msg->getSourceNode()
			<< " to " << msg->getDestinationNode()
		);
		route( msg );
		delete msg;
		return true;
	}

	// now, we are the closest node...
	switch (msg->getType()) {

	// ----------------------------------------------------------------- put ---
	case OverlayMsg::typeDHTPut: {
		logging_debug("DHT-Put: Attempt to store values for key "
				<< dhtMsg->getKey());
		if (dhtMsg->doReplace()) {
			logging_debug("DHT-Put: Attempt to replace key: remove old values first!");
			dht->remove(dhtMsg->getKey());
		}
		BOOST_FOREACH( Data value, dhtMsg->getValues() ) {
			logging_debug("DHT-Put: Stored value: " << value );
			dht->put(dhtMsg->getKey(), value, dhtMsg->getTTL() );
		}
		break;
	}

	// ----------------------------------------------------------------- get ---
	case OverlayMsg::typeDHTGet: {
		logging_info("DHT-Get: key=" << dhtMsg->getKey() );
		vector<Data> vect = dht->get(dhtMsg->getKey());
		BOOST_FOREACH(const Data& d, vect)
			logging_info("DHT-Get: value=" << d);
		OverlayMsg omsg(*msg);
		omsg.swapRoles();
		omsg.setType(OverlayMsg::typeDHTData);
		DHTMessage dhtmsg(dhtMsg->getKey(), vect);
		omsg.encapsulate(&dhtmsg);
		dhtSend(&omsg, omsg.getDestinationNode());
		break;
	}

	// -------------------------------------------------------------- remove ---
	case OverlayMsg::typeDHTRemove: {
		if (dhtMsg->hasValues()) {
			BOOST_FOREACH( Data value, dhtMsg->getValues() )
							dht->remove(dhtMsg->getKey(), value );
		} else
			dht->remove( dhtMsg->getKey() );
		break;
	}

	// -------------------------------------------------------------- default---
	default:
		logging_error("DHT Message type unknown.");
		return false;
	}
	delete msg;
	return true;
}

/// put a value to the DHT with a ttl given in seconds
void BaseOverlay::dhtPut( const Data& key, const Data& value, int ttl, bool replace, bool no_local_refresh ) {

	// log
	logging_info("DHT-Put:"
		<< " key=" << key << " value=" << value
		<< " ttl=" << ttl << " replace=" << replace
	);

	if (!no_local_refresh) {

		// put into local data store (for refreshes)
		if (replace) localDHT->remove(key);
		localDHT->put(key, value, ttl);
	}

	// calculate hash
	NodeID dest = NodeID::sha1(key.getBuffer(), key.getLength() / 8);
	DHTMessage dhtmsg( key, value );
	dhtmsg.setReplace( replace );
	dhtmsg.setTTL(ttl);

	OverlayMsg msg( OverlayMsg::typeDHTPut );
	msg.encapsulate( &dhtmsg );
	dhtSend(&msg, dest);
}

/// removes a key value pair from the DHT
void BaseOverlay::dhtRemove( const Data& key, const Data& value ) {
	// remove from local data store
	localDHT->remove(key,value);

	// calculate hash
	NodeID dest = NodeID::sha1(key.getBuffer(), key.getLength() / 8);
	DHTMessage dhtmsg(key,value);

	// send message
	OverlayMsg msg(OverlayMsg::typeDHTRemove);
	msg.encapsulate( &dhtmsg );
	dhtSend(&msg, dest);
}

/// removes all data stored at the given key
void BaseOverlay::dhtRemove( const Data& key ) {
	// log: remove key
	logging_info("DHT-Remove: Removing key=" << key );

	// calculate hash
	NodeID dest = NodeID::sha1(key.getBuffer(), key.getLength() / 8);
	DHTMessage dhtmsg(key);

	// send message
	OverlayMsg msg(OverlayMsg::typeDHTRemove);
	msg.encapsulate( &dhtmsg );
	dhtSend(&msg, dest);
}

/// requests data stored using key
void BaseOverlay::dhtGet( const Data& key, const ServiceID& service ) {
	// log: get
	logging_info("DHT-Get: Trying to resolve key=" <<
			key << " for service=" << service.toString() );

	// calculate hash
	NodeID dest = NodeID::sha1(key.getBuffer(), key.getLength() / 8);
	DHTMessage dhtmsg(key);

	// send message
	OverlayMsg msg(OverlayMsg::typeDHTGet);
	msg.setService(service);
	msg.encapsulate( &dhtmsg );
	dhtSend(&msg, dest);
}

void BaseOverlay::dhtSend( OverlayMsg* msg, const NodeID& dest ) {
	// log: dht send
	logging_info("DHT-Send: Sending message with key=" << dest.toString() );

	/// set source and destination
	msg->setSourceNode(this->nodeId);
	msg->setDestinationNode(dest);

	// local storage? yes-> put into DHT directly
	if (overlayInterface->isClosestNodeTo(msg->getDestinationNode())) {
		Data d = data_serialize(msg);
		Message* m2 = new Message(d);
		OverlayMsg* m3 = m2->decapsulate<OverlayMsg>();
		handleDHTMessage(m3);
		delete m2;
		return;
	}

	// send message "normally"
	send( msg, dest );
}

std::string BaseOverlay::debugInformation() {
	std::stringstream s;
	int i=0;

	// dump overlay information
	s << "Long debug info ... [see below]" << endl << endl;
	s << "--- overlay information ----------------------" << endl;
	s << overlayInterface->debugInformation() << endl;

	// dump link state
	s << "--- link state -------------------------------" << endl;
	BOOST_FOREACH( LinkDescriptor* ld, links ) {
		s << "link " << i << ": " << ld << endl;
		i++;
	}
	s << endl << endl;

	return s.str();
}

}} // namespace ariba, overlay
