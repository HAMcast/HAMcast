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

#include "ariba/overlay/BaseOverlay.h"
#include "ariba/overlay/messages/OverlayMsg.h"

#include "Chord.h"
#include "detail/chord_routing_table.hpp"

#include "messages/Discovery.h"

namespace ariba {
namespace overlay {

enum signalMessageTypes {
	typeDiscovery = OverlayMsg::typeSignalingStart + 0x01,
	typeLeave = OverlayMsg::typeSignalingStart + 0x02,
};

typedef chord_routing_table::item route_item;

use_logging_cpp( Chord );

Chord::Chord(BaseOverlay& _baseoverlay, const NodeID& _nodeid,
		OverlayStructureEvents* _eventsReceiver, const OverlayParameterSet& param) :
	OverlayInterface(_baseoverlay, _nodeid, _eventsReceiver, param) {

	// create routing table
	this->table = new chord_routing_table(_nodeid, 4);
	orphan_removal_counter = 0;
	discovery_count = 0;
	stabilize_counter = 0;
	stabilize_finger = 0;
}

Chord::~Chord() {

	// delete routing table
	delete table;
}

/// helper: sets up a link using the base overlay
LinkID Chord::setup(const EndpointDescriptor& endpoint, const NodeID& remote ) {

	// check if we already have a connection
	for (size_t i=0; i<table->size(); i++)
		if ((*table)[i]->ref_count > 0 && (*table)[i]->id == remote && !((*table)[i]->info.isUnspecified()))
			return LinkID::UNSPECIFIED;

	// check if we are already trying to establish a link
	for (size_t i=0; i<pending.size(); i++)
		if ( pending[i] == remote ) {
			logging_debug("Already trying to establish a link to node "
				<< remote.toString() );
			return LinkID::UNSPECIFIED;
		}

	// adding node to list of pending connections
	pending.push_back( remote );

	logging_info("Request to setup link to " << endpoint.toString() );

	// establish link via base overlay
	return baseoverlay.establishLink( endpoint, remote,
			OverlayInterface::OVERLAY_SERVICE_ID );
}

/// helper: sends a message using the "base overlay"
seqnum_t Chord::send( OverlayMsg* msg, const LinkID& link ) {
	if (link.isUnspecified()) return 0;
	return baseoverlay.send_link( msg, link );
}

/// sends a discovery message
void Chord::send_discovery_to(const NodeID& remote, int ttl) {
	LinkID link = getNextLinkId(remote);
	if ( remote == nodeid || link.isUnspecified()) return;
	if ( table->size() == 0 ) return;
	ttl = 2;

	OverlayMsg msg( typeDiscovery );
	msg.setRegisterRelay(true);
	Discovery dmsg( Discovery::normal, (uint8_t)ttl, baseoverlay.getEndpointDescriptor() );
	msg.encapsulate(&dmsg);

	// send to node
	baseoverlay.send_node( &msg, remote );
}

void Chord::discover_neighbors( const LinkID& link ) {
	uint8_t ttl = 1;
	{
		// send predecessor discovery
		OverlayMsg msg( typeDiscovery );
		msg.setRegisterRelay(true);
		Discovery dmsg( Discovery::predecessor, ttl,
			baseoverlay.getEndpointDescriptor() );
		msg.encapsulate(&dmsg);
		send(&msg, link);
	}
	{
		// send successor discovery
		OverlayMsg msg( typeDiscovery );
		msg.setSourceEndpoint( baseoverlay.getEndpointDescriptor() );
		msg.setRegisterRelay(true);
		Discovery dmsg( Discovery::successor, ttl,
			baseoverlay.getEndpointDescriptor() );
		msg.encapsulate(&dmsg);
		send(&msg, link);
	}
}


void Chord::createOverlay() {
}

void Chord::deleteOverlay() {

}

void Chord::joinOverlay(const EndpointDescriptor& boot) {
	logging_info( "joining Chord overlay structure through end-point " <<
			(boot.isUnspecified() ? "local" : boot.toString()) );

	// initiator? no->setup first link
	if (!boot.isUnspecified())
		bootstrapLinks.push_back( setup(boot) );

	// timer for stabilization management
	Timer::setInterval(1000);
	Timer::start();
}

void Chord::leaveOverlay() {
	Timer::stop();
	for (size_t i = 0; i < table->size(); i++) {
		route_item* it = (*table)[i];
		OverlayMsg msg( typeLeave );
		send( &msg, it->info );
	}
}

/// @see OverlayInterface.h
const EndpointDescriptor& Chord::resolveNode(const NodeID& node) {
	const route_item* item = table->get(node);
	if (item == NULL || item->info.isUnspecified()) return EndpointDescriptor::UNSPECIFIED();
	return baseoverlay.getEndpointDescriptor(item->info);
}

/// @see OverlayInterface.h
bool Chord::isClosestNodeTo( const NodeID& node ) {
	return table->is_closest_to(node);
}

/// @see OverlayInterface.h
const LinkID& Chord::getNextLinkId( const NodeID& id ) const {
	// get next hop
	const route_item* item = table->get_next_hop(id);

	// returns a unspecified id when this is itself
	if (item == NULL || item->id == nodeid)
		return LinkID::UNSPECIFIED;

	/// return routing info
	return item->info;
}

OverlayInterface::NodeList Chord::getKnownNodes(bool deep) const {
	OverlayInterface::NodeList nodelist;

	if( deep ){
		// all nodes that I know, fingers, succ/pred
		for (size_t i = 0; i < table->size(); i++){
			if ((*table)[i]->ref_count != 0
					&& !(*table)[i]->info.isUnspecified())
				nodelist.push_back((*table)[i]->id);
		}
	} else {
		// only succ and pred
		if( table->get_predesessor() != NULL ){
			nodelist.push_back( *(table->get_predesessor()) );
		}
		if( table->get_successor() != NULL ){
			OverlayInterface::NodeList::iterator i =
				std::find( nodelist.begin(), nodelist.end(), *(table->get_successor()) );
			if( i == nodelist.end() )
				nodelist.push_back( *(table->get_successor()) );
		}
	}

	return nodelist;
}

/// @see CommunicationListener.h
/// @see OverlayInterface.h
void Chord::onLinkUp(const LinkID& lnk, const NodeID& remote) {
	logging_info("link_up: link=" << lnk.toString() << " remote=" <<
			remote.toString() );
	for (vector<NodeID>::iterator i=pending.begin(); i!=pending.end(); i++)
		if (*i == remote) {
			pending.erase(i);
			break;
		}

	if (remote==nodeid) {
		logging_warn("dropping link that has been established to myself (nodes have same nodeid?)");
		baseoverlay.dropLink(lnk);
		return;
	}

	route_item* item = table->insert(remote);

	// item added to routing table?
	if (item != NULL) { // yes-> add to routing table
		logging_info("new routing neighbor: " << remote.toString()
				<< " with link " << lnk.toString());

		// replace with new link if link is "better"
		if (item->info!=lnk && item->info.isUnspecified()==false) {
			if (baseoverlay.compare( item->info, lnk ) == 1) {
				logging_info("Replacing link due to concurrent link establishment.");
				baseoverlay.dropLink(item->info);
				item->info = lnk;
			}
		} else {
			item->info = lnk;
		}

		// discover neighbors of new overlay neighbor
		showLinks();
	} else { // no-> add orphan entry to routing table
		logging_info("new orphan: " << remote.toString()
				<< " with link " << lnk.toString());
		table->insert_orphan(remote)->info = lnk;
	}

	// erase bootstrap link
	vector<LinkID>::iterator it = std::find(bootstrapLinks.begin(), bootstrapLinks.end(), lnk);
	if( it != bootstrapLinks.end() ) bootstrapLinks.erase( it );
}

/// @see CommunicationListener.h or @see OverlayInterface.h
void Chord::onLinkDown(const LinkID& lnk, const NodeID& remote) {
	logging_debug("link_down: link=" << lnk.toString() << " remote=" <<
			remote.toString() );

	// remove link from routing table
	route_item* item = table->get(remote);
	if (item!=NULL && item->info==lnk) {
		item->info = LinkID::UNSPECIFIED;
		table->remove(remote);
	}
}

/// @see CommunicationListener.h
/// @see OverlayInterface.h
void Chord::onMessage(const DataMessage& msg, const NodeID& remote,
		const LinkID& link) {

	// decode message
	OverlayMsg* m = dynamic_cast<OverlayMsg*>(msg.getMessage());
	if (m == NULL) return;

	// handle messages
	switch ((signalMessageTypes)m->getType()) {

	// discovery request
	case typeDiscovery: {
		// decapsulate message
		Discovery* dmsg = m->decapsulate<Discovery> ();
		logging_debug("Received discovery message with"
			    << " src=" << m->getSourceNode().toString()
				<< " dst=" << m->getDestinationNode().toString()
				<< " ttl=" << (int)dmsg->getTTL()
				<< " type=" << (int)dmsg->getType()
		);

		// add discovery node id
		bool found = false;
		BOOST_FOREACH( NodeID& value, discovery )
			if (value == m->getSourceNode()) {
				found = true;
				break;
			}
		if (!found) discovery.push_back(m->getSourceNode());

		// check if source node can be added to routing table and setup link
		if (m->getSourceNode() != nodeid)
			setup( dmsg->getEndpoint(), m->getSourceNode() );

		// process discovery message -------------------------- switch start --
		switch (dmsg->getType()) {

		// normal: route discovery message like every other message
		case Discovery::normal: {
			// closest node? yes-> split to follow successor and predecessor
			if ( table->is_closest_to(m->getDestinationNode()) ) {
				logging_debug("Discovery split:");
				if (!table->get_successor()->isUnspecified()) {
					OverlayMsg omsg(*m);
					dmsg->setType(Discovery::successor);
					omsg.encapsulate(dmsg);
					logging_debug("* Routing to successor "
							<< table->get_successor()->toString() );
					baseoverlay.send( &omsg, *table->get_successor() );
				}

				// send predecessor message
				if (!table->get_predesessor()->isUnspecified()) {
					OverlayMsg omsg(*m);
					dmsg->setType(Discovery::predecessor);
					omsg.encapsulate(dmsg);
					logging_debug("* Routing to predecessor "
							<< table->get_predesessor()->toString() );
					baseoverlay.send( &omsg, *table->get_predesessor() );
				}
			}
			// no-> route message
			else {
				baseoverlay.route( m );
			}
			break;
		}

		// successor mode: follow the successor until TTL is zero
		case Discovery::successor:
		case Discovery::predecessor: {
			// reached destination? no->forward!
			if (m->getDestinationNode() != nodeid) {
				OverlayMsg omsg(*m);
				omsg.encapsulate(dmsg);
				omsg.setService(OverlayInterface::OVERLAY_SERVICE_ID);
				baseoverlay.route( &omsg );
				break;
			}

			// time to live ended? yes-> stop routing
			if (dmsg->getTTL() == 0 || dmsg->getTTL() > 10) break;

			// decrease time-to-live
			dmsg->setTTL(dmsg->getTTL() - 1);

			const route_item* item = NULL;
			if (dmsg->getType() == Discovery::successor &&
					table->get_successor() != NULL) {
				item = table->get(*table->get_successor());
			} else {
				if (table->get_predesessor()!=NULL)
					item = table->get(*table->get_predesessor());
			}
			if (item == NULL)
				break;

			logging_debug("Routing discovery message to succ/pred "
				<< item->id.toString() );
			OverlayMsg omsg(*m);
			omsg.encapsulate(dmsg);
			omsg.setDestinationNode(item->id);
			omsg.setService(OverlayInterface::OVERLAY_SERVICE_ID);
			baseoverlay.send(&omsg, omsg.getDestinationNode());
			break;
		}
		case Discovery::invalid:
			break;

		default:
			break;
		}
		// process discovery message ---------------------------- switch end --

		delete dmsg;
		break;
	}

	// leave
	case typeLeave: {
		if (link!=LinkID::UNSPECIFIED) {
			route_item* item = table->get(remote);
			if (item!=NULL) item->info = LinkID::UNSPECIFIED;
			table->remove(remote);
			baseoverlay.dropLink(link);
		}
		break;
	}}
}

void Chord::eventFunction() {
	stabilize_counter++;
	if (stabilize_counter < 0 || stabilize_counter == 2) {

		// reset counter
		stabilize_counter = 0;

		// clear pending connections
		pending.clear();

		// get number of real neighbors
		size_t numNeighbors = 0;
		for (size_t i = 0; i < table->size(); i++) {
			route_item* it = (*table)[i];
			if (it->ref_count != 0 && !it->info.isUnspecified()) numNeighbors++;
		}
		logging_info("Running stabilization: #links="
				<< table->size() << " #neighbors=" << numNeighbors );

		// updating neighbors
		logging_debug("Discover new ring neighbors");
		for (size_t i=0; i<table->size(); i++) {
			LinkID id = (*table)[i]->info;
			if (!id.isUnspecified()) discover_neighbors(id);
		}

		// sending discovery
		logging_debug("Sending discovery message to my neighbors and fingers");
		stabilize_finger = ((stabilize_finger+1) % table->get_finger_table_size() );
		const NodeID disc = table->get_finger_table(stabilize_finger).get_compare().get_center();
		if (disc != nodeid)
			send_discovery_to(disc);

		// remove orphan links
		orphan_removal_counter++;
		if (orphan_removal_counter <0 || orphan_removal_counter >= 2) {
			logging_info("Discovered nodes: ");
			BOOST_FOREACH( NodeID& id, discovery )
				logging_info("* " << id.toString());
			discovery.clear();
			logging_info("Running orphan removal");
			orphan_removal_counter = 0;
			for (size_t i = 0; i < table->size(); i++) {
				route_item* it = (*table)[i];
				if (it->ref_count == 0 && !it->info.isUnspecified()) {
					logging_info("Dropping orphaned link " << it->info.toString() << " to " << it->id.toString());
					table->insert(it->id);
					if (it->ref_count==0) {
						LinkID id = it->info;
						it->info = LinkID::UNSPECIFIED;
						baseoverlay.dropLink(id);
					}
				}
			}
		}
	}
}

void Chord::showLinks() {
	logging_info("--- chord routing information ----------------------------------");
	logging_info("predecessor: " << (table->get_predesessor()==NULL? "<none>" :
		table->get_predesessor()->toString()) );
	logging_info("node_id    : " << nodeid.toString() );
	logging_info("successor  : " << (table->get_successor()==NULL? "<none>" :
		table->get_successor()->toString()));
	logging_info("----------------------------------------------------------------");
}

/// @see OverlayInterface.h
std::string Chord::debugInformation() const {
	std::ostringstream s;
	s << "protocol   : Chord" << endl;
	s << "node_id    : " << nodeid.toString() << endl;
	s << "predecessor: " << (table->get_predesessor()==NULL? "<none>" :
		table->get_predesessor()->toString()) << endl;
	s << "successor  : " << (table->get_successor()==NULL? "<none>" :
		table->get_successor()->toString()) << endl;
	s << "nodes: " << endl;
	for (size_t i = 0; i < table->size(); i++) {
		route_item* it = (*table)[i];
		if (it->ref_count != 0 && !it->info.isUnspecified()) {
			s << it->id.toString().substr(0,6)
			  << " using " << it->info.toString().substr(0,6) << endl;
		}
	}
	return s.str();
}



}} // namespace ariba, overlay
