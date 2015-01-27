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

#include "DddVis.h"

namespace ariba {
namespace utility {

const string DddVis::del = ":";
use_logging_cpp(DddVis);

DddVis::DddVis() : commandid(0) {
	srand(time(NULL));
}

DddVis::~DddVis(){
}

void DddVis::sendMessage( const string msg, NETWORK_ID nid ) {
	sendSocket( msg + "\n" );
}

long DddVis::getCommandID() {
	return ++commandid;
}

long DddVis::getTimestamp() {
	return 0; //time(NULL);
}

unsigned int DddVis::getNodeNumber(const NodeID& node){
	NodeSet::iterator i = nodeSet.find(node);

	if(i == nodeSet.end()){
		unsigned int number = node.get(MAX_KEYLENGTH-16, 16);
		nodeSet.insert(make_pair(node, number));
		return number;
	} else {
		return i->second;
	}
}

void DddVis::visCreate(
		NETWORK_ID network,
		NodeID& node,
		string nodename,
		string info
){
	//
	// create layer first if not already done
	//

	if(layerSet.find(network) == layerSet.end()){

		//
		// create layer
		//

		{
			ostringstream out;
			out		<< CREATE_LAYER_TYPE		<< del
					<< getCommandID()			<< del
					<< getTimestamp()			<< del
					<< getNetworkName(network)	<< del
					<< "null"					<< del
					<< 0						<< del;

			sendMessage( out.str(), network );
			layerSet.insert(network);
		}

		//
		// set layer layout
		//

		{
			LayoutType layout = FORCE_LAYOUT;
			LayoutOrderStrategie order = ORDER_RANDOMLY;

			switch(network){
			case NETWORK_ID_BASE_COMMUNICATION:
				layout = FORCE_LAYOUT;
				order = ORDER_RANDOMLY;
				break;
			case NETWORK_ID_BASE_OVERLAY:
				layout = CIRCULAR_LAYOUT;
				order = ORDER_BY_ID;
				break;
			case NETWORK_ID_MCPO:
				layout = FORCE_LAYOUT;
				order = ORDER_RANDOMLY;
				break;
			default:
				break;
			}

			ostringstream out;
			out		<< SET_CLUSTER_LAYOUT_TYPE	<< del
					<< getCommandID()			<< del
					<< getTimestamp()			<< del
					<< getNetworkName(network)	<< del
					<< 0						<< del
					<< layout					<< del
					<< order					<< del;

			sendMessage( out.str(), network );
		}

	} // if(layerSet.find(network) == layerSet.end())

	//
	// create node
	//

	ostringstream out;
	out 	<< CREATE_NODE_TYPE 		<< del
			<< getCommandID() 			<< del
			<< getTimestamp() 			<< del
			<< getNetworkName(network) 	<< del
			<< 0						<< del
			<< getNodeNumber(node) 		<< del
			<< 0x00000000				<< del
			<< "null" 					<< del;

	sendMessage( out.str(), network );

	//
	// set node color, if any given
	//

	if(this->nodecolor != 0){
		this->visChangeNodeColor(network, node,
				(nodecolor & 0xFF0000) >> 16, (nodecolor & 0x00FF00) >> 8, nodecolor & 0x0000FF );
	}
}

void DddVis::visConnect(
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

	out	<< CREATE_EDGE_TYPE 			<< del
			<< getCommandID() 			<< del
			<< getTimestamp() 			<< del
			<< getNetworkName(network) 	<< del
			<< 0						<< del
			<< getNodeNumber(srcnode) 	<< del
			<< getNodeNumber(destnode) 	<< del
			<< edgekey 					<< del
			<< 0x00000000 				<< del
			<< "null" 					<< del;

	sendMessage( out.str(), network );
}

void DddVis::visDisconnect(
		NETWORK_ID network,
		NodeID& srcnode,
		NodeID& destnode,
		string info
){
	if( !networkLinks.exists(network, NodePair(srcnode, destnode)) ) return;

	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );
	networkLinks.remove( network, NodePair(srcnode, destnode) );

	ostringstream out;
	out	<< REMOVE_EDGE_TYPE 		<< del
		<< getCommandID() 			<< del
		<< getTimestamp() 			<< del
		<< getNetworkName(network)	<< del
		<< edgekey 					<< del;

	sendMessage( out.str(), network );
}

void DddVis::visShutdown(
		NETWORK_ID network,
		NodeID& node,
		string info
){
	ostringstream out;

	out	<< REMOVE_NODE_TYPE 		<< del
		<< getCommandID() 			<< del
		<< getTimestamp() 			<< del
		<< getNetworkName(network) 	<< del
		<< getNodeNumber(node) 		<< del;

	sendMessage( out.str(), network );
}

void DddVis::visChangeNodeColor (
		NETWORK_ID network,
		NodeID& node,
		unsigned char r,
		unsigned char g,
		unsigned char b
){
	ostringstream out;
	out	<< SET_NODE_COLOR_TYPE 		<< del
		<< getCommandID() 			<< del
		<< getTimestamp() 			<< del
		<< getNetworkName(network) 	<< del
		<< getNodeNumber(node) 		<< del
		<< makeColor(r, g, b) 		<< del;

	sendMessage( out.str(), network );
}

int DddVis::makeColor(unsigned char r, unsigned char g, unsigned char b){
	return ((r<<16)+(g<<8)+b);
}

void DddVis::visChangeNodeColor (
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

void DddVis::visChangeLinkColor (
		NETWORK_ID network,
		NodeID& srcnode,
		NodeID& destnode,
		unsigned char r,
		unsigned char g,
		unsigned char b
){
	ostringstream out;
	unsigned long edgekey = networkLinks.get( network, NodePair(srcnode, destnode) );

	out	<< SET_EDGE_COLOR_TYPE 		<< del
			<< getCommandID() 			<< del
			<< getTimestamp() 			<< del
			<< getNetworkName(network) 	<< del
			<< edgekey 					<< del
			<< makeColor(r, g, b) 		<< del;

	sendMessage( out.str(), network );
}

void DddVis::visChangeLinkColor (
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

void DddVis::visShowNodeLabel (
			NETWORK_ID network,
			NodeID& node,
			string label
){
	ostringstream out;
	out	<< SET_NODE_INFO_TYPE 		<< del
		<< getCommandID() 			<< del
		<< getTimestamp() 			<< del
		<< getNetworkName(network) 	<< del
		<< getNodeNumber(node) 		<< del
		<< label << del;

	sendMessage( out.str(), network );
}

}} // namespace ariba, common
