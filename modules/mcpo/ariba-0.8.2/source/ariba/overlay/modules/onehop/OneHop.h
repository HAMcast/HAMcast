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

#ifndef _ONE_HOP_H
#define _ONE_HOP_H

#include <map>
#include "ariba/overlay/modules/OverlayInterface.h"
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/system/Timer.h"

using std::map;
using ariba::utility::Timer;

namespace ariba {
namespace overlay {

class OneHop : public OverlayInterface, protected Timer {
	use_logging_h( OneHop );
public:
	OneHop(BaseOverlay& _overlay, const NodeID& _nodeid,
			OverlayStructureEvents* _eventsReceiver, const OverlayParameterSet& param);

	virtual ~OneHop();

protected:

	/// @see Timer.h
	virtual void eventFunction();

	/// @see OverlayInterface.h
	virtual void createOverlay();

	/// @see OverlayInterface.h
	virtual void deleteOverlay();

	/// @see OverlayInterface.h
	virtual void joinOverlay(const EndpointDescriptor& boot =
			EndpointDescriptor::UNSPECIFIED());

	/// @see OverlayInterface.h
	virtual void leaveOverlay();

	/// @see OverlayInterface.h
	virtual const EndpointDescriptor& resolveNode(const NodeID& node);

	/// @see OverlayInterface.h
	virtual bool isClosestNodeTo( const NodeID& node );

	/// @see OverlayInterface.h
	virtual const LinkID& getNextLinkId( const NodeID& id ) const;
	
	/// @see OverlayInterface.h
	virtual const NodeID& getNextNodeId( const NodeID& id ) const;

	/// @see OverlayInterface.h
	virtual void routeMessage(const NodeID& destnode, Message* msg);

	/// @see OverlayInterface.h
	virtual void routeMessage(const NodeID& node, const LinkID& link, Message* msg);

	/// @see OverlayInterface.h
	virtual NodeList getKnownNodes(bool deep = true) const;

	/// @see CommunicationListener.h or @see OverlayInterface.h
	virtual void onLinkUp(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener.h or @see OverlayInterface.h
	virtual void onLinkDown(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener.h or @see OverlayInterface.h
	virtual void onMessage(const DataMessage& msg, const NodeID& remote,
			const LinkID& lnk = LinkID::UNSPECIFIED);

private:
	/// The other nodes in the overlay
	typedef map<const NodeID, const LinkID> OverlayNodeMapping;
	OverlayNodeMapping overlayNodes;

	/// The current state of the overlay
	typedef enum _OneHopState {
		OneHopStateInvalid = 0,
		OneHopStateCompleted = 1,
	} OneHopState;

	OneHopState state;
	vector<LinkID> bootstrapLinks;
};

}} // namespace ariba, overlay

#endif // _ONE_HOP_H
