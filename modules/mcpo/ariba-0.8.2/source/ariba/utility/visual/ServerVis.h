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

#ifndef SERVERVIS_H__
#define SERVERVIS_H__

#include <string>
#include <sstream>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/types/NodeID.h"

using ariba::utility::NodeID;
using std::ostringstream;
using std::string;
using boost::asio::ip::tcp;

namespace ariba {
namespace utility {

class ServerVis {
	use_logging_h(ServerVis);
public:
	void configure(string ip, unsigned int port, unsigned int _color = 0);
	static unsigned int nodecolor;

	typedef enum _NETWORK_ID {
		NETWORK_ID_BASE_COMMUNICATION 	= 1,
		NETWORK_ID_BASE_OVERLAY 		= 2,
		NETWORK_ID_EONSON 				= 3,
		NETWORK_ID_MCPO 				= 4,
		NETWORK_ID_CLIO 				= 5,
		NETWORK_ID_VIDEOSTREAM 			= 6,
		NETWORK_ID_GAME 				= 7,
		NETWORK_ID_SECURITY 			= 8,
	} NETWORK_ID;

	// get the name of the network
	string getNetworkName(NETWORK_ID network) const;

	/// Create a node
	virtual void visCreate (NETWORK_ID network, NodeID& node, string nodename, string info) = 0;

	/// Delete a node
	virtual void visShutdown (NETWORK_ID network, NodeID& node, string info) = 0;

	/// Connect two nodes
	virtual void visConnect (NETWORK_ID network, NodeID& srcnode, NodeID& destnode, string info) = 0;

	/// Disconnect two nodes
	virtual void visDisconnect (NETWORK_ID network, NodeID& srcnode, NodeID& destnode, string info) = 0;

	/// Node colors
	typedef enum _NODE_COLORS {
		NODE_COLORS_GREY,
		NODE_COLORS_GREEN,
		NODE_COLORS_RED,
	} NODE_COLORS;

	/// Change the node color
	virtual void visChangeNodeColor (NETWORK_ID network, NodeID& node, unsigned char r, unsigned char g, unsigned char b) = 0;

	/// Change the node color
	virtual void visChangeNodeColor (NETWORK_ID network, NodeID& node, NODE_COLORS color) = 0;

	/// Change the link color
	virtual void visChangeLinkColor (NETWORK_ID network, NodeID& srcnode, NodeID& destnode, unsigned char r, unsigned char g, unsigned char b) = 0;

	/// Change the link color
	virtual void visChangeLinkColor (NETWORK_ID network, NodeID& srcnode, NodeID& destnode, NODE_COLORS color) = 0;

	/// Show the label of the node.
	virtual void visShowNodeLabel (NETWORK_ID network, NodeID& node, string label) = 0;

protected:
	ServerVis();
	virtual ~ServerVis();
	void sendSocket( const string& msg );

private:
	boost::asio::io_service io_service;
	tcp::socket socket;
	volatile bool socketOpened;
};

}} // namespace ariba, common

#endif // SERVERVIS_H__
