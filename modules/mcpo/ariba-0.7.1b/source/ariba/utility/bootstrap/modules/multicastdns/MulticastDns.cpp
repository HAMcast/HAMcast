
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

#include "MulticastDns.h"

namespace ariba {
namespace utility {

const string MulticastDns::serviceType = "_spovnet._tcp";
use_logging_cpp(MulticastDns);

MulticastDns::MulticastDns(BootstrapInformationCallback* _callback, string info)
	: BootstrapModule(_callback) {
  #ifdef HAVE_AVAHI_CLIENT_CLIENT_H
	avahiclient = NULL;
	avahipoll = NULL;
	avahibrowser = NULL;
  #endif // HAVE_AVAHI_CLIENT_CLIENT_H
}

MulticastDns::~MulticastDns(){
}

string MulticastDns::getName(){
	return "MulticastDns";
}

string MulticastDns::getInformation(){
	return "bootstrap module based on multicast-dns using the avahi library";
}

bool MulticastDns::isFunctional(){
  #ifdef HAVE_AVAHI_CLIENT_CLIENT_H
	return true;
  #else
	return false;
  #endif
}

void MulticastDns::start(){
  #ifdef HAVE_AVAHI_CLIENT_CLIENT_H

	int error = 0;

	// create a new avahi polling thread
	avahipoll = avahi_threaded_poll_new();
	if( avahipoll == NULL){
		logging_error("creating avahi poll failed");
		return;
	}

	// create a new avahi client
	avahiclient = avahi_client_new(
			avahi_threaded_poll_get(avahipoll),
			(AvahiClientFlags)0,
			MulticastDns::client_callback,
			this,
			&error
			);

	if( avahiclient == NULL){
		logging_error("creating avahi client failed with error "<<
				error << ". make sure that the avahi-daemon is running. e.g. by installing avahi-utils");
		return;
	}

	// block the event loop
	avahi_threaded_poll_lock( avahipoll );

	// create the service browser for the specified type
	avahibrowser = avahi_service_browser_new(
			avahiclient, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
			serviceType.c_str(), NULL,
			(AvahiLookupFlags)0, MulticastDns::browse_callback, this);

	if( avahibrowser == NULL){
		logging_error("creating avahi browser failed");
		return;
	}

	//unblock the event loop and let it run
	avahi_threaded_poll_unlock( avahipoll );
	avahi_threaded_poll_start( avahipoll );

  #endif // HAVE_AVAHI_CLIENT_CLIENT_H
}

void MulticastDns::stop(){
  #ifdef HAVE_AVAHI_CLIENT_CLIENT_H

	//
	// stop poll and free browser
	//

	avahi_threaded_poll_stop( avahipoll );
	avahi_service_browser_free( avahibrowser );
	avahibrowser = NULL;

	//
	// free all registered groups
	//

	AvahiGroupMap::iterator i = avahigroups.begin();
	AvahiGroupMap::iterator iend = avahigroups.end();

	for( ; i != iend; i++)
		avahi_entry_group_free( i->second );

	//
	// free client and poll
	//

	if(avahiclient != NULL)
		avahi_client_free( avahiclient );
	avahiclient = NULL;

	if(avahipoll != NULL)
		avahi_threaded_poll_free( avahipoll );
	avahipoll = NULL;

  #endif // HAVE_AVAHI_CLIENT_CLIENT_H
}

void MulticastDns::publishService(string name, string info1, string info2, string info3){
  #ifdef HAVE_AVAHI_CLIENT_CLIENT_H

	if(name.length() > 63){
		logging_error("service name length must not exceed 63 characters. "
				<< name << " is " << name.length() << " characters");
		return;
	}

	if( avahiclient == NULL ){
		logging_error("avahi client is invalid");
		return;
	}

	avahi_threaded_poll_lock(avahipoll);
	int ret = 0;

	//
	// if we have no group for this service, create one
	//

	AvahiGroupMap::iterator igroup = avahigroups.find(name);
	AvahiEntryGroup* currentgroup = (igroup != avahigroups.end() ? igroup->second : NULL);

	if( currentgroup == NULL ){

		logging_debug("creating group for service " << name);
		currentgroup = avahi_entry_group_new(avahiclient, MulticastDns::entry_group_callback, this);

		if(currentgroup == NULL){
			logging_error("failed creating avahi group for service "
					<< name << ": " << avahi_strerror(avahi_client_errno(avahiclient)));
			avahi_threaded_poll_unlock(avahipoll);
			return;
		}

		avahigroups.insert( make_pair(name, currentgroup) );
	}

	assert( currentgroup != NULL );

	logging_debug("avahi adding service " << name << " to new group");

	ret = avahi_entry_group_add_service(
			currentgroup, 			// group to add service to
			AVAHI_IF_UNSPEC, 		// interface to announce, we use all interfaces
			AVAHI_PROTO_UNSPEC, 	// protocol to announce, we use all protocols
			(AvahiPublishFlags)0,	// no special flags
			name.c_str(),			// name of the service, no more than 63 characters
			serviceType.c_str(),	// type of the service: _spovnet._tcp (tcp does not mean anything here, just have to stick with this structure
			NULL,					// publish in all domains
			NULL,					// host name of our machine, let avahi find out
			3333,					// port number the service is on, just dummy, everything is encoded in TXT
			info1.c_str(),			// arbitrary info
			info2.c_str(),			// arbitrary info
			info3.c_str(),			// arbitrary info
			NULL);					// make that this is the last info field

	if( ret < 0 ){
		logging_warn("failed to add service " << name << ": " << avahi_strerror(ret));
		avahigroups.erase(name);
		avahi_threaded_poll_unlock(avahipoll);
		return;
	}

	// tell the server to register the service
	ret = avahi_entry_group_commit( currentgroup );
	if(ret < 0) {
		logging_warn("failed to commit entry group: " << avahi_strerror(ret));
		avahigroups.erase(name);
		avahi_threaded_poll_unlock(avahipoll);
		return;
	}

	avahi_threaded_poll_unlock(avahipoll);

  #endif // HAVE_AVAHI_CLIENT_CLIENT_H
}

void MulticastDns::revokeService(string name){
  #ifdef HAVE_AVAHI_CLIENT_CLIENT_H

	avahi_threaded_poll_lock(avahipoll);

	AvahiGroupMap::iterator i = avahigroups.find(name);
	if( i != avahigroups.end() ){

		logging_debug("revoking service " << name);
		avahi_entry_group_reset( i->second );

	} else {
		logging_warn("service " << name << " is not registered, cannot revoke");
	}

	avahi_threaded_poll_unlock(avahipoll);

  #endif // HAVE_AVAHI_CLIENT_CLIENT_H
}

#ifdef HAVE_AVAHI_CLIENT_CLIENT_H

void MulticastDns::client_callback(AvahiClient* client, AvahiClientState state, void* userdata){

	MulticastDns* obj = (MulticastDns*)userdata;
	assert( obj != NULL );

    switch (state) {
        case AVAHI_CLIENT_S_RUNNING:

        	// server has startup successfully and registered its host
            // name on the network, so it's time to create our services

        	logging_debug("avahi client is running");
            break;

        case AVAHI_CLIENT_FAILURE:

        	logging_warn( "avahi client failure "
        			<< avahi_strerror(avahi_client_errno(client)) << ". quitting" );
            avahi_threaded_poll_quit(obj->avahipoll);

            break;

        case AVAHI_CLIENT_S_COLLISION:

        	logging_warn("avahi client collision");
        	break;

        case AVAHI_CLIENT_S_REGISTERING:

            logging_debug("avahi client registering");
            break;

        case AVAHI_CLIENT_CONNECTING:

        	logging_debug("avahi client conencting");
            break;
    }
}

void MulticastDns::entry_group_callback(AvahiEntryGroup* group, AvahiEntryGroupState state, void* userdata){

	AvahiClient* client = avahi_entry_group_get_client( group );
	assert( client != NULL);

	MulticastDns* obj = (MulticastDns*)userdata;
	assert(obj != NULL);

	//
	// called whenever the entry group state changes
	//

	switch(state) {
		case AVAHI_ENTRY_GROUP_ESTABLISHED:

			logging_debug( "service entry group successfully established" );
			break;

		case AVAHI_ENTRY_GROUP_COLLISION:

			logging_warn("service name collision for name");
			break;

		case AVAHI_ENTRY_GROUP_FAILURE:

			logging_warn("service group failure: " << avahi_strerror(avahi_client_errno(client)));
			avahi_threaded_poll_quit(obj->avahipoll);

			break;

		case AVAHI_ENTRY_GROUP_UNCOMMITED:

			logging_debug("avahi entry group uncommited");
			break;

		case AVAHI_ENTRY_GROUP_REGISTERING:

			logging_debug("avahi entry group registering");
			break;

	} //switch(state)
}

void MulticastDns::browse_callback(AvahiServiceBrowser* browser, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiBrowserEvent event, const char* name,
		const char* type, const char* domain, AvahiLookupResultFlags flags, void* userdata){

	AvahiClient* client = avahi_service_browser_get_client(browser);
	MulticastDns* obj = (MulticastDns*)userdata;

	assert( client != NULL);
	assert( obj != NULL );

	switch (event) {
		case AVAHI_BROWSER_FAILURE:

			logging_warn("avahi browser failure " << avahi_strerror(avahi_client_errno(client)));
			avahi_threaded_poll_quit( obj->avahipoll );

			break;

		case AVAHI_BROWSER_NEW:

			if (!(avahi_service_resolver_new(client,
					interface, protocol, name, type, domain,
					AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0,
					MulticastDns::resolve_callback, obj))){
				logging_warn( "failed to resolve service " << name
						<< ", error " << avahi_strerror(avahi_client_errno(client)));
			}

			break;

		case AVAHI_BROWSER_REMOVE:

			logging_debug("avahi browser remove");
			break;

		case AVAHI_BROWSER_ALL_FOR_NOW:

			logging_debug("avahi all for now");
			break;

		case AVAHI_BROWSER_CACHE_EXHAUSTED:

			logging_debug("avahi browser cache exhausted");
			break;
	}
}

void MulticastDns::resolve_callback(AvahiServiceResolver* resolver, AvahiIfIndex interface,
		AvahiProtocol protocol, AvahiResolverEvent event, const char *name,
		const char* type, const char* domain, const char* host_name,
		const AvahiAddress* address, uint16_t port, AvahiStringList* txt,
		AvahiLookupResultFlags flags, void* userdata){

	AvahiClient* client = avahi_service_resolver_get_client(resolver);
	MulticastDns* obj = (MulticastDns*)userdata;

	assert( client != NULL );
	assert( obj  != NULL );

	switch(event) {
		case AVAHI_RESOLVER_FAILURE:

			logging_warn("resolver failed to resolve service " << name
					<< ", error " << avahi_strerror(avahi_client_errno(client)));
			break;

		case AVAHI_RESOLVER_FOUND:

			string info1 = "";
			string info2 = "";
			string info3 = "";

			if(txt != NULL){
				char* cinfo = (char*)avahi_string_list_get_text(txt);
				info1 = string(cinfo);
				txt = avahi_string_list_get_next(txt);
			}

			if(txt != NULL){
				char* cinfo = (char*)avahi_string_list_get_text(txt);
				info2 = string(cinfo);
				txt = avahi_string_list_get_next(txt);
			}

			if(txt != NULL){
				char* cinfo = (char*)avahi_string_list_get_text(txt);
				info3 = string(cinfo);
				txt = avahi_string_list_get_next(txt);
			}

			if(obj != NULL && obj->callback != NULL)
				obj->callback->onBootstrapServiceFound(name, info1, info2, info3);

			break;
	}

	avahi_service_resolver_free( resolver );
}

#endif // HAVE_AVAHI_CLIENT_CLIENT_H

}} //namespace ariba, utility
