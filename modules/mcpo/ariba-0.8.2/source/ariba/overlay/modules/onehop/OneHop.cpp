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

#include "OneHop.h"
#include "ariba/overlay/BaseOverlay.h"

#include "ariba/overlay/modules/onehop/messages/OneHopMessage.h"
#include "ariba/overlay/modules/onehop/messages/NodeListingRequest.h"
#include "ariba/overlay/modules/onehop/messages/NodeListingReply.h"

namespace ariba {
namespace overlay {

use_logging_cpp( OneHop );

OneHop::OneHop(BaseOverlay& _baseoverlay, const NodeID& _nodeid,
		OverlayStructureEvents* _eventsReceiver, const OverlayParameterSet& param)
	: 	OverlayInterface( _baseoverlay, _nodeid, _eventsReceiver, param ),
		state( OneHopStateInvalid ) {

	//
	// insert us as the first node in the overlay
	//
	overlayNodes.insert( make_pair(_nodeid, LinkID::UNSPECIFIED) );

	Timer::setInterval(5000);
	Timer::start();
}

OneHop::~OneHop(){
	Timer::stop();
	deleteOverlay();
}

const EndpointDescriptor& OneHop::resolveNode(const NodeID& node){

	OverlayNodeMapping::const_iterator i = overlayNodes.find( node );
	if (i == overlayNodes.end()) return EndpointDescriptor::UNSPECIFIED();

	const EndpointDescriptor& ep = baseoverlay.getEndpointDescriptor( i->second );

	logging_debug( "resolved node " << node.toString() << " to endpoint " << ep.toString() );
	return ep;
}


/// @see OverlayInterface.h
bool OneHop::isClosestNodeTo( const NodeID& node ) {
	throw "NOT IMPLEMENTED!";
	return false;
}

void OneHop::routeMessage(const NodeID& destnode, Message* msg){

	// in the fullmesh overlay we know every other node
	// so we also have a link to each other node

	logging_debug( "routing message to node " << destnode.toString() );

	// msg for ourselfs
	if(destnode == nodeid)
		baseoverlay.incomingRouteMessage( msg, LinkID::UNSPECIFIED, nodeid );

	// msg for other node
	OverlayNodeMapping::const_iterator i = overlayNodes.find( destnode );
	if (i == overlayNodes.end()) {
		logging_error( "not able to route message to node " << destnode.toString() );
		return;
	}
	OneHopMessage onehopRoute( OneHopMessage::OneHopMessageTypeRoute );
	onehopRoute.encapsulate(msg);

	baseoverlay.sendMessage( &onehopRoute, i->second );
}

void OneHop::routeMessage(const NodeID& node, const LinkID& link, Message* msg) {
	OneHopMessage onehopRoute( OneHopMessage::OneHopMessageTypeRoute );
	onehopRoute.encapsulate(msg);
	baseoverlay.sendMessage( &onehopRoute, link );
}

/// @see OverlayInterface.h
const LinkID& OneHop::getNextLinkId( const NodeID& id ) const {
	OverlayNodeMapping::const_iterator i = overlayNodes.find( id );
	if (i == overlayNodes.end()) return LinkID::UNSPECIFIED;
	return i->second;
}

/// @see OverlayInterface.h
const NodeID& OneHop::getNextNodeId( const NodeID& id ) const {
	OverlayNodeMapping::const_iterator i = overlayNodes.find( id );
	
	// FIXME: in case the NodeID is not known we should return the nearest node
	if (i == overlayNodes.end()) {
		return NodeID::UNSPECIFIED;
	}
	
	return i->first;
}

void OneHop::createOverlay() {
	// don't need to bootstrap against ourselfs.
	// the create and join process is completed now.
	logging_info( "creating onehop overlay structure" );
}

void OneHop::deleteOverlay(){

	logging_info( "deleting onehop overlay structure" );
	state = OneHopStateInvalid;
}

OverlayInterface::NodeList OneHop::getKnownNodes(bool deep) const {

	OverlayInterface::NodeList retlist;

	OverlayNodeMapping::const_iterator i = overlayNodes.begin();
	OverlayNodeMapping::const_iterator iend = overlayNodes.end();

	for( ; i != iend; i++ )
		retlist.push_back( i->first );

	return retlist;
}

void OneHop::joinOverlay(const EndpointDescriptor& bootstrapEp){

	logging_info( "joining onehop overlay structure through end-point " <<
			(bootstrapEp.isUnspecified() ? "local" : bootstrapEp.toString()) );

	if( bootstrapEp.isUnspecified() ){

		// we are the initiator and we are to bootstrap against
		// ourselfs. in the onehop overlay this is not an issue
		// and we can just ignore this call.

		state = OneHopStateCompleted;
	} else {
		bootstrapLinks.push_back(
				baseoverlay.establishDirectLink( bootstrapEp,
					OverlayInterface::OVERLAY_SERVICE_ID )
					);
	}
}

void OneHop::leaveOverlay(){

	logging_info( "leaving onehop overlay structure" );

	// set the state to invalid, this will prevent from
	// handling onLinkDown events, as we are traversing the
	// overlayNodes map and the onLinkDown function is called
	// from the BaseOverlay and OneHop::onLinkDown will also
	// try to access the overlayNodes structure.
	state = OneHopStateInvalid;

	//
	// send leave messages to all nodes. the nodes
	// will then drop the links
	//

	OverlayNodeMapping::iterator i = overlayNodes.begin();
	OverlayNodeMapping::iterator iend = overlayNodes.end();

	for( ; i != iend; i++){
		if( i->first != nodeid && i->second != LinkID::UNSPECIFIED ){

			OneHopMessage msg (OneHopMessage::OneHopMessageTypeLeave);
			baseoverlay.sendMessage( &msg, i->second );
		}
	}
}


void OneHop::onLinkDown(const LinkID& lnk, const NodeID& remote){

	// don't handle when we are in state-invalid,
	// see comment in OneHop::leaveOverlay
	if( state == OneHopStateInvalid ) return;

	// node went down, remove from overlay mapping
	logging_debug( "link " << lnk.toString() << " to node " << remote.toString() << " went down, removing node" );

	OverlayNodeMapping::iterator i = overlayNodes.begin();
	OverlayNodeMapping::iterator iend = overlayNodes.end();

	for( ; i != iend; i++ ){
		if( i->second == lnk ){
			overlayNodes.erase( i );
			break;
		}
	}

	vector<LinkID>::iterator it = std::find( bootstrapLinks.begin(), bootstrapLinks.end(), lnk );
	if( it != bootstrapLinks.end() ) bootstrapLinks.erase( it );
}

void OneHop::onLinkUp(const LinkID& lnk, const NodeID& remote){

	logging_debug( "link is up, sending out node listing request" );

	NodeListingRequest requestmsg;
	OneHopMessage onemsg( OneHopMessage::OneHopMessageTypeListingRequest );
	onemsg.encapsulate( &requestmsg );

	baseoverlay.sendMessage( &onemsg, lnk );
}

void OneHop::onMessage(const DataMessage& msg, const NodeID& remote, const LinkID& lnk){

	OneHopMessage* onemsg = msg.getMessage()->convert<OneHopMessage>();
	if( onemsg == NULL ) return;

	//
	// handle node listing request
	//

	if( onemsg->isType( OneHopMessage::OneHopMessageTypeListingRequest ) ){

		//NodeListingRequest* request = onemsg->decapsulate<NodeListingRequest>();

		logging_info( "onehop received node listing request from node " << remote.toString() );

		//
		// first, insert the nodes and the link into our mapping
		//

		overlayNodes.insert( make_pair(remote, lnk) );

		//
		// send back a message with all nodes
		// and their current EndpointDescriptor
		//

		OneHopMessage onehopReply( OneHopMessage::OneHopMessageTypeListingReply );
		NodeListingReply listingReply;

		OverlayNodeMapping::iterator i = overlayNodes.begin();
		OverlayNodeMapping::iterator iend = overlayNodes.end();

		logging_debug( "sending out node listing reply with the following items" );

		for( ; i != iend; i++ ){

			const NodeID node = i->first;
			const LinkID link = i->second;
			const EndpointDescriptor& endpoint = baseoverlay.getEndpointDescriptor( link );

			logging_debug( "node: " + node.toString() + ", endp: " + endpoint.toString());
			listingReply.add( node, const_cast<EndpointDescriptor*>(new EndpointDescriptor(endpoint)) );
		}

		onehopReply.encapsulate( &listingReply );
		baseoverlay.sendMessage( &onehopReply, lnk );

		//
		// now that we know the node, we can tell the baseoverlay
		// that the node has joined our overlay structure
		//

		eventsReceiver->onNodeJoin( remote );

	} // OneHopMessageTypeListingRequest

	//
	// handle node listing reply
	//

	if( onemsg->isType( OneHopMessage::OneHopMessageTypeListingReply) ){

		NodeListingReply* reply = onemsg->decapsulate<NodeListingReply>();

		logging_debug( "received node listing reply from node " << remote.toString()
					<< " with all overlay nodes. connecting to all of them" );

		//
		// get out all the EndpointDescriptors from the
		// overlay nodes and connect to all nodes where
		// we don't have a link yet
		//

		const NodeListingReply::NodeEndpointList& endpoints = reply->getList();
		logging_debug( "received " << endpoints.size() << " nodes in listing" );

		NodeListingReply::NodeEndpointList::const_iterator i = endpoints.begin();
		NodeListingReply::NodeEndpointList::const_iterator iend = endpoints.end();

		for( ; i != iend; i++ ){

			//
			// don't connect to nodes that we already have
			// a link to and don't connect to ourself
			//

			const NodeID& node = (*i).first;
			if( overlayNodes.find(node) != overlayNodes.end() ) continue;
			if( node == nodeid ) continue;

			logging_debug( "building up link to node in overlay " << node.toString() );
			const LinkID link = baseoverlay.establishDirectLink( *((*i).second),
							OverlayInterface::OVERLAY_SERVICE_ID );

			overlayNodes.insert( make_pair(node, link) );

		} // for( ; i != iend; i++ )

		delete reply;
	} // OneHopMessageTypeListingReply

	//
	// handle node leaves
	//

	if( onemsg->isType(OneHopMessage::OneHopMessageTypeLeave) ){

		logging_debug("received leave message from " <<
				remote.toString() << " on link " << lnk.toString());

		// drop the link to the node
		baseoverlay.dropLink( lnk );

	} // OneHopMessageTypeLeave

	//
	// handle kbr route messages
	//

	if( onemsg->isType( OneHopMessage::OneHopMessageTypeRoute) ){
		logging_debug( "Route message arrived at destination node -> delegate to BaseOverlay" );
		baseoverlay.incomingRouteMessage( onemsg, lnk, remote);
	} // OneHopMessageTypeRoute

	delete onemsg;
}

void OneHop::eventFunction(){

	logging_debug("<<<<<<<<<<<<<<<<onehop-table<<<<<<<<<<<<<<<<<<<");

		OverlayNodeMapping::iterator i = overlayNodes.begin();
		OverlayNodeMapping::iterator iend = overlayNodes.end();

		for( ; i != iend; i++ ){

			const NodeID node = i->first;
			const LinkID link = i->second;
			const EndpointDescriptor& endpoint = baseoverlay.getEndpointDescriptor( link );

			logging_debug( 	"node: " << node.toString() <<
							", link_: " << link.toString() << ", endp: " << endpoint.toString());
		}

	logging_debug(">>>>>>>>>>>>>>>>>onehop-table>>>>>>>>>>>>>>>>>>>>>");

}

}} // namespace ariba, overlay
