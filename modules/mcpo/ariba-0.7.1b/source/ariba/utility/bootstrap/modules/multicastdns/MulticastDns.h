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

#ifndef __MULTICAST_DNS_H
#define __MULTICAST_DNS_H

#include "ariba/config.h"

#ifdef HAVE_AVAHI_CLIENT_CLIENT_H
  #include <avahi-client/client.h>
  #include <avahi-client/lookup.h>
  #include <avahi-client/publish.h>
  #include <avahi-common/alternative.h>
  #include <avahi-common/thread-watch.h>
  #include <avahi-common/malloc.h>
  #include <avahi-common/error.h>
  #include <avahi-common/timeval.h>
#endif // HAVE_AVAHI_CLIENT_CLIENT_H

#include <iostream>
#include <string>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "ariba/utility/bootstrap/modules/BootstrapModule.h"
#include "ariba/utility/logging/Logging.h"

using std::string;
using std::map;
using std::make_pair;

namespace ariba {
namespace utility {

class MulticastDns : public BootstrapModule {
	use_logging_h(MulticastDns);
public:
	MulticastDns(BootstrapInformationCallback* _callback, string info);
	virtual ~MulticastDns();

	virtual void start();
	virtual void stop();

	virtual string getName();
	virtual string getInformation();
	virtual bool isFunctional();
	virtual void publishService(string name, string info1, string info2, string info3);
	virtual void revokeService(string name);

private:
	static const string serviceType;

#ifdef HAVE_AVAHI_CLIENT_CLIENT_H

	AvahiClient*         avahiclient;
	AvahiThreadedPoll*   avahipoll;
	AvahiServiceBrowser* avahibrowser;

	typedef map<string, AvahiEntryGroup*> AvahiGroupMap;
	AvahiGroupMap avahigroups;

	static void client_callback(
			AvahiClient* client,
			AvahiClientState state,
			void* userdata
			);

	static void entry_group_callback(
			AvahiEntryGroup* group,
			AvahiEntryGroupState state,
			void* userdata);

	static void browse_callback(
			AvahiServiceBrowser* browser,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiBrowserEvent event,
			const char* name,
			const char* type,
			const char* domain,
			AvahiLookupResultFlags flags,
			void* userdata);

	static void resolve_callback(
			AvahiServiceResolver* resolver,
			AvahiIfIndex interface,
			AvahiProtocol protocol,
			AvahiResolverEvent event,
			const char* name,
			const char* type,
			const char* domain,
			const char* host_name,
			const AvahiAddress* address,
			uint16_t port,
			AvahiStringList* txt,
			AvahiLookupResultFlags flags,
			void* userdata
			);

#endif // HAVE_AVAHI_CLIENT_CLIENT_H

};

}} //namespace ariba, utility

#endif // __MULTICAST_DNS_H
