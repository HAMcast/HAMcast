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

#ifndef BASEOVERLAY_H_
#define BASEOVERLAY_H_

#include <map>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <list>
#include <vector>
#include <deque>
#include <boost/foreach.hpp>

#include "ariba/utility/messages.h"
#include "ariba/utility/types.h"
#include "ariba/utility/misc/Helper.h"
#include "ariba/utility/misc/Demultiplexer.hpp"
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/system/Timer.h"

#include "ariba/communication/EndpointDescriptor.h"
#include "ariba/communication/BaseCommunication.h"
#include "ariba/communication/CommunicationEvents.h"

#include "ariba/overlay/modules/OverlayInterface.h"
#include "ariba/overlay/modules/OverlayFactory.h"
#include "ariba/overlay/modules/OverlayStructureEvents.h"
#include "ariba/overlay/OverlayBootstrap.h"

// forward declarations
namespace ariba {
  class NodeListener;
  class CommunicationListener;
  class SideportListener;
}

using std::vector;
using std::list;
using std::cout;
using std::map;
using std::make_pair;
using std::pair;
using std::find;
using std::deque;

// ariba interface
using ariba::NodeListener;
using ariba::SideportListener;
using ariba::CommunicationListener;

// overlay
using ariba::overlay::OverlayBootstrap;

// communication
using ariba::communication::EndpointDescriptor;
using ariba::communication::BaseCommunication;
using ariba::communication::CommunicationEvents;

// utilities
using ariba::utility::NodeID;
using ariba::utility::SpoVNetID;
using ariba::utility::LinkID;
using ariba::utility::Identifier;
using ariba::utility::ServiceID;
using ariba::utility::QoSParameterSet;
using ariba::utility::SecurityParameterSet;
using ariba::utility::Demultiplexer;
using ariba::utility::MessageReceiver;
using ariba::utility::MessageSender;
using ariba::utility::seqnum_t;
using ariba::utility::Timer;

namespace ariba {
namespace overlay {

using namespace ariba::addressing;

class LinkDescriptor;
class OverlayMsg;
class DHT;

class BaseOverlay: public MessageReceiver,
		public CommunicationEvents,
		public OverlayStructureEvents,
		protected Timer {

	friend class OneHop;
	friend class Chord;
	friend class ariba::SideportListener;

	use_logging_h( BaseOverlay );

public:

	/**
	 * Constructs an empty non-functional base overlay instance
	 */
	BaseOverlay();

	/**
	 * Destructs a base overlay instance
	 */
	virtual ~BaseOverlay();

	/**
	 * Starts the Base Overlay instance
	 */
	void start(BaseCommunication& _basecomm, const NodeID& _nodeid);

	/**
	 * Stops the Base Overlay instance
	 */
	void stop();

	/**
	 * Is the BaseOverlay instance started up yet
	 */
	bool isStarted();

	/// Tries to establish a direct or overlay link
	const LinkID establishLink(	const EndpointDescriptor& ep,
		const NodeID& node, const ServiceID& service );

	/**
	 * Starts a link establishment procedure to the specfied node
	 * for the service with id service
	 *
	 * @param node Destination node id
	 * @param service Service to connect to
	 * @param linkid Link identifier to be used with this link
	 */
	const LinkID establishLink(	const NodeID& remote,
		const ServiceID& service = OverlayInterface::OVERLAY_SERVICE_ID );

	/**
	 * Starts a link establishment procedure to the specified
	 * endpoint and to the specified service
	 */
	const LinkID establishDirectLink( const EndpointDescriptor& endpoint,
		const ServiceID& service = OverlayInterface::OVERLAY_SERVICE_ID );

	/// drops a link
	void dropLink( const LinkID& link );

	/// sends a message over an existing link
	seqnum_t sendMessage(const Message* message, const LinkID& link );

	/// sends a message to a node and a specific service
	seqnum_t sendMessage(const Message* message, const NodeID& remote,
		const ServiceID& service = OverlayInterface::OVERLAY_SERVICE_ID);

	/**
	 * Send out a message to all nodes that are known in the overlay structure.
	 * Depending on the structure of the overlay, this can be very different.
	 */
	void broadcastMessage(Message* message, const ServiceID& service);

	/**
	 * Returns the end-point descriptor of a link.
	 *
	 * @param link the link id of the requested end-point
	 * @return The end-point descriptor of the link's end-point
	 */
	const EndpointDescriptor& getEndpointDescriptor(
		const LinkID& link = LinkID::UNSPECIFIED) const;

	/**
	 * Get a list of overlay neighbors.
	 *
	 * @return A list of overlay neighbors.
	 */
	vector<NodeID> getOverlayNeighbors(bool deep = true) const;

	/**
	 * Returns a end-endpoint descriptor of a overlay neighbor.
	 * If the node is not known -- an unspecified endpoint descriptor is
	 * returned.
	 *
	 * @param node The node identifer of a overlay neighbor.
	 * @return The end-point descriptor of the node or unspecified.
	 */
	const EndpointDescriptor& getEndpointDescriptor(const NodeID& node) const;

	// TODO: Doc
	bool bind(CommunicationListener* listener, const ServiceID& sid);

	// TODO: Doc
	bool unbind(CommunicationListener* listener, const ServiceID& sid);

	// TODO: Doc
	bool bind(NodeListener* listener);

	// TODO: Doc
	bool unbind(NodeListener* listener);

	// TODO: Doc
	bool registerSidePort(SideportListener* _sideport);

	// TODO: Doc
	bool unregisterSidePort(SideportListener* _sideport);

	/**
	 * Returns the own nodeID or the NodeID of the specified link
	 *
	 * @param lid The link identifier
	 * @return The NodeID of the link
	 */
	const NodeID& getNodeID(const LinkID& lid = LinkID::UNSPECIFIED) const;

	/**
	 * Return all Links for the specified remote nodeid, or all links when
	 * the node id given is set to unspecified
	 *
	 * @param nid The node id to request links for, or unspecified for all links
	 * @return a vector that contains all the link ids requested
	 */
	vector<LinkID> getLinkIDs(const NodeID& nid = NodeID::UNSPECIFIED) const;

	/**
	 * Join a existing sponaneous virtual network (spovnet).
	 *
	 * @param id A spovnet identifier
	 * @param boot A bootstrap node
	 */
	void joinSpoVNet(const SpoVNetID& id, const EndpointDescriptor& boot = EndpointDescriptor::UNSPECIFIED());

	/**
	 * Initiates a new spontaneous virtual network.
	 * This makes this BaseOverlay to the SpoVNet-Initiator.
	 *
	 * @param id The spovnet identifier
	 */
	void createSpoVNet(const SpoVNetID& id, const OverlayParameterSet& param =
			OverlayParameterSet::DEFAULT, const SecurityParameterSet& sec =
			SecurityParameterSet::DEFAULT, const QoSParameterSet& qos =
			QoSParameterSet::DEFAULT);

	/**
	 * Start the bootstrap modules
	 */
	void startBootstrapModules(vector<pair<BootstrapManager::BootstrapType,string> > modules);

	/**
	 * Stop the bootstrap modules
	 */
	void stopBootstrapModules();

	/**
	 * Let the node leave the SpoVNet.
	 */
	void leaveSpoVNet();

	/// put a value to the DHT with a ttl given in seconds
	void dhtPut( const Data& key, const Data& value, int ttl = 0, bool replace = false, bool no_local_refresh = false);

	/// removes a key value pair from the DHT
	void dhtRemove( const Data& key, const Data& value );

	/// removes all data stored at the given key
	void dhtRemove( const Data& key );

	/// requests data stored using key
	void dhtGet( const Data& key, const ServiceID& service );

protected:
	/**
	 * @see ariba::communication::CommunicationEvents.h
	 */
	virtual void onLinkUp(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * @see ariba::communication::CommunicationEvents.h
	 */
	virtual void onLinkDown(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * @see ariba::communication::CommunicationEvents.h
	 */
	virtual void onLinkChanged(const LinkID& id,
		const address_v* oldlocal, const address_v* newlocal,
		const address_v* oldremote, const address_v* newremote);

	/**
	 * @see ariba::communication::CommunicationEvents.h
	 */
	virtual void onLinkFail(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * @see ariba::communication::CommunicationEvents.h
	 */
	virtual void onLinkQoSChanged(const LinkID& id,
		const address_v* local, const address_v* remote,
		const QoSParameterSet& qos);

	/**
	 * @see ariba::communication::CommunicationEvents.h
	 */
	virtual bool onLinkRequest(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * Processes a received message from BaseCommunication
	 *
	 * In case of a message routed by the overlay the source identifies
	 * the node the message came from!
	 */
	virtual bool receiveMessage( const Message* message, const LinkID& link,
		const NodeID& );

	/**
	 * This method is called, when a new node joined the network
	 *
	 * @see OverlayStructureEvents.h
	 */
	virtual void onNodeJoin(const NodeID& node);

	/**
	 * Timer Event method
	 */
	virtual void eventFunction();

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	std::string getLinkHTMLInfo();

private:
	/// is the base overlay started yet
	bool started;

	/// The state of the BaseOverlay
	typedef enum _BaseOverlayState {
		BaseOverlayStateInvalid = 0,
		BaseOverlayStateCompleted = 1,
	} BaseOverlayState;

	BaseOverlayState state;         ///< Current Base-Overlay state
	BaseCommunication* bc;          ///< reference to the base communication
	NodeID nodeId;                  ///< the node id of this node
	SpoVNetID spovnetId;            ///< the spovnet id of the currently joined overlay
	vector<LinkID> bootstrapLinks;  ///< the link id of the link to the initiator node
	NodeID spovnetInitiator;        ///< The initiator node

	/// the service id communication listeners
	Demultiplexer<CommunicationListener*, ServiceID> communicationListeners;
	CommunicationListener* getListener( const ServiceID& id );

	/// the node listeners
	typedef vector<NodeListener*> NodeListenerVector;
	NodeListenerVector nodeListeners;

	/// the sideport listener
	SideportListener* sideport;

	/// the used overlay structure
	OverlayInterface* overlayInterface;

	/// Bootstrapper for our spovnet
	OverlayBootstrap overlayBootstrap;

	// message handlers --------------------------------------------------------

	/// demultiplexes a incoming message with link descriptor
	bool handleMessage( const Message* message, LinkDescriptor* ld,
		const LinkID bcLink = LinkID::UNSPECIFIED );

	// handle data and signalling messages
	bool handleData( OverlayMsg* msg, LinkDescriptor* ld );
	bool handleSignaling( OverlayMsg* msg, LinkDescriptor* ld );

	// handle join request / reply messages
	bool handleJoinRequest( OverlayMsg* msg, const LinkID& bcLink );
	bool handleJoinReply( OverlayMsg* msg, const LinkID& bcLink );

	// handle DHT messages
	bool handleDHTMessage( OverlayMsg* msg );

	// handle link messages
	bool handleLinkRequest( OverlayMsg* msg, LinkDescriptor* ld );
	bool handleLinkReply( OverlayMsg* msg, LinkDescriptor* ld );
	bool handleLinkUpdate( OverlayMsg* msg, LinkDescriptor* ld );
	bool handleLinkDirect( OverlayMsg* msg, LinkDescriptor* ld );
	bool handleLinkAlive( OverlayMsg* msg, LinkDescriptor* ld );


	// link state handling -----------------------------------------------------

	/// link state information counter
	int counter;

	/// The link mapping of the node
	vector<LinkDescriptor*> links;

	/// erases a link descriptor
	void eraseDescriptor(const LinkID& link, bool communication = false);

	/// returns a link descriptor for the given id
	LinkDescriptor* getDescriptor(const LinkID& link,
			bool communication = false);

	/// returns a link descriptor for the given id
	const LinkDescriptor* getDescriptor(const LinkID& link,
			bool communication = false) const;

	/// returns a auto-link descriptor for the given node and service id
	LinkDescriptor* getAutoDescriptor(const NodeID& node, const ServiceID& service);

	/// adds a new link descriptor or uses an existing one
	LinkDescriptor* addDescriptor(const LinkID& link = LinkID::UNSPECIFIED);

	/// stabilizes link information
	void stabilizeLinks();

	/// print the currently known links
	void showLinks();

	/// compares two arbitrary links to the same node
	int compare( const LinkID& lhs, const LinkID& rhs );

	// relay route management --------------------------------------------------

	/// relay route definitions
	class relay_route {
	public:
		NodeID  node;
		LinkID  link;
		uint8_t hops;
		time_t  used;
	};
	vector<relay_route> relay_routes;

	/// stabilize relay information
	void stabilizeRelays();

	/// refreshes relay information
	void refreshRelayInformation( const OverlayMsg* message, LinkDescriptor* ld );

	/// returns a known relay link
	LinkDescriptor* getRelayLinkTo( const NodeID& remote );

	/// removes relay link information
	void removeRelayLink( const LinkID& link );

	/// removes relay node information
	void removeRelayNode( const NodeID& link );

	// internal message delivery -----------------------------------------------

	/// routes a message to its destination node
	void route( OverlayMsg* message );

	/// sends a raw message to another node, delivers it to the base overlay class
	seqnum_t send( OverlayMsg* message, const NodeID& destination );

	/// send a raw message using a link descriptor, delivers it to the base overlay class
	seqnum_t send( OverlayMsg* message, LinkDescriptor* ld,
		bool ignore_down = false );

	/// send a message using a node id using overlay routing
	/// sets necessary fields in the overlay message!
	seqnum_t send_node( OverlayMsg* message, const NodeID& remote,
		const ServiceID& service = OverlayInterface::OVERLAY_SERVICE_ID);

	/// send a message using a node id using overlay routing using a link
	/// sets necessary fields in the overlay message!
	seqnum_t send_link( OverlayMsg* message, const LinkID& link,
		bool ignore_down = false );

	// distributed hashtable handling ------------------------------------------

	DHT* dht;
	DHT* localDHT;
	int republishCounter;

	void initDHT();
	void destroyDHT();
	void stabilizeDHT();
	void dhtSend( OverlayMsg* msg, const NodeID& dest );

	// misc --------------------------------------------------------------------

	std::string debugInformation();

	/**
	 * nodes with pending joines. TODO: should be cleaned every
	 * some seconds, add timestamps to each, and check on occasion
	 */
	typedef vector<NodeID> JoiningNodes;
	JoiningNodes joiningNodes;

	void updateVisual();
};

}} // namespace ariba, overlay

#endif /*BASEOVERLAY_H_*/
