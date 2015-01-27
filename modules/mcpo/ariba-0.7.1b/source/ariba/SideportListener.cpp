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
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OF TELEMATICS OR
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

#include "SideportListener.h"

#include "ariba/overlay/BaseOverlay.h"
#include "ariba/overlay/LinkDescriptor.h"
#include "ariba/utility/addressing/endpoint_set.hpp"

using ariba::overlay::LinkDescriptor;

namespace ariba {

SideportListener SideportListener::DEFAULT;

SideportListener::SideportListener() : overlay(NULL) {
}

SideportListener::~SideportListener(){
}

string SideportListener::getEndpointDescription( const LinkID& link ) const {
	if( overlay == NULL ) {
		return "";
	}
	return overlay->getEndpointDescriptor(link).toString();
}

string SideportListener::getEndpointDescription( const NodeID& node ) const {
	if( overlay == NULL ) {
		return "";
	}
	return overlay->getEndpointDescriptor(node).toString();
}

const NodeID& SideportListener::getNodeID( const LinkID& link ) const {
	if( overlay == NULL ) return NodeID::UNSPECIFIED;
	return overlay->getNodeID(link);
}

vector<LinkID> SideportListener::getLinkIDs( const NodeID& node ) const {
	if( overlay == NULL ) return vector<LinkID>();
	return overlay->getLinkIDs( node );
}

string SideportListener::getHtmlLinks(){
	return overlay->getLinkHTMLInfo();
}

vector<NodeID> SideportListener::getOverlayNeighbors(bool deep){
	vector<NodeID> nodes;
	if( overlay == NULL ) return nodes;

	nodes = overlay->getOverlayNeighbors(deep);
	return nodes;
}

bool SideportListener::isRelayedNode(const NodeID& node){
	if( overlay == NULL ) return false;

	bool relay = false;

	BOOST_FOREACH( LinkDescriptor* link, overlay->links ){

		// is we find a direct connection this is not a relayed node
		if(link->relayed == false && link->remoteNode == node && link->up)
			return false;

		// if we find a relay connection this can be a relayed node
		// but only if we find no direct connection as above
		if( link->relayed && link->remoteNode == node && link->up)
			relay = true;
	}

	return relay;
}

bool SideportListener::isRelayingNode(const NodeID& node){
	if( overlay == NULL ) return false;

	vector<NodeID> directnodes;
	BOOST_FOREACH( LinkDescriptor* link, overlay->links ){
		if(link == NULL) continue;

		BOOST_FOREACH(NodeID route, link->routeRecord){
			if(route == node) return true;
		}
	}

	return false;
}

SideportListener::Protocol SideportListener::getReachabilityProtocol(const NodeID& node){
	int ret = SideportListener::undefined;
	if( overlay == NULL ) return (Protocol)ret;

	using namespace ariba::addressing;

	LinkDescriptor* link = NULL;
	BOOST_FOREACH( LinkDescriptor* lnk, overlay->links ){
		if(lnk->up && lnk->remoteNode == node && !lnk->relayed && lnk->communicationUp){
			link = lnk;
			break;
		}
	}

	if (link == NULL) return (Protocol)ret;

	BaseCommunication::LinkDescriptor& bclink =
			overlay->bc->queryLocalLink(link->communicationId);

	if(bclink.isUnspecified() || bclink.remoteLocator == NULL) return (Protocol)ret;

	const address_v* locator = bclink.remoteLocator;

	if( locator->instanceof<tcpip_endpoint>() ){
		tcpip_endpoint tcpip = *locator;

		if( tcpip.address().is_v4() || tcpip.address().is_v4_mapped() ){
			ret = SideportListener::ipv4;
		}else if( tcpip.address().is_v6() ){
			ret = SideportListener::ipv6;
		}

	}else if( locator->instanceof<rfcomm_endpoint>() ){
		ret = SideportListener::rfcomm;
	}

	return (Protocol)ret;
}

void SideportListener::configure( overlay::BaseOverlay* _overlay ) {
	overlay = _overlay;
}

void SideportListener::onLinkUp(const LinkID& lnk, const NodeID& local, const NodeID& remote, const SpoVNetID& spovnet){
}

void SideportListener::onLinkDown(const LinkID& lnk, const NodeID& local, const NodeID& remote, const SpoVNetID& spovnet){
}

void SideportListener::onLinkChanged(const LinkID& lnk, const NodeID& local, const NodeID& remote, const SpoVNetID& spovnet){
}

void SideportListener::onLinkFail(const LinkID& lnk, const NodeID& local, const NodeID& remote, const SpoVNetID& spovnet){
}

} // namespace ariba
