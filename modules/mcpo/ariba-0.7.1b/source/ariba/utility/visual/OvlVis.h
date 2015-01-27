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

#ifndef OVLVIS_H__
#define OVLVIS_H__

#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <boost/utility.hpp>
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/misc/Helper.h"
#include "ariba/utility/misc/KeyMapping.hpp"
#include "ariba/utility/visual/ServerVis.h"

using std::string;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::ostringstream;
using ariba::utility::KeyMapping;
using ariba::utility::Timer;

namespace ariba {
namespace utility {

class OvlVis : public ServerVis, private boost::noncopyable {
	use_logging_h(OvlVis);
public:
	static OvlVis& instance() { static OvlVis the_inst; return the_inst; }

	//****************************************************************
	// Node creation, node connections, status, node deletion, ...
	//****************************************************************

	/**
	 * Create a node in the network that is initially unconnected.
	 */
	void visCreate (
			NETWORK_ID network,
			NodeID& node,
			string nodename,
			string info
	);

	/**
	 * Change the status of a node -> enable/disable a node.
	 */
	void visChangeStatus(
			NETWORK_ID network,
			NodeID& node,
			bool enable,
			string info
	);

	/**
	 * Connect two nodes using a link.
	 */
	void visConnect (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			string info
	);

	/**
	 * Disconnect the link between two nodes.
	 */
	void visDisconnect (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			string info
	);

	/**
	 * Indicate that the connection procedure
	 * between two nodes failed.
	 */
	void visFailedConnect (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			string info
	);

	/**
	 * Delete a node from the network.
	 */
	void visShutdown (
			NETWORK_ID network,
			NodeID& node,
			string info
	);

	//****************************************************************
	// Node manipulation: change color, change icon
	//****************************************************************

	/**
	 * Change the color of the node.
	 */
	void visChangeNodeColor (
			NETWORK_ID network,
			NodeID& node,
			unsigned char r,
			unsigned char g,
			unsigned char b
	);

	/**
	 * Change the color of the node.
	 */
	void visChangeNodeColor (
			NETWORK_ID network,
			NodeID& node,
			NODE_COLORS color
	);

	/**
	 * The available icons for changing the
	 * icon of a node, showing an icon besides
	 * a node and showing an icon at a link.
	 */
	typedef enum _ICON_ID {
		ICON_ID_DEFAULT_NODE 	= 0,
		ICON_ID_PC 				= 1,
		ICON_ID_PC_WORLD 		= 2,
		ICON_ID_FAILURE 		= 3,
		ICON_ID_RED_CROSS 		= 4,
		ICON_ID_CHARACTER_A 	= 5,
		ICON_ID_CHARACTER_W 	= 6,
		ICON_ID_CAMERA          = 7,
	} ICON_ID;

	/**
	 * Change the icon of a node.
	 */
	void visChangeNodeIcon (
			NETWORK_ID network,
			NodeID& node,
			ICON_ID icon
	);

	/**
	 * Show the label of the node.
	 */
	void visShowNodeLabel (
			NETWORK_ID network,
			NodeID& node,
			string label
	);

	/**
	 * Delete the label of the node.
	 */
	void visDeleteNodeLabel (
			NETWORK_ID network,
			NodeID& node
	);

	/**
	 * Show a bubble at the node.
	 */
	void visShowNodeBubble (
			NETWORK_ID network,
			NodeID& node,
			string label
	);

	/**
	 * Delete a bubble at the node.
	 */
	void visDeleteNodeBubble (
			NETWORK_ID network,
			NodeID& node
	);

	/**
	 * Show an icon besides the node.
	 */
	void visShowShiftedNodeIcon (
			NETWORK_ID network,
			NodeID& node,
			ICON_ID iconID,
			unsigned int timeout = 0
	);

	/**
	 * Delete an icon besides the node
	 */
	void visDeleteShiftedNodeIcon (
			NETWORK_ID network,
			NodeID& node
	);

	//****************************************************************
	// Link manipulation: change width, color, show bubbles, icons, ...
	//****************************************************************

	/**
	 * Change the link width
	 */
	void visChangeLinkWidth (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			unsigned int width
	);

	/**
	 * Change the link color
	 */
	void visChangeLinkColor (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			unsigned char r,
			unsigned char g,
			unsigned char b
	);

	/**
	 * Change the link color
	 */
	void visChangeLinkColor (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			NODE_COLORS color
	);

	/**
	 * Show a link label
	 */
	void visShowLinkLabel (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			string label
	);

	/**
	 * Delete a link label
	 */
	void visDeleteLinkLabel (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode
	);

	/**
	 * Show an icon at the link
	 */
	void visShowOnLinkIcon (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			ICON_ID iconID
	);

	/**
	 * Delete an icon at the link
	 */
	void visDeleteOnLinkIcon (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode
	);

	/**
	 * Show a bubble besides the link
	 */
	void visShowLinkBubble (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode,
			string label
	);

	/**
	 * Delete a bubble besides the link
	 */
	void visDeleteLinkBubble (
			NETWORK_ID network,
			NodeID& srcnode,
			NodeID& destnode
	);

	//****************************************************************
	// Send message between two nodes
	//****************************************************************

	/**
	 * Animate the message sending between two nodes
	 */
	void visSendMessage (
			NETWORK_ID network,
			NodeID& startnode,
			NodeID& endnode
	);

	//*******************************************************
	//*
	void visCLIOInitMeasurement (
			NETWORK_ID network,
			unsigned long edgekey,
			NodeID& srcnode,
			NodeID& destnode,
			string info
	);

	void visCLIOEndMeasurement (
			NETWORK_ID network,
			unsigned long edgekey,
			NodeID& srcnode,
			NodeID& destnode,
			string info,
			string value,
			string unit
	);
	//*
	//*******************************************************

protected:
	OvlVis();
	virtual ~OvlVis();

private:
	void sendMessage( const string msg, NETWORK_ID nid );

	typedef pair<NodeID, NodeID> NodePair;
	typedef KeyMapping<NodePair> NetworkLinks;
	typedef KeyMapping<NodePair> LinkBubbles;
	typedef KeyMapping<NodeID>   NodeBubbles;
	typedef KeyMapping<NodeID>   ShiftedNodeIcons;
	typedef KeyMapping<NodePair> OnLinkIcons;

	NetworkLinks 		networkLinks;
	LinkBubbles 		linkBubbles;
	NodeBubbles 		nodeBubbles;
	ShiftedNodeIcons 	shiftedNodeIcons;
	OnLinkIcons 		onLinkIcons;

	class TimedoutIcon : public Timer {
	private:
		NETWORK_ID network;
		NodeID node;
		unsigned int timeout;
	public:
		TimedoutIcon(NETWORK_ID _network, NodeID _node, unsigned int _timeout) :
			network(_network), node(_node), timeout(_timeout) {
		}

		virtual ~TimedoutIcon(){
			Timer::stop();
		}

		void startIcon(){
			Timer::setInterval( timeout, true );
			Timer::start();
		}

	protected:
		virtual void eventFunction(){
			OvlVis::instance().visDeleteShiftedNodeIcon( network, node );
			delete this;
		}
	};

};

}} // namespace ariba, common

#endif // OVLVIS_H__
