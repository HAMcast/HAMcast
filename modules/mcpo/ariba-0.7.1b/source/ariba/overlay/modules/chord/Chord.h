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

#ifndef CHORD_H_
#define CHORD_H_

#include "ariba/utility/system/Timer.h"
#include "ariba/utility/logging/Logging.h"
#include "ariba/communication/EndpointDescriptor.h"
#include "../OverlayInterface.h"
#include <vector>

class chord_routing_table;

namespace ariba {
namespace overlay {

class OverlayMsg;

using ariba::communication::EndpointDescriptor;
using ariba::utility::Timer;

using namespace std;

/**
 * This class implements a structured overlay inspired by chord.
 * It differs to the original form of chord in the following manner:
 *
 * (1) The graph is bidirectional
 * (2) Stabilization is done in a reactive manner
 *
 * It therefore can be considered as a kind of Chorded-Kademlia :)
 *
 * The resulting overlay graph has a diameter of O(log N).
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class Chord : public OverlayInterface, protected Timer {
	use_logging_h( Chord );
private:
	chord_routing_table* table;
	int orphan_removal_counter;
	int stabilize_counter;
	int stabilize_finger;
	vector<LinkID> bootstrapLinks;
	vector<NodeID> pending;
	vector<NodeID> discovery;
	int discovery_count;

	// helper: sets up a link using the "base overlay"
	LinkID setup( const EndpointDescriptor& endp,
		const NodeID& node = NodeID::UNSPECIFIED );

	// helper: sends a message using the "base overlay"
	seqnum_t send( OverlayMsg* msg, const LinkID& link );

	// stabilization: sends a discovery message to the specified neighborhood
	void send_discovery_to( const NodeID& destination, int ttl = 3 );

	void discover_neighbors( const LinkID& lnk );

	void showLinks();

public:
	Chord(BaseOverlay& _baseoverlay, const NodeID& _nodeid,
			OverlayStructureEvents* _eventsReceiver, const OverlayParameterSet& param);
	virtual ~Chord();

	/// @see OverlayInterface.h
	virtual const LinkID& getNextLinkId( const NodeID& id ) const;

	/// @see OverlayInterface.h
	virtual void createOverlay();

	/// @see OverlayInterface.h
	virtual void deleteOverlay();

	/// @see OverlayInterface.h
	virtual void joinOverlay(
		const EndpointDescriptor& boot = EndpointDescriptor::UNSPECIFIED()
	);

	/// @see OverlayInterface.h
	virtual void leaveOverlay();

	/// @see OverlayInterface.h
	virtual const EndpointDescriptor& resolveNode( const NodeID& node );

	/// @see OverlayInterface.h
	virtual bool isClosestNodeTo( const NodeID& node );

	/// @see OverlayInterface.h
	virtual NodeList getKnownNodes(bool deep = true) const;

	/// @see CommunicationListener.h or @see OverlayInterface.h
	virtual void onLinkUp(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener.h or @see OverlayInterface.h
	virtual void onLinkDown(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener.h or @see OverlayInterface.h
	virtual void onMessage(const DataMessage& msg, const NodeID& remote,
			const LinkID& lnk = LinkID::UNSPECIFIED);

	/// @see OverlayInterface.h
	virtual std::string debugInformation() const;

	/// @see Timer.h
	virtual void eventFunction();

};

}}

#endif /* CHORD_H_ */
