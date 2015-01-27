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

#include "ServerVis.h"

namespace ariba {
namespace utility {

use_logging_cpp(ServerVis);
unsigned int ServerVis::nodecolor = 0;

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

string ServerVis::getNetworkName(NETWORK_ID network) const {
	switch(network){
		case NETWORK_ID_BASE_COMMUNICATION	: return "BaseCommunication";
		case NETWORK_ID_BASE_OVERLAY 		: return "BaseOverlay";
		case NETWORK_ID_EONSON 				: return "Eonson";
		case NETWORK_ID_MCPO 				: return "MCPO";
		case NETWORK_ID_CLIO 				: return "CLIO";
		case NETWORK_ID_VIDEOSTREAM 		: return "Video";
		case NETWORK_ID_GAME 				: return "Game";
		case NETWORK_ID_SECURITY 			: return "Security";
		default								: return "<undefined>";
	}
}

void ServerVis::configure(string ip, unsigned int port, unsigned int _color){
	nodecolor = _color;
	if( ip.length() == 0 ) return;

	ostringstream sport;
	sport << port;
	logging_debug( "connecting to visualization server " << ip << " on " << sport.str());

	tcp::resolver resolver(io_service);
	tcp::resolver::query query(
			ip, sport.str(),
			tcp::resolver::query::passive |
			tcp::resolver::query::address_configured |
			tcp::resolver::query::numeric_service);

	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end){
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}

	if (error){
		logging_warn( "visualization could not connect to visualization server" );
	} else {
		logging_info( "connected to visualization server on " << ip << ":" << port );
		socketOpened = true;
	}
}

ServerVis::ServerVis() : socket(io_service), socketOpened(false) {
}

ServerVis::~ServerVis(){
	socket.close();
}

void ServerVis::sendSocket(const string& msg){
	if( socket.is_open()==false || socketOpened==false ) return;
	logging_debug("sending visual command: " << msg);

	try{
		socket.send( boost::asio::buffer(msg) );
	}catch( std::exception& e ){
		logging_warn("visual connection failed");
		socket.close();
		socketOpened = false;
	}
}

}} // namespace ariba, common
