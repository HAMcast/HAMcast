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

#ifndef NODE_H_
#define NODE_H_

// forward declarations
namespace ariba {
	class Node;
	namespace overlay {
		class BaseOverlay;
	}
}

#include <vector>
#include <iostream>
#include <boost/foreach.hpp>
#include "Module.h"
#include "Identifiers.h"
#include "SpoVNetProperties.h"
#include "NodeListener.h"
#include "Name.h"
#include "AribaModule.h"
#include "CommunicationListener.h"
#include "DataMessage.h"
#include "SideportListener.h"

using std::vector;
using ariba::overlay::BaseOverlay;

namespace ariba {

/**
 * \addtogroup public
 * @{
 *
 * This class should implement all ariba node functionality.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 * @author Christoph Mayer <mayer@tm.uka.de>
 */
class Node: public Module {
public:
	/**
	 * Constructs a new node using a given ariba module
	 *
	 * @param ariba_mod The ariba module
	 * @param name The canonical node name of the new node. When NULL a
	 *   randomly chosen name is created.
	 * @param len The length of the canonical node name or -1, if the name
	 *   is a zero-terminated char-string.
	 */
	Node(AribaModule& ariba_mod, const Name& node_name = Name::UNSPECIFIED);

	/**
	 * Destroys the node. Before destruction some pre-conditions
	 * must be met:<br />
	 *
	 * 1. The node is not part of any SpoVNet <br />
	 * 2. All listeners must be unbound from this node <br />
	 * 3. The module has been stopped<br />
	 */
	virtual ~Node();

	//--- node control ---

	/**
	 * This method instructs the node to join a particular spovnet.
	 * Callees may bind with a NodeListener to receive events, when
	 * a node has been successfully joined.
	 *
	 * @param vnetId The SpoVNet name
	 */
	void join(const Name& name);

	/**
	 * This method initiates a new SpoVNet with the given SpoVNetID and
	 * parameters.
	 *
	 * @param name The SpoVNet name
	 * @param param The SpoVNet properties
	 */
	void initiate(const Name& name, const SpoVNetProperties& parm =
			SpoVNetProperties::DEFAULT);

	/**
	 * This method initiates the leave procedure of this node.
	 */
	void leave();

	/**
	 * This method is used to bind a node listener to this node.
	 *
	 * @param listener The node listener
	 * @return boolean indicating success of failure
	 */
	bool bind(NodeListener* listener);

	/**
	 * This method is used to unbind a node listener to this node.
	 *
	 * @param listener The node listener
	 * @return boolean indicating success of failure
	 */
	bool unbind(NodeListener* listener);

	//--- spovnet properties ---

	/**
	 * Returns the properties of the spovnet the node has joined.
	 *
	 * @return The properties of the spovnet the node has joined
	 */
	const SpoVNetProperties& getSpoVNetProperties() const;

	/**
	 * Returns the spovnet identifier
	 *
	 * @return The spovnet idenfifier
	 */
	const SpoVNetID& getSpoVNetId() const;

	/**
	 * Returns true, if the node is part of a spovnet.
	 *
	 * @return True, if the node is part of a spovnet
	 */
	bool isJoined() const;

	//--- addressing ---

	/**
	 * Returns the node id of this node if the link id is unspecified or
	 * the node id of the remote node.
	 *
	 * @return The local or the remote node identifier
	 */
	const NodeID& getNodeId(const LinkID& lid = LinkID::UNSPECIFIED) const;

	/**
	 * Returns the node id to a node name according to the currently joined
	 * spovnet (usually derives a node identifier by hashing the name).
	 *
	 * @return The node identifier to the given name
	 */
	NodeID generateNodeId(const Name& name) const;

	/**
	 * Returns the name of this node if the link id is unspecified or
	 * the node name of the remote node, if known -- otherwise it returns
	 * an unspecified name.
	 *
	 * @return A node's name or an unspecified name, if unknown
	 */
	const Name getNodeName(const LinkID& lid = LinkID::UNSPECIFIED) const;

	/**
	 * Get a list of neighboring nodes in the overlay structure.
	 * The number and identities of nodes depends highly on the
	 * used overlay structure.
	 *
	 * @return a list of NodeIDs that are neighbors in the overlay structure
	 * @see sendBroadcastMessage
	 */
	vector<NodeID> getNeighborNodes() const;

	//--- communication ---

	/**
	 * Establishes a new link to another node and service with the given
	 * link properties. An optional message could be sent with the request.
	 *
	 * @param nid The remote node identifier
	 * @param sid The remote service identifier
	 * @param req The required link properties
	 * @param msg An optional message that is sent with the request
	 * @return A new link id
	 */
	LinkID establishLink(const NodeID& nid, const ServiceID& sid);

	/**
	 * This method drops an established link.
	 *
	 * @param lnk The link identifier of an active link
	 */
	void dropLink(const LinkID& lnk);

	// message sending

	/**
	 * Sends a one-shot message to a service. If link properties are specified,
	 * the node tries to fulfill those requirements. This may cause the node
	 * to first establish a temporary link, second sending the message and last
	 * dropping the link. This would result in a small amount of extra latency
	 * until the message is delivered. If reliable transport was selected,
	 * the method returns a sequence number and a communication event is
	 * triggered on message delivery or loss.
	 *
	 * @param msg The message to be sent
	 * @param nid The remote node identifier
	 * @param sid The remote service identifier
	 * @param req The requirements associated with the message
	 * @return A sequence number
	 */
	seqnum_t sendMessage(const DataMessage& msg, const NodeID& nid, const ServiceID& sid,
			const LinkProperties& req = LinkProperties::DEFAULT);

	/**
	 * Sends a message via an established link. If reliable transport was
	 * selected, the method returns a sequence number and a communication event
	 * is triggered on message delivery or loss.
	 *
	 * @param msg The message to be sent
	 * @param lnk The link to be used for sending the message
	 */
	seqnum_t sendMessage(const DataMessage& msg, const LinkID& lnk);

	/**
	 * Sends a message to all known hosts in the overlay structure
	 * the nodes that are reached here depend on the overlay structure.
	 *
	 * @param msg The message to be send
	 * @param sid The id of the service that should receive the message
	 * @see getNeighborNodes
	 */
	void sendBroadcastMessage(const DataMessage& msg, const ServiceID& sid);

	// --- communication listeners ---

	/**
	 * Binds a listener to a specifed service identifier.
	 * Whenever a link is established/dropped or messages are received the
	 * events inside the interface are called.
	 *
	 * @param listener The listener to be registered
	 * @param sid The service identifier
	 * @return boolean indicating success of failure
	 */
	bool bind(CommunicationListener* listener, const ServiceID& sid);

	/**
	 * Un-binds a listener from this node.
	 *
	 * @param The listener to be unbound
	 * @return boolean indicating success of failure
	 */
	bool unbind(CommunicationListener* listener, const ServiceID& sid);

	/**
	 * Adds a key value pair to the DHT
	 *
	 * @param key The key data
	 * @param value The value data
	 * @param ttl The time to live in seconds
	 */
	void put( const Data& key, const Data& value, uint16_t ttl, bool replace = false);

	/**
	 * Queries for values stored in the DHT. Fires an communication event when
	 * values arrive.
	 *
	 * @param key The key data
	 * @param sid The service that is requesting the values
	 */
	void get( const Data& key, const ServiceID& sid );


	//-------------------------------------------------------------------------
	//
	// --- optimization proposal: allow direct endpoint descriptor exchange ---
	// main-idea: directly allow exchanging endpoint descriptors to establish
	// links. Depending on the overlay structure used in the base overlay, this
	// allows a node to directly establish links between two nodes when an
	// endpoint descriptor is known.
	//
	//const EndpointDescriptor& getEndpointDescriptor( const LinkID& lid );
	//void sendMessage( EndpointDescriptor& epd, Message* msg );
	//LinkID setupLink( const EndpointDescriptor& endpointDesc,
	//		const LinkProperties& req = LinkProperties::UNSPECIFIED,
	//		const Message* msg = NULL );
	//
	//-------------------------------------------------------------------------

	// --- module implementation ---
	//
	// main-idea: use module properties to configure nodeid, spovnetid etc. and
	// select start/stop procedures. This allows simulations to start a
	// node without manually calling join etc.

	/** @see Module.h */
	string getName() const;

	uint32_t getSendetPackageCount();

protected:
	// friends
	friend class AribaModule;

	// member variables
	Name name;                             //< node name
	AribaModule& ariba_mod;	               //< ariba module
	SpoVNetID spovnetId; 	               //< current spovnet id
	NodeID nodeId; 		              	   //< current node id
	overlay::BaseOverlay* base_overlay;    //< the base overlay
	uint32_t sendetPackages;

};

} // namespace ariba

/** @} */

#endif /* NODE_H_ */
