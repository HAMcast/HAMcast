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

#ifndef OVERLAY_MSG_H__
#define OVERLAY_MSG_H__

#include <boost/cstdint.hpp>

#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"
#include "ariba/utility/types/ServiceID.h"
#include "ariba/utility/types/NodeID.h"
#include "ariba/utility/types/LinkID.h"
#include "ariba/communication/EndpointDescriptor.h"


namespace ariba {
namespace overlay {

using ariba::utility::LinkID;
using ariba::utility::NodeID;
using ariba::utility::ServiceID;
using ariba::utility::Message;
using ariba::communication::EndpointDescriptor;
using_serialization;

/**
 * A general purpose overlay message that is used to exchange messages
 * between nodes.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class OverlayMsg: public Message { VSERIALIZEABLE;
public:
	/// message types, is: uint8_t
	enum type_ { // is: uint8_t
		typeInvalid     = 0x00, ///< invalid, unspecified type

		// data transfer
		maskTransfer    = 0x10, ///< bit mask for transfer messages
		typeData        = 0x11, ///< message contains data for higher layers

		// join signaling
		maskJoin        = 0x20, ///< bit mask for join messages
		typeJoinRequest = 0x21, ///< join request
		typeJoinReply   = 0x22, ///< join reply

		// link messages
		maskLink        = 0x30, ///< bit mask for link messages
		typeLinkRequest = 0x31, ///< request a new link
		typeLinkReply   = 0x32, ///< link request reply
		typeLinkUpdate  = 0x33, ///< update message for link association
		typeLinkDirect  = 0x34, ///< direct connection has been established
		typeLinkAlive   = 0x35, ///< keep-alive message

		// DHT routed messages
		maskDHT			= 0x40, ///< bit mask for dht messages
		typeDHTPut      = 0x41, ///< DHT put operation
		typeDHTGet      = 0x42, ///< DHT get operation
		typeDHTRemove   = 0x43, ///< DHT remove operation

		/// DHT response messages
		maskDHTResponse = 0x50, ///< bit mask for dht responses
		typeDHTData     = 0x51, ///< DHT get data

		// topology signaling
		typeSignalingStart = 0x80, ///< start of the signaling types
		typeSignalingEnd = 0xFF    ///< end of the signaling types
	};

	/// default constructor
	OverlayMsg(
		uint8_t type = typeInvalid,
		const ServiceID& _service      = ServiceID::UNSPECIFIED,
		const NodeID& _sourceNode      = NodeID::UNSPECIFIED,
		const NodeID& _destinationNode = NodeID::UNSPECIFIED,
		const LinkID& _sourceLink      = LinkID::UNSPECIFIED,
		const LinkID& _destinationLink = LinkID::UNSPECIFIED )
	:	type(type), flags(0), hops(0), ttl(10),
		service(_service),
		sourceNode(_sourceNode), destinationNode(_destinationNode),
		sourceLink(_sourceLink), destinationLink(_destinationLink),
		routeRecord() {
		if (!_sourceLink.isUnspecified() || !_destinationLink.isUnspecified())
			setLinkMessage(true);
	}

	// copy constructor
	OverlayMsg(const OverlayMsg& rhs)
	:	type(rhs.type), flags(rhs.flags), hops(rhs.hops), ttl(rhs.ttl),
		service(rhs.service),
		sourceNode(rhs.sourceNode), destinationNode(rhs.destinationNode),
		sourceLink(rhs.sourceLink), destinationLink(rhs.destinationLink),
		routeRecord(rhs.routeRecord) {
	}

	/// destructor
	~OverlayMsg();

	/// type -------------------------------------------------------------------

	type_ getType() const {
		return (type_) type;
	}

	void setType( type_ type ) {
		this->type = type;
	}

	bool hasTypeMask( type_ mask ) const {
		return (type & (uint8_t)mask) == (uint8_t)mask;
	}

	/// flags ------------------------------------------------------------------

	bool isRelayed() const {
		return (flags & 0x01)!=0;
	}

	void setRelayed( bool relayed = true ) {
		if (relayed) flags |= 1; else flags &= ~1;
	}

	bool isRegisterRelay() const {
		return (flags & 0x02)!=0;
	}

	void setRegisterRelay( bool relayed = true ) {
		if (relayed) flags |= 0x02; else flags &= ~0x02;
	}

	bool isRouteRecord() const {
		return (flags & 0x04)!=0;
	}

	void setRouteRecord( bool route_record = true ) {
		if (route_record) flags |= 0x04; else flags &= ~0x04;
	}

	bool isAutoLink() const {
		return (flags & 0x80) == 0x80;
	}

	void setAutoLink(bool auto_link = true ) {
		if (auto_link) flags |= 0x80; else flags &= ~0x80;
	}

	bool isLinkMessage() const {
		return (flags & 0x40)!=0;
	}

	void setLinkMessage(bool link_info = true ) {
		if (link_info) flags |= 0x40; else flags &= ~0x40;
	}

	bool containsSourceEndpoint() const {
		return (flags & 0x20)!=0;
	}

	void setContainsSourceEndpoint(bool contains_endpoint) {
		if (contains_endpoint) flags |= 0x20; else flags &= ~0x20;
	}

	bool isDHTMessage() const {
		return hasTypeMask(maskDHT);
	}

	/// number of hops and time to live ----------------------------------------

	uint8_t getNumHops() const {
		return hops;
	}

	void setNumHops( uint8_t hops ) {
		this->hops = hops;
	}

	uint8_t increaseNumHops() {
		hops++;
		return hops;
	}

	uint8_t getTimeToLive() const {
		return ttl;
	}

	void setTimeToLive( uint8_t ttl ) {
		this->ttl = ttl;
	}

	/// addresses and links ----------------------------------------------------

	const ServiceID& getService() const {
		return service;
	}

	void setService( const ServiceID& service ) {
		this->service = service;
	}

	const NodeID& getSourceNode() const {
		return sourceNode;
	}

	void setSourceNode( const NodeID& node ) {
		this->sourceNode = node;
	}

	const NodeID& getDestinationNode() const {
		return destinationNode;
	}

	void setDestinationNode( const NodeID& node ) {
		this->destinationNode = node;
	}

	const LinkID& getSourceLink() const {
		return sourceLink;
	}

	void setSourceLink( const LinkID& link ) {
		this->sourceLink = link;
		setLinkMessage();
	}

	const LinkID& getDestinationLink() const {
		return destinationLink;
	}

	void setDestinationLink( const LinkID& link ) {
		this->destinationLink = link;
		setLinkMessage();
	}

	void setSourceEndpoint( const EndpointDescriptor& endpoint ) {
		sourceEndpoint = endpoint;
		setContainsSourceEndpoint(true);
	}

	const EndpointDescriptor& getSourceEndpoint() const {
		return sourceEndpoint;
	}

	/// swaps source and destination
	void swapRoles() {
		NodeID dummyNode = sourceNode;
		sourceNode = destinationNode;
		destinationNode = dummyNode;
		LinkID dummyLink = sourceLink;
		sourceLink = destinationLink;
		destinationLink = dummyLink;
		hops = 0;
	}

	const vector<NodeID> getRouteRecord() const {
		return routeRecord;
	}

	void addRouteRecord( const NodeID& node ) {
		if (isRouteRecord())
			routeRecord.push_back(node);
	}

private:
	uint8_t type, flags, hops, ttl;
	ServiceID service;
	NodeID sourceNode;
	NodeID destinationNode;
	LinkID sourceLink;
	LinkID destinationLink;
	EndpointDescriptor sourceEndpoint;
	vector<NodeID> routeRecord;
};

}} // ariba::overlay

/// serialization
sznBeginDefault( ariba::overlay::OverlayMsg, X ){
	// header
	X && type && flags && hops && ttl;

	// addresses
	X && &service && &sourceNode && &destinationNode;

	// message is associated with a end-to-end link
	if (isLinkMessage())
		X && &sourceLink && &destinationLink;

	// message is associated with a source end-point
	if (containsSourceEndpoint())
		X && sourceEndpoint;

	// message should record its route
	if (isRouteRecord()) {
		uint8_t size = routeRecord.size();
		X && size;
		if (X.isDeserializer()) routeRecord.resize(size);
		for (uint8_t i=0;i<size; i++) X && &routeRecord[i];
	}

	// payload
	X && Payload();
} sznEnd();

#endif // OVERLAY_MSG_H__
