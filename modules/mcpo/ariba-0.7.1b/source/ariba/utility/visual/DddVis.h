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

#ifndef DDDVIS_H__
#define DDDVIS_H__

#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <boost/utility.hpp>
#include "ariba/utility/types/NodeID.h"
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/misc/Helper.h"
#include "ariba/utility/misc/KeyMapping.hpp"
#include "ariba/utility/configuration/Configuration.h"
#include "ariba/utility/visual/ServerVis.h"

using std::set;
using std::string;
using std::map;
using std::pair;
using std::make_pair;
using std::cout;
using std::ostringstream;
using ariba::utility::NodeID;
using ariba::utility::Configuration;
using ariba::utility::KeyMapping;
using ariba::utility::Timer;

namespace ariba {
namespace utility {

class DddVis : public ServerVis, private boost::noncopyable {
	use_logging_h(DddVis);
public:
	static DddVis& instance() { static DddVis the_inst; return the_inst; }

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
	 * Show the label of the node
	 */
	void visShowNodeLabel (
			NETWORK_ID network,
			NodeID& node,
			string label
	);

protected:
	DddVis();
	virtual ~DddVis();

private:

	typedef enum _CommandType {
		CREATE_LAYER_TYPE 			= 0,
		CREATE_CLUSTER_TYPE 		= 1,
		CREATE_NODE_TYPE 			= 2,
		CREATE_EDGE_TYPE 			= 3,
		REMOVE_LAYER_TYPE 			= 4,
		REMOVE_CLUSTER_TYPE 		= 5,
		REMOVE_NODE_TYPE 			= 6,
		REMOVE_EDGE_TYPE 			= 7,
		SET_CLUSTER_LAYOUT_TYPE 	= 8,
		SET_NODE_COLOR_TYPE 		= 9,
		SET_EDGE_COLOR_TYPE 		= 10,
		SET_NODE_INFO_TYPE 			= 11,
		SET_EDGE_INFO_TYPE 			= 12,
		SET_LAYOUT_LEADER_TYPE 		= 13,
	} CommandType;

	typedef enum _LayoutType {
		CIRCULAR_LAYOUT				= 0,
		FORCE_LAYOUT				= 1,
		LEADER_LAYOUT				= 2,
		RANDOM_LAYOUT				= 3,
	} LayoutType;

	typedef enum _LayoutOrderStrategie {
		ORDER_BY_ID					= 0,
		ORDER_RANDOMLY				= 1,
	} LayoutOrderStrategie;

	long getCommandID();
	long getTimestamp();
	int makeColor(unsigned char r, unsigned char g, unsigned char b);
	void sendMessage( const string msg, NETWORK_ID nid );
	unsigned int getNodeNumber(const NodeID& node);

	unsigned long commandid;
	static const string del;

	typedef set<ServerVis::NETWORK_ID> LayerSet;
	LayerSet layerSet;

	typedef map<NodeID,unsigned int> NodeSet;
	NodeSet nodeSet;

	typedef pair<NodeID, NodeID> NodePair;
	typedef KeyMapping<NodePair> NetworkLinks;
	NetworkLinks networkLinks;
};

}} // namespace ariba, common

#endif // DDDVIS_H__
