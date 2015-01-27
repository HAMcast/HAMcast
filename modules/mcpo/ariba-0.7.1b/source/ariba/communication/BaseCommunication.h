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

#ifndef BASECOMMUNICATION_H_
#define BASECOMMUNICATION_H_

// boost & std includes
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

// utilities
#include "ariba/utility/types.h"
#include "ariba/utility/messages.h"
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/misc/Demultiplexer.hpp"
#include "ariba/utility/system/SystemEventListener.h"

// new transport and addressing
#include "ariba/utility/addressing/addressing.hpp"
#include "ariba/utility/transport/transport.hpp"

// communication
#include "ariba/communication/CommunicationEvents.h"
#include "ariba/communication/EndpointDescriptor.h"
#include "ariba/communication/messages/AribaBaseMsg.h"

// network changes
#include "ariba/communication/networkinfo/NetworkChangeInterface.h"
#include "ariba/communication/networkinfo/NetworkChangeDetection.h"
#include "ariba/communication/networkinfo/NetworkInformation.h"

// disabled
//#ifndef UNDERLAY_OMNET
//  #include "ariba/communication/modules/transport/tcp/TCPTransport.h"
//  #include "ariba/communication/modules/network/ip/IPv4NetworkProtocol.h"
//  using ariba::communication::IPv4NetworkProtocol;
//  using ariba::communication::TCPTransport;
//#endif

namespace ariba {
  class SideportListener;
}

namespace ariba {
namespace communication {

using namespace std;
using namespace ariba::addressing;
using namespace ariba::transport;
using namespace ariba::utility;

// use base ariba types (clarifies multiple definitions)
using ariba::utility::Message;
using ariba::utility::seqnum_t;

/**
 * This class implements the Ariba Base Communication<br />
 *
 * Its primary task is to provide an abstraction to existing
 * protocols and addressing schemes.
 *
 * @author Sebastian Mies, Christoph Mayer
 */
class BaseCommunication:
	public NetworkChangeInterface,
	public SystemEventListener,
	public transport_listener {

	use_logging_h(BaseCommunication);
	friend class ariba::SideportListener;

public:
	/// Default ctor that just creates an non-functional base communication
	BaseCommunication();

	/// Default dtor that does nothing
	virtual ~BaseCommunication();

	/// Startup the base communication, start modules etc.
	void start();

	/// Stops the base communication, stop modules etc.
	void stop();

	/// Sets the endpoints
	void setEndpoints( string& endpoints );

	/// Check whether the base communication has been started up
	bool isStarted();

	/// Establishes a link to another end-point.
	const LinkID establishLink(const EndpointDescriptor& descriptor,
		const LinkID& linkid = LinkID::UNSPECIFIED, const QoSParameterSet& qos =
				QoSParameterSet::DEFAULT, const SecurityParameterSet& sec =
				SecurityParameterSet::DEFAULT);

	/// Drops a link
	void dropLink(const LinkID link);

	/**
	 * Sends a message though an existing link to an end-point.
	 *
	 * @param lid The link identifier
	 * @param message The message to be sent
	 * @return A sequence number for this message
	 */
	seqnum_t sendMessage(const LinkID lid, const Message* message);

	/**
	 * Returns the end-point descriptor
	 *
	 * @param link the link id of the requested end-point
	 * @return The end-point descriptor of the link's end-point
	 */
	const EndpointDescriptor& getEndpointDescriptor(const LinkID link =
			LinkID::UNSPECIFIED) const;

	/**
	 * Get local links to the given endpoint of all local link
	 * using the default parameter EndpointDescriptor::UNSPECIFIED
	 * @param ep The remote endpoint to get all links to.
	 * @return List of LinkID
	 */
	LinkIDs getLocalLinks(const address_v* addr) const;

	/**
	 * Registers a receiver.
	 *
	 * @param _receiver The receiving side
	 */
	void registerMessageReceiver(MessageReceiver* receiver) {
		messageReceiver = receiver;
	}

	/**
	 * Unregister a receiver.
	 *
	 * @param _receiver The receiving side
	 */
	void unregisterMessageReceiver(MessageReceiver* receiver) {
		messageReceiver = NULL;
	}

	void registerEventListener(CommunicationEvents* _events);

	void unregisterEventListener(CommunicationEvents* _events);

	/// called when a system event is emitted by system queue
	virtual void handleSystemEvent(const SystemEvent& event);

	/// called when a message is received form transport_peer
	virtual void receive_message(transport_protocol* transport,
		const address_vf local, const address_vf remote, const uint8_t* data,
		uint32_t size);

protected:

	/// handle received message from a transport module
	void receiveMessage(const Message* message,
		const address_v* local, const address_v* remote );

	/// called when a network interface change happens
	virtual void onNetworkChange(
		const NetworkChangeInterface::NetworkChangeInfo& info);

private:
	/**
	 * A link descriptor consisting of the end-point descriptor and currently
	 * used underlay address.
	 */
	class LinkDescriptor {
	public:

		/// default constructor
		LinkDescriptor() :
			localLink(LinkID::UNSPECIFIED), localLocator(NULL),
			remoteLink(LinkID::UNSPECIFIED), remoteLocator(NULL),
			up(false) {
		}

		~LinkDescriptor() {
			if (localLocator!=NULL)  delete localLocator;
			if (remoteLocator!=NULL) delete remoteLocator;
		}

		bool isUnspecified() const {
			return (this == &UNSPECIFIED());
		}

		static LinkDescriptor& UNSPECIFIED(){
			static LinkDescriptor* unspec = NULL;
			if(unspec == NULL) unspec = new LinkDescriptor();
			return *unspec;
		}

		bool unspecified;

		/// link identifiers
		LinkID localLink;
		const address_v* localLocator;

		/// used underlay addresses for the link
		LinkID remoteLink;
		const address_v* remoteLocator;

		/// the remote end-point descriptor
		EndpointDescriptor remoteEndpoint;

		/// flag, whether this link is up
		bool up;
	};

	/// Link management: list of links
	typedef vector<LinkDescriptor*> LinkSet;

	/// Link management: the set of currently managed links
	LinkSet linkSet;

	/// Link management: add a link
	void addLink( LinkDescriptor* link );

	/// Link management: remove a link
	void removeLink(const LinkID& localLink);

	/// Link management: get link information using the local link
	LinkDescriptor& queryLocalLink(const LinkID& localLink) const;

	/// Link management: get link information using the remote link
	LinkDescriptor& queryRemoteLink(const LinkID& remoteLink) const;

	/// The local end-point descriptor
	EndpointDescriptor localDescriptor;

#ifndef UNDERLAY_OMNET
	/// network change detector
	NetworkChangeDetection networkMonitor;
#endif

	/// list of all remote addresses of links to end-points
	class endpoint_reference {
	public:
		int count; ///< the number of open links to this end-point
		const address_v* endpoint; ///< the end-point itself
	};
	vector<endpoint_reference> remote_endpoints;

	/// adds an end-point to the list
	void add_endpoint( const address_v* endpoint );

	/// removes an end-point from the list
	void remove_endpoint( const address_v* endpoint );

	/// event listener
	typedef set<CommunicationEvents*> EventListenerSet;
	EventListenerSet eventListener;

	/// sequence numbers
	seqnum_t currentSeqnum;

	/// transport peer
	transport_peer* transport;

	/// the base overlay message receiver
	MessageReceiver* messageReceiver;

	/// convenience: send message to peer
	void send( Message* message, const EndpointDescriptor& endpoint );
	void send( Message* message, const LinkDescriptor& descriptor );

	/// state of the base communication
	bool started;

};

}} // namespace ariba, communication

#endif /* BASECOMMUNICATION_H_ */
