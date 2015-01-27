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

#include "Node.h"

#include "ariba/overlay/BaseOverlay.h"
#include "ariba/utility/types/OverlayParameterSet.h"
#include "ariba/communication/EndpointDescriptor.h"

using ariba::communication::EndpointDescriptor;

namespace ariba {

Node::Node(AribaModule& ariba_mod, const Name& node_name) :
	name(node_name), ariba_mod(ariba_mod), sendetPackages(0) {
	base_overlay = new BaseOverlay();
}

Node::~Node() {
	delete base_overlay;
	base_overlay = NULL;
}

void Node::join(const Name& vnetname) {
	spovnetId = vnetname.toSpoVNetId();
	nodeId = generateNodeId(name);

	// start base comm if not started
	if( !ariba_mod.base_comm->isStarted() )
		ariba_mod.base_comm->start();

	// start base overlay if not started
	// join against ourselfs
	if( !base_overlay->isStarted() )
		base_overlay->start( *ariba_mod.base_comm, nodeId );
	base_overlay->joinSpoVNet( spovnetId );

	// join against static bootstrap points and
	// start automatic bootstrapping modules
	vector<AribaModule::BootstrapMechanism> mechanisms
		= ariba_mod.getBootstrapMechanisms(vnetname);

	vector<pair<BootstrapManager::BootstrapType,string> > internalmodules;

	BOOST_FOREACH(AribaModule::BootstrapMechanism m, mechanisms){
		switch(m){
			case AribaModule::BootstrapMechanismStatic:
			{
				const communication::EndpointDescriptor* ep =
							ariba_mod.getBootstrapNode(vnetname, m);
					if( ep != NULL && ep->isUnspecified() == false )
						base_overlay->joinSpoVNet( spovnetId, *ep);
				break;
			}
			case AribaModule::BootstrapMechanismBroadcast:
				internalmodules.push_back(make_pair(
						BootstrapManager::BootstrapTypePeriodicBroadcast,
						ariba_mod.getBootstrapInfo(vnetname, m)));
				break;
			case AribaModule::BootstrapMechanismMulticastDNS:
				internalmodules.push_back(make_pair(
						BootstrapManager::BootstrapTypeMulticastDns,
						ariba_mod.getBootstrapInfo(vnetname, m)));
				break;
			case AribaModule::BootstrapMechanismSDP:
				internalmodules.push_back(make_pair(
						BootstrapManager::BootstrapTypeBluetoothSdp,
						ariba_mod.getBootstrapInfo(vnetname, m)));
				break;
			default:
				break;
		}
	}

	// start automatic overlay bootstrapping modules
	base_overlay->startBootstrapModules(internalmodules);

	// done
}

void Node::initiate(const Name& vnetname, const SpoVNetProperties& parm) {
	utility::OverlayParameterSet ovrpset;
	ovrpset.setOverlayStructure(
			(utility::OverlayParameterSet::_OverlayStructure)
			parm.getBaseOverlayType()
			);

	spovnetId = vnetname.toSpoVNetId();
	nodeId = generateNodeId(name);

	// start base comm if not started
	if( !ariba_mod.base_comm->isStarted() )
		ariba_mod.base_comm->start();

	// start base overlay if not started
	if( !base_overlay->isStarted() )
		base_overlay->start( *ariba_mod.base_comm, nodeId );

	base_overlay->createSpoVNet( spovnetId, ovrpset );
}

void Node::leave() {
	base_overlay->stopBootstrapModules();
	base_overlay->leaveSpoVNet();
	ariba_mod.base_comm->stop();
	base_overlay->stop();
}

const SpoVNetProperties& Node::getSpoVNetProperties() const {
	return SpoVNetProperties::DEFAULT;
}

const SpoVNetID& Node::getSpoVNetId() const {
	return spovnetId;
}

const NodeID& Node::getNodeId(const LinkID& lid) const {
	if( lid == LinkID::UNSPECIFIED ) return nodeId;
	else return base_overlay->getNodeID( lid );
}

NodeID Node::generateNodeId(const Name& name) const {
	if (name == Name::UNSPECIFIED) return Name::random().toNodeId();
	else return name.toNodeId();
}

vector<NodeID> Node::getNeighborNodes() const {
	return base_overlay->getOverlayNeighbors();
}

LinkID Node::establishLink(const NodeID& nid, const ServiceID& sid) {
	return base_overlay->establishLink(nid, sid);
}

void Node::dropLink(const LinkID& lnk) {
	base_overlay->dropLink(lnk);
}

seqnum_t Node::sendMessage(const DataMessage& msg, const NodeID& nid,
		const ServiceID& sid, const LinkProperties& req) {
	sendetPackages++;
	return base_overlay->sendMessage((Message*) msg, nid, sid);
}

seqnum_t Node::sendMessage(const DataMessage& msg, const LinkID& lnk) {
	
	return base_overlay->sendMessage((Message*) msg, lnk);
}

void Node::sendBroadcastMessage(const DataMessage& msg, const ServiceID& sid) {
	return base_overlay->broadcastMessage((Message*)msg, sid);
}

bool Node::bind(NodeListener* listener) {
    return base_overlay->bind(listener);
}

bool Node::unbind(NodeListener* listener) {
	return base_overlay->unbind(listener);
}

bool Node::bind(CommunicationListener* listener, const ServiceID& sid) {
	// bind the listener
	bool ret = base_overlay->bind(listener, sid);

	// now that we have a listener, we can ask if sniffing is ok
	if( ariba_mod.sideport_sniffer != NULL ){
		base_overlay->registerSidePort(ariba_mod.sideport_sniffer);
	}

	return ret;
}

bool Node::unbind(CommunicationListener* listener, const ServiceID& sid) {
	return base_overlay->unbind(listener, sid);
}

// service directory

void Node::put( const Data& key, const Data& value, uint16_t ttl, bool replace ) {
	base_overlay->dhtPut(key,value,ttl,replace);
}

void Node::get( const Data& key, const ServiceID& sid ) {
	base_overlay->dhtGet(key,sid);
}

// @see Module.h
string Node::getName() const {
	return name.toString();
}
uint32_t Node::getSendetPackageCount(){
	return sendetPackages;
}


} // namespace ariba
