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

#include "AribaModule.h"

// boost/std includes
#include <assert.h>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

// ariba includes
#include "ariba/SideportListener.h"
#include "ariba/utility/misc/Helper.h"
#include "ariba/utility/misc/StringFormat.h"
#include "ariba/communication/BaseCommunication.h"
#include "ariba/communication/EndpointDescriptor.h"

using namespace ariba::utility::Helper;
using ariba::communication::BaseCommunication;
using ariba::communication::EndpointDescriptor;

namespace ariba {

use_logging_cpp(AribaModule);
const string AribaModule::BootstrapMechanismNames[5]
         = {"invalid", "static", "broadcast", "mdns", "sdp"};

AribaModule::AribaModule()
	: started(false), base_comm(NULL), sideport_sniffer(NULL) {
	endpoints = "tcp{41322};rfcomm{10};";
}

AribaModule::~AribaModule() {
}

string AribaModule::getBootstrapHints(const Name& spoVNetName) const {
	std::ostringstream o;
	int i=0;
	BOOST_FOREACH( const BootstrapInfo& info, bootstrapNodes ) {
		o << info.spovnetName.toString() << "{";
		if (i!=0) o << ",";

		BOOST_FOREACH( const BootstrapNode& node, info.nodes ) {
			if( node.desc != NULL )
				o << node.desc->toString();
			else if(node.mechanism == BootstrapMechanismBroadcast
					|| node.mechanism == BootstrapMechanismMulticastDNS
					|| node.mechanism == BootstrapMechanismSDP){
				o << BootstrapMechanismNames[node.mechanism];
				if( !node.info.empty() ) o << "{" << node.info << "}";
				o << ";";
			}
		}
		o << "}";
		i++;
	}
	return o.str();
}

void AribaModule::addBootstrapHints(string boot_info) {
	using namespace boost::xpressive;
	using namespace ariba::utility::string_format;
	using namespace ariba::utility::Helper;
	using namespace std;

	boost::erase_all(boot_info, " ");
	boost::erase_all(boot_info, "\t");
	boost::erase_all(boot_info, "\n");
	boost::erase_all(boot_info, "\r");

	smatch match;
	if (regex_search(boot_info, match, robjects)) {
		regex_nav nav = match;
		for (int i = 0; i < nav.size(); i++) {
			string type = nav[i][robject_id].str();
			string data = nav[i][robject_data].str();
			data = data.substr(1, data.size() - 2);
			Name name(type);

			// find static bootstrap info --> BootstrapMechanismStatic
			EndpointDescriptor* desc = EndpointDescriptor::fromString(data);
			addBootstrapNode(name, desc, "", BootstrapMechanismStatic);

			// find automatic bootstrap info --> {BootstrapMechanismBroadcast,
			// 					BootstrapMechanismMulticastDNS,BootstrapMechanismSDP}
			typedef vector< string > split_vector_type;
			split_vector_type splitvec;
			boost::split( splitvec, data, boost::is_any_of(";") );
			for(unsigned int i=0; i<splitvec.size(); i++){
				string x = splitvec[i];
				split_vector_type innervec;

				boost::split( innervec, x, boost::is_any_of("{}") );
				BootstrapNode node;
				if(innervec.size() < 1) continue;

				if(innervec[0] == BootstrapMechanismNames[BootstrapMechanismBroadcast])
					node.mechanism = BootstrapMechanismBroadcast;
				else if(innervec[0] == BootstrapMechanismNames[BootstrapMechanismMulticastDNS])
					node.mechanism = BootstrapMechanismMulticastDNS;
				else if(innervec[0] == BootstrapMechanismNames[BootstrapMechanismSDP])
					node.mechanism = BootstrapMechanismSDP;
				else
					continue;

				if(innervec.size() > 1)
					node.info = innervec[1];

				this->addBootstrapNode(name, node);
			}
		}
	}

	logging_info( "Added bootstrap hints: " << getBootstrapHints() );
}

void AribaModule::addBootstrapNode(const Name& spovnet,
		communication::EndpointDescriptor* desc, const string& info,
		const BootstrapMechanism& mechanism) {
	BootstrapNode node(0, desc, mechanism, info);
	addBootstrapNode(spovnet, node);
}

void AribaModule::addBootstrapNode(const Name& spovnet, const BootstrapNode& node){
	bool added = false;

	// add node to existing bootstrap list
	BOOST_FOREACH( BootstrapInfo& info, bootstrapNodes ){
		if (info.spovnetName == spovnet) {
			info.nodes.push_back(node);
			added = true;
			break;
		}
	}

	// create new entry
	if (!added) {
		BootstrapInfo info;
		info.spovnetName = spovnet;
		info.nodes.push_back(node);
		bootstrapNodes.push_back(info);
	}
}

const communication::EndpointDescriptor* AribaModule::getBootstrapNode(
		const Name& spovnet, const BootstrapMechanism mechanism) const {
	BOOST_FOREACH( const BootstrapInfo& info, bootstrapNodes ) {
		if( info.spovnetName == spovnet ) {
			BOOST_FOREACH( const BootstrapNode& node, info.nodes ) {
				if( node.mechanism == mechanism && node.desc != NULL )
					return node.desc;
			}
		}
	}
	return NULL;
}

string AribaModule::getBootstrapInfo(
		const Name& spovnet, const BootstrapMechanism mechanism) const {
	BOOST_FOREACH( const BootstrapInfo& info, bootstrapNodes ) {
		if( info.spovnetName == spovnet ) {
			BOOST_FOREACH( const BootstrapNode& node, info.nodes ) {
				if( node.mechanism == mechanism && node.desc != NULL )
					return node.info;
			}
		}
	}

	return string();
}

vector<AribaModule::BootstrapMechanism> AribaModule::getBootstrapMechanisms(
		const Name& spovnet) const {
	vector<AribaModule::BootstrapMechanism> ret;
	BOOST_FOREACH( const BootstrapInfo& info, bootstrapNodes ) {
		if( info.spovnetName == spovnet ) {
			BOOST_FOREACH( const BootstrapNode& node, info.nodes ) {
				if(std::find(ret.begin(), ret.end(), node.mechanism) == ret.end())
					ret.push_back(node.mechanism);
			}
		}
	}
	return ret;
}

void AribaModule::registerSideportListener(SideportListener* sideport){
	sideport_sniffer = sideport;
}

// @see Module.h
void AribaModule::initialize() {

	// preconditions
	assert(!started);

	// init variables
	base_comm = NULL;
}

// @see Module.h
void AribaModule::start() {

	// preconditions
	assert(base_comm == NULL);
	assert(!started);

	// create the base communication component
	started = true;
	base_comm = new BaseCommunication();
	base_comm->setEndpoints(endpoints);
}

// @see Module.h
void AribaModule::stop() {

	// preconditions
	assert(base_comm != NULL);
	assert(started);

	// delete base communication component
	started = false;
	delete base_comm;
}

// @see Module.h
string AribaModule::getName() const {
	return "ariba";
}

// @see Module.h
void AribaModule::setProperty(string key, string value) {
	if (key == "endpoints") endpoints = value;
	else if (key == "bootstrap.hints") addBootstrapHints(value);
}

// @see Module.h
const string AribaModule::getProperty(string key) const {
	if (key == "endpoints") return endpoints; // TODO: return local endpoints
	else if (key == "bootstrap.hints") return getBootstrapHints();
	return "";
}

// @see Module.h
const vector<string> AribaModule::getProperties() const {
	vector<string> properties;
	properties.push_back("endpoints");
	properties.push_back("bootstrap.hints");
	return properties;
}

const string AribaModule::getLocalEndpoints() {
	return base_comm->getEndpointDescriptor().toString();
}

} // namespace ariba
