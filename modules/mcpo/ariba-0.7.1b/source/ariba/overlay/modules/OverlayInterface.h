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

#ifndef __OVERLAY_INTERFACE_H
#define __OVERLAY_INTERFACE_H

#include "ariba/CommunicationListener.h"
#include "ariba/communication/EndpointDescriptor.h"
#include "ariba/overlay/modules/OverlayStructureEvents.h"
#include "ariba/utility/types/NodeID.h"
#include "ariba/utility/types/ServiceID.h"
#include "ariba/utility/types/OverlayParameterSet.h"

using ariba::CommunicationListener;
using ariba::communication::EndpointDescriptor;
using ariba::overlay::OverlayStructureEvents;
using ariba::utility::NodeID;
using ariba::utility::ServiceID;
using ariba::utility::OverlayParameterSet;

namespace ariba {
namespace overlay {

class BaseOverlay;

/**
 * This class declares an interface for an structured overlay.
 */
class OverlayInterface: public CommunicationListener {
	friend class BaseOverlay;

public:
	/**
	 * A node list
	 */
	typedef vector<NodeID> NodeList;

	/**
	 * Constructs a new overlay.
	 */
	OverlayInterface( BaseOverlay& _baseoverlay, const NodeID& _nodeid,
		OverlayStructureEvents* _eventsReceiver, OverlayParameterSet _parameters
	);

	/**
	 * Destructs the overlay.
	 */
	virtual ~OverlayInterface();

	/**
	 * Creates the overlay.
	 */
	virtual void createOverlay() = 0;

	/**
	 * Destroys the overlay.
	 */
	virtual void deleteOverlay() = 0;

	/**
	 * Joins the overlay. Starts integration and stabilization of the overlay
	 * Node.
	 *
	 * @param bootstrap The bootstrap end-point descriptor or the default
	 *    end-point, if this node is the initiator
	 */
	virtual void joinOverlay(
		const EndpointDescriptor& bootstrap = EndpointDescriptor::UNSPECIFIED()) = 0;

	/**
	 * Leaves the overlay gracefully.
	 */
	virtual void leaveOverlay() = 0;

	/**
	 * Resolves a overlay neighbor.
	 *
	 * @param node The node to resolve
	 * @return Endpoint descriptor of local neighbor or UNSPECIFIED
	 */
	virtual const EndpointDescriptor& resolveNode(const NodeID& node) = 0;


	/**
	 * Returns true if this is the closest node to the given node
	 * identifier.
	 *
	 * @param node The node identifier to compare with
	 * @return True if this is the closest node to the given node identifier
	 */
	virtual bool isClosestNodeTo( const NodeID& node ) = 0;

	/**
	 * Returns the nodes known to this overlay.
	 *
	 * Usually this are the direct neighbors in the overlay structure.
	 * For instance, Chord would return his predecessor, successor and finger
	 * nodes. On the other hand OneHop would likely return all participating
	 * nodes in the overlay.
	 *
	 * @return The list of all known nodes
	 */
	virtual NodeList getKnownNodes(bool deep = true) const = 0;

	/**
	 * Returns the link id of the next hop a route message would take.
	 *
	 * @param id The destination node id
	 * @return The link id of the next hop
	 */
	virtual const LinkID& getNextLinkId( const NodeID& id ) const = 0;

	//--- functions from CommunicationListener that we _can_ use as overlay ---

	/// @see CommunicationListener
	virtual void onLinkUp(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener
	virtual void onLinkDown(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener
	virtual void onLinkChanged(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener
	virtual void onLinkFail(const LinkID& lnk, const NodeID& remote);

	/// @see CommunicationListener
	virtual void onLinkQoSChanged(const LinkID& lnk, const NodeID& remote,
			const LinkProperties& prop);

	/// @see CommunicationListener
	virtual bool onLinkRequest(const NodeID& remote, const DataMessage& msg);

	/// @see CommunicationListener
	virtual void onMessage(const DataMessage& msg, const NodeID& remote,
			const LinkID& lnk = LinkID::UNSPECIFIED);

	const OverlayParameterSet& getParameters() const;

	virtual std::string debugInformation() const;

protected:
	/// Reference to an active base overlay
	BaseOverlay& baseoverlay;

	/// The node identifier to use with this overlay
	const NodeID& nodeid;

	/// The listener used to inform about overlay structure changes
	OverlayStructureEvents* eventsReceiver;

	/// The parameters of the overlay structure
	OverlayParameterSet parameters;

	/// The service identifer of this overlay
	static ServiceID OVERLAY_SERVICE_ID;
};

}} // namespace ariba, overlay

#endif // __OVERLAY_INTERFACE_H
