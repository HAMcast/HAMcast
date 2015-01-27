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

#ifndef SIDEPORTLISTENER_H_
#define SIDEPORTLISTENER_H_

#include <vector>
#include <map>
#include <iostream>
#include <boost/foreach.hpp>
#include "Identifiers.h"
#include "CommunicationListener.h"

using std::map;
using std::vector;

namespace ariba {

// forward declarations
class Node;
class AribaModule;
namespace overlay {
	class BaseOverlay;
}

/**
 * \addtogroup public
 * @{
 *
 * A sideport class to gather advanced information about nodes, links,
 * their endpoints and get information about all link activity on a node.
 *
 * @author Christoph Mayer <mayer@tm.uka.de>
 */
class SideportListener {

	friend class Node;
	friend class AribaModule;
	friend class overlay::BaseOverlay;

public:

	/**
	 * A default object of the SideportListener that has empty
	 * event functions and will return invalid information.
	 */
	static SideportListener DEFAULT;

	/**
	 * Constructor of the SideportListener.
	 */
	SideportListener();

	/**
	 * Virtual Desctructor for the SideportListener.
	 */
	virtual ~SideportListener();

	/**
	 * Get a descriptive string that identifies
	 * the remote endpoint for the given link.
	 *
	 *  @param link The link to query endpoint information for.
	 *  @return A descriptive endpoint information.
	 */
	string getEndpointDescription(
			const LinkID& link
			) const;

	/**
	 * Get a descriprive string that identifiers the remote node.
	 *
	 * @param node The node id to query endpoint information.
	 * @return A descriptive endpoint information.
	 */
	string getEndpointDescription(
			const NodeID& node = NodeID::UNSPECIFIED
			) const;

	/**
	 * Get the remote endpoint node id for the given string,
	 * or the local nodeid for an unspecified link.
	 *
	 * @param link The link to get the remote node.
	 * @return The nodeid of the remote end of the link
	 * 			or the local nodeid for an unspecified link.
	 */
	const NodeID& getNodeID(
			const LinkID& link = LinkID::UNSPECIFIED
			) const;

	/**
	 * Get all links that end at the specified node id.
	 * Or all links from the local node when the node id
	 * is set to unspecified.
	 *
	 * @param node The remote node to query all links or unspecified
	 * 			for all local starting links
	 * @return A vector of link ids.
	 */
	vector<LinkID> getLinkIDs(
			const NodeID& node = NodeID::UNSPECIFIED
			) const;

	/**
	 * Get html presentation of the links.
	 * @return html of links
	 */
	string getHtmlLinks();

	/**
	 * Get the neighbots in the overlay structure
	 * @return A vector of NodeIDs of the neighbors
	 */
	vector<NodeID> getOverlayNeighbors(bool deep = true);

	/**
	 * Is this node acting as a relay for us
	 *
	 * @param node The node in question
	 * @return true, if this node is relaying for us
	 */
	bool isRelayingNode(const NodeID& node);

	/**
	 * Is this node only reachable for us through a relay?
	 *
	 * @param node The node in question
	 * @return true, if we reach this node only over a relay
	 */
	bool isRelayedNode(const NodeID& node);

	/**
	 * Protocols for some layer, can be combined
	 */
	enum Protocol {
		undefined = 0x0,
		rfcomm = 0x1,
		ipv4 = 0x2,
		ipv6 = 0x3
	};

	/**
	 * Through which protocol is a node reachable.
	 *
	 * @param node The node for which to return protocol reachability
	 * @return Combination of protocols
	 */
	Protocol getReachabilityProtocol(const NodeID& node);

protected:

	/**
	 * Notification function when a link has gone up.
	 *
	 * @param lnk The corresponding link id.
	 * @param local The local node id.
	 * @param remote The remote node id.
	 * @param spovnet The SpoVNet ID.
	 */
	virtual void onLinkUp(
			const LinkID& lnk,
			const NodeID& local,
			const NodeID& remote,
			const SpoVNetID& spovnet
			);

	/**
	 * Notification function when a link has gone down.
	 *
	 * @param lnk The corresponding link id.
	 * @param local The local node id.
	 * @param remote The remote node id.
	 * @param spovnet The SpoVNet ID.
	 */
	virtual void onLinkDown(
			const LinkID& lnk,
			const NodeID& local,
			const NodeID& remote,
			const SpoVNetID& spovnet
			);

	/**
	 * Notification function when a link has changed
	 *
	 * @param lnk The corresponding link id.
	 * @param local The local node id.
	 * @param remote The remote node id.
	 * @param spovnet The SpoVNet ID.
	 */
	virtual void onLinkChanged(
			const LinkID& lnk,
			const NodeID& local,
			const NodeID& remote,
			const SpoVNetID& spovnet
			);

	/**
	 * Notification function when a link has failed
	 *
	 * @param lnk The corresponding link id.
	 * @param local The local node id.
	 * @param remote The remote node id.
	 * @param spovnet The SpoVNet ID.
	 */
	virtual void onLinkFail(
			const LinkID& lnk,
			const NodeID& local,
			const NodeID& remote,
			const SpoVNetID& spovnet
			);

private:

	/**
	 * Configure the sideport with the correct base overlay.
	 *
	 * @param _overlay The BaseOverlay where to attach the sideport.
	 */
	void configure(
			overlay::BaseOverlay* _overlay
			);

	/**
	 * The configured BaseOverlay where
	 * the sideport is attached to.
	 */
	overlay::BaseOverlay* overlay;

};

} // namespace ariba

/** @} */

#endif // SIDEPORTLISTENER_H_
