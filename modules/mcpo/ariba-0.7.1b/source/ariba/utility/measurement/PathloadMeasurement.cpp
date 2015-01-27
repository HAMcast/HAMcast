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

#include "PathloadMeasurement.h"

namespace ariba {
namespace utility {

use_logging_cpp(PathloadMeasurement);

PathloadMeasurement::PathloadMeasurement(BaseOverlay* _overlay)
	: running( false ), listener( NULL), resultNode( NodeID::UNSPECIFIED ),
	   serverpid( -1 ) {

	if( _overlay != NULL ) // this is important due to the singleton interface!
		baseoverlay = _overlay;

	//
	// start the pathload sender which acts as server
	// the server is started in the child process
	// if an instance is already running, the second
	// will just fail starting up ...
	//

	const char* argv[4];
	argv[0] = "pathload_snd";
	argv[1] = "-q";
	argv[2] = "-i";
	argv[3] = 0;

	if( (serverpid = vfork()) == 0 ){

		// execute in child process
		execvp( "pathload_snd", (char* const*)argv );

		// if we reach here, the exec failed
		// and we quit the child process
		logging_warn( "no bandwidth measurement module found " <<
				"(executing pathload_snd for bandwidth measurement server failed)" );
	}
}

PathloadMeasurement::~PathloadMeasurement(){

	// quit the pathload server
	if( serverpid != -1 )
		kill( serverpid, SIGQUIT );
}

void PathloadMeasurement::measure(const NodeID& destnode,  PathloadMeasurementListener* _listener){

	if( running ){
		logging_warn( "measurement already running" );
		return;
	}

	logging_info( "starting new measurement for " << destnode.toString() );

	listener = _listener;
	resultNode = destnode;
	resultMbps = -1;

	runBlockingMethod();
}

void PathloadMeasurement::dispatchFunction(){
	if( listener != NULL )
		listener->onMeasurement( resultNode, resultMbps );
}

void PathloadMeasurement::blockingFunction(){

	// get the endpoint for this node
	const EndpointDescriptor& endp = baseoverlay->getEndpointDescriptor( resultNode );
	if( endp.isUnspecified() ){
		logging_warn( "can not measure node " << resultNode.toString() << ": can't resolve endpoint" );
		return;
	}

	// if this is our local endpoint don't perform measurement
	if( endp == baseoverlay->getEndpointDescriptor() ){
		logging_debug( "don't perform measurement on local machine" );
		resultMbps = -1;
		dispatch();
		return;
	}

	// currently we can be sure that this is
	// an IPv4 and port format, or only IPv4
	string endpoint = endp.toString();
	string::size_type p = endpoint.find(':');
	if( p != string::npos ) endpoint = endpoint.substr( 0, p );

	logging_info( "measuring node " << resultNode.toString() <<
					" on endpoint " << endpoint );

	string cmdline = "pathload_rcv -s " + endpoint;

	// execute the binary and open a stream to the executables output (stdout)
	FILE* stream = popen( cmdline.c_str(), "r" );
	if( stream == NULL || stream <= 0){
		logging_warn( "no bandwidth measurement module found " <<
				"(executing pathload_rvc for bandwidth measurement failed)" );
		return;
	}

	char buf[128];
	string content = "";
//	bool failed = true;

	while( fgets(buf, 100, stream) != NULL ){
		content += buf;
	}

	logging_debug("pathload measurment output:\n" << content );

	//
	// evaluate the output
	//

	if( content.find("Connection refused") != string::npos ){
		logging_warn( "bandwidth measurement failed due to connection error" );
		return;
	}

	if( content.find("Measurements terminated") != string::npos ){
		logging_warn( "bandwidth measurement failed" );
		return;
	}

	if( content.find("Measurements finished") == string::npos ){
		logging_warn( "bandwidth measurement failed" );
		return;
	}

	try {
		content = content.substr( content.find("Available bandwidth range") );
		content = content.substr( content.find("-")+2 );
		content = content.erase( content.find("(")-1, string::npos );

		resultMbps = strtod(content.c_str(), NULL);
		logging_info( "measurement for node " << resultNode.toString() <<
				" on endpoint " << endp.toString() << " ended with: " << resultMbps );
	} catch(...) {
		logging_warn( "bandwidth measurement failed" );
	}

	// dispatch into the main thread and async jump into
	// void PathloadMeasurement::dispatchFunction()
	dispatch();
}

}} // namespace ariba, utility
