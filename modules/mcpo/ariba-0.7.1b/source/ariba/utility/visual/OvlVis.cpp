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

#include "OvlVis.h"

namespace ariba {
namespace utility {

use_logging_cpp(OvlVis);

OvlVis::OvlVis() {
}

OvlVis::~OvlVis(){
}

void OvlVis::sendMessage( const string msg, NETWORK_ID nid ) {

	sendSocket( msg );

// 	{
// 		// auto layout
// 		ostringstream out;
//
// 		out 	<< "VisMsgOptimizeView" << ";"
// 			<< Helper::ultos(nid)     << ";" // network id is ignored in the message renderer, just renders the current viewed network
// 			<< "0"                  << std::endl;
//
// 		sendSocket( out.str() );
// 	}

// 	{	// auto zoom
// 		ostringstream out;
//
// 		out 	<< "VisMsgOptimizeView" << ";"
// 			<< Helper::ultos(nid)     << ";" // network id is ignored in the message renderer, just renders the current viewed network
// 			<< "1"                  << std::endl;
//
// 		sendSocket( out.str() );
// 	}

}

//*****************************************************
//*****************************************************
//*****************************************************

void OvlVis::visCreate(
	NETWORK_ID network,
	NodeID& node,
	string nodename,
	string info
	){

	ostringstream out;

	out 	<< "VisMsgCreate"         << ";"
		<< Helper::ultos(network) << ";"
		<< node.toString()        << ";"
		<< nodename               << ";"
		<< "" /*netName*/         << ";"
		<< "" /*ip*/              << ";"
		<< "" /*port*/            << ";"
		<< "0"                    << ";"
		<< info                   << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visChangeStatus(
	NETWORK_ID network,
	NodeID& node,
	bool enable,
	string info
	){

	ostringstream out;

	out 	<< "VisMsgChangeStatus"   << ";"
		<< Helper::ultos(network) << ";"
		<< node.toString()        << ";"
		<< (enable ? "1" : "0")   << ";"
		<< info                   << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visConnect(
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	string info
	){

	// if we already have a link between the two nodes
	// we just ignore the call and leave the old link

	if( networkLinks.exists( network, NodePair(srcnode,destnode) )) return;

	ostringstream out;
	unsigned long edgekey = networkLinks.insert( network, NodePair(srcnode,destnode) );

	out 	<< "VisMsgConnect"        << ";"
		<< Helper::ultos(network) << ";"
		<< edgekey 		  << ";"
		<< srcnode.toString()     << ";"
		<< destnode.toString()    << ";"
		<< "0"                    << ";"
		<< info                   << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visDisconnect(
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	string info
	){

	if( !networkLinks.exists(network, NodePair(srcnode, destnode)) ) return;

	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );
	networkLinks.remove( network, NodePair(srcnode, destnode) );

	ostringstream out;
	out	<< "VisMsgDisconnect"     << ";"
		<< Helper::ultos(network) << ";"
		<< Helper::ultos(edgekey) << ";"
		<< info                   << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visFailedConnect(
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	string info
	){

	ostringstream out;

	out 	<< "VisMsgFailedConnect"  << ";"
		<< Helper::ultos(network) << ";"
		<< srcnode.toString()     << ";"
		<< destnode.toString()    << ";"
		<< info                   << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visShutdown(
	NETWORK_ID network,
	NodeID& node,
	string info
	){

	ostringstream out;

	out	<< "VisMsgShutdown"       << ";"
		<< Helper::ultos(network) << ";"
		<< node.toString()        << ";"
		<< info                   << std::endl;

	sendMessage( out.str(), network );
}

//*****************************************************
//*****************************************************
//*****************************************************

void OvlVis::visChangeNodeColor (
	NETWORK_ID network,
	NodeID& node,
	unsigned char r,
	unsigned char g,
	unsigned char b
	){

	ostringstream out;

	out	<< "VisMsgChangeNodeColor"       	<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< node.toString()        		<< ";"
		<< ariba::utility::Helper::ultos(r) 	<< ";"
		<< ariba::utility::Helper::ultos(g) 	<< ";"
		<< ariba::utility::Helper::ultos(b) 	<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visChangeNodeColor (
	NETWORK_ID network,
	NodeID& node,
	NODE_COLORS color
	){

	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	switch( color ) {
		case NODE_COLORS_GREY: 	r = 128; g = 128; b = 128; break;
		case NODE_COLORS_GREEN:	r = 0;   g = 200; b = 0;   break;
		case NODE_COLORS_RED:	r = 255; g = 0;   b = 0;   break;
	}

	visChangeNodeColor( network, node, r, g, b );
}

void OvlVis::visChangeNodeIcon (
                NETWORK_ID network,
                NodeID& node,
                ICON_ID icon
                ){

        ostringstream out;

        out	<< "VisMsgChangeNodeIcon"               << ";"
                << Helper::ultos(network) 		<< ";"
                << node.toString()        		<< ";"
                << Helper::ultos((unsigned int)icon)	<< std::endl;

        sendMessage( out.str(), network );
}

void OvlVis::visShowNodeLabel (
	NETWORK_ID network,
	NodeID& node,
	string label
	){

	ostringstream out;

	out	<< "VisMsgShowNodeLabel"       		<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< node.toString()        		<< ";"
		<< label				<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visDeleteNodeLabel (
	NETWORK_ID network,
	NodeID& node
	){

	ostringstream out;

	out	<< "VisMsgDeleteNodeLable"       	<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< node.toString()        		<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visShowNodeBubble (
                NETWORK_ID network,
                NodeID& node,
                string label
                ){

	unsigned long bubbleKey = nodeBubbles.insert( network, node );
        ostringstream out;

        out       << "VisMsgShowNodeBubble"     << ";"
                  << Helper::ultos(network)     << ";"
                  << Helper::ultos(bubbleKey)   << ";"
                  << node.toString()            << ";"
                  << label                      << std::endl;

        sendMessage( out.str(), network );
}


void OvlVis::visDeleteNodeBubble (
	NETWORK_ID network,
	NodeID& node
	){

	if( !nodeBubbles.exists(network, node)) return;

	unsigned long bubbleID = nodeBubbles.get( network, node );
	nodeBubbles.remove( network, node );

	ostringstream out;

	out	<< "VisMsgDeleteBubble"         << ";"
                << Helper::ultos(network) 	<< ";"
                << Helper::ultos(bubbleID)      << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visShowShiftedNodeIcon (
                NETWORK_ID network,
                NodeID& node,
                ICON_ID iconID,
		unsigned int timeout
                ){

	unsigned long iconKey = shiftedNodeIcons.insert( network, node );

        ostringstream out;

        out	<< "VisMsgShowNodeIcon"      	<< ";"
                << Helper::ultos(network)   	<< ";"
                << Helper::ultos(iconKey)       << ";"
                << node.toString()       	<< ";"
                << Helper::ultos(iconID)	<< std::endl;

        sendMessage( out.str(), network );

	if( timeout > 0 ){
		TimedoutIcon* obj = new TimedoutIcon( network, node, timeout );
		obj->startIcon();
	}
}

void OvlVis::visDeleteShiftedNodeIcon (
                NETWORK_ID network,
                NodeID& node
                ){

	if( !shiftedNodeIcons.exists(network, node) )return;

	unsigned long iconKey = shiftedNodeIcons.get( network, node );
	shiftedNodeIcons.remove( network, node );

	ostringstream out;

	out	<< "VisMsgDeleteIcon"           << ";"
                << Helper::ultos(network) 	<< ";"
                << Helper::ultos(iconKey)       << std::endl;

	sendMessage( out.str(), network );
}

//*****************************************************
//*****************************************************
//*****************************************************

void OvlVis::visChangeLinkWidth (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	unsigned int width
	){

	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );

	ostringstream out;
	out	<< "VisMsgChangeLinkWidth"       	<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< Helper::ultos(edgekey)		<< ";"
		<< Helper::ultos(width)			<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visChangeLinkColor (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	unsigned char r,
	unsigned char g,
	unsigned char b
	){

	ostringstream out;
	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );

	out	<< "VisMsgChangeLinkColor"       	<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< Helper::ultos(edgekey)			<< ";"
		<< Helper::ultos(r)			<< ";"
		<< Helper::ultos(g)			<< ";"
		<< Helper::ultos(b)			<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visChangeLinkColor (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	NODE_COLORS color
	){

	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	switch( color ) {
		case NODE_COLORS_GREY: 	r = 128; g = 128; b = 128; break;
		case NODE_COLORS_GREEN:	r = 0;   g = 200; b = 0;   break;
		case NODE_COLORS_RED:	r = 255; g = 0;   b = 0;   break;
	}

	visChangeLinkColor( network, srcnode, destnode, r, g, b );
}

void OvlVis::visShowLinkLabel (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	string label
	){

	ostringstream out;
	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );

	out	<< "VisMsgShowLinkLabel"       		<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< Helper::ultos(edgekey)			<< ";"
		<< label				<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visDeleteLinkLabel (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode
	){

	if( !networkLinks.exists(network, NodePair(srcnode, destnode))) return;

	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );
	ostringstream out;

	out	<< "VisMsgDeleteLinkLabel"       	<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< Helper::ultos(edgekey)		<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visShowOnLinkIcon (
                NETWORK_ID network,
                NodeID& srcnode,
                NodeID& destnode,
                ICON_ID iconID
                ){

	unsigned long iconKey = onLinkIcons.insert(network, NodePair(srcnode, destnode));
        ostringstream out;

        out	<< "VisMsgShowLinkIcon"      	<< ";"
                << Helper::ultos(network)   	<< ";"
                << Helper::ultos(iconKey)       << ";"
                << srcnode.toString()   	<< ";"
                << destnode.toString()   	<< ";"
                << Helper::ultos(iconID)	<< std::endl;

        sendMessage( out.str(), network );
}

void OvlVis::visDeleteOnLinkIcon (
                NETWORK_ID network,
                NodeID& srcnode,
		NodeID& destnode
                ){

	if( !onLinkIcons.exists(network, NodePair(srcnode, destnode))) return;

	unsigned long iconKey = onLinkIcons.get( network, NodePair(srcnode, destnode) );
	onLinkIcons.remove( network, NodePair(srcnode, destnode) );

	ostringstream out;

	out	<< "VisMsgDeleteIcon"           << ";"
                << Helper::ultos(network) 	<< ";"
                << Helper::ultos(iconKey)       << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visShowLinkBubble (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode,
	string label
	){

	ostringstream out;
	unsigned long bubble = linkBubbles.insert( network, NodePair(srcnode, destnode) );

	out	<< "VisMsgShowLinkBubble"	<< ";"
		<< Helper::ultos(network)	<< ";"
		<< Helper::ultos(bubble)	<< ";"
		<< srcnode.toString()   	<< ";"
		<< destnode.toString()   	<< ";"
		<< label			<< std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visDeleteLinkBubble (
	NETWORK_ID network,
	NodeID& srcnode,
	NodeID& destnode
	){

	if( !linkBubbles.exists(network, NodePair(srcnode, destnode))) return;

	ostringstream out;
	unsigned long bubble = linkBubbles.get( network, NodePair(srcnode, destnode) );
	linkBubbles.remove( network, NodePair(srcnode, destnode) );

	out	<< "VisMsgDeleteBubble"		<< ";"
		<< Helper::ultos(network)	<< ";"
		<< Helper::ultos(bubble)	<< std::endl;

	sendMessage( out.str(), network );
}

//*****************************************************
//*****************************************************
//*****************************************************

void OvlVis::visSendMessage (
	NETWORK_ID network,
	NodeID& startnode,
	NodeID& endnode
	){

	ostringstream out;

	out	<< "VisMsgSendMessage"       		<< ";"
		<< Helper::ultos(network) 		<< ";"
		<< startnode.toString()        		<< ";"
		<< endnode.toString()			<< std::endl;

	sendMessage( out.str(), network );
}

//*****************************************************
//*****************************************************
//*****************************************************

void OvlVis::visCLIOInitMeasurement(
	NETWORK_ID network,
	unsigned long edgekey,
	NodeID& srcnode,
	NodeID& destnode,
	string info
	){

	ostringstream out;

	out << "VisMsgCLIOInitMeasurement" << ";"
		<< Helper::ultos(network)      << ";"
		<< Helper::ultos(edgekey)      << ";"
		<< srcnode.toString()          << ";"
		<< destnode.toString()         << ";"
		<< info                        << std::endl;

	sendMessage( out.str(), network );
}

void OvlVis::visCLIOEndMeasurement(
	NETWORK_ID network,
	unsigned long edgekey,
	NodeID& srcnode,
	NodeID& destnode,
	string info,
	string value,
	string unit
	){

	ostringstream out;

	out << "VisMsgCLIOEndMeasurement" << ";"
		<< Helper::ultos(network)     << ";"
		<< Helper::ultos(edgekey)     << ";"
		<< srcnode.toString()         << ";"
		<< destnode.toString()        << ";"
		<< info                       << ";"
		<< value		      << ";"
		<< unit			      << std::endl;

	sendMessage( out.str(), network );
}

}} // namespace ariba, common
