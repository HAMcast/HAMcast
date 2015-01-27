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
// notice, this list of co// [License]
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

#include "PeriodicBroadcast.h"

namespace ariba {
namespace utility {

use_logging_cpp(PeriodicBroadcast);
const long PeriodicBroadcast::timerinterval = 1;
const long PeriodicBroadcast::servicetimeout = 3;
const unsigned int PeriodicBroadcast::serverport_v4 = 5634;
const unsigned int PeriodicBroadcast::serverport_v6 = 5636;

PeriodicBroadcast::PeriodicBroadcast(BootstrapInformationCallback* _callback, string info)
	: BootstrapModule(_callback),
	server(io_service, &newRemoteServices, &newRemoteServicesMutex) {
}

PeriodicBroadcast::~PeriodicBroadcast(){
}

void PeriodicBroadcast::threadFunc(PeriodicBroadcast* obj){
	obj->io_service.run();
}

string PeriodicBroadcast::getName(){
	return "PeriodicBroadcast";
}

string PeriodicBroadcast::getInformation(){
	return "periodic broadcasting of service information";
}

bool PeriodicBroadcast::isFunctional(){
	return true;
}

void PeriodicBroadcast::start(){
	io_service_thread = new boost::thread(
			boost::bind(&PeriodicBroadcast::threadFunc, this) );

	Timer::setInterval( timerinterval*1000 );
	Timer::start();
}

void PeriodicBroadcast::stop(){
	io_service.stop();
	io_service_thread->join();
	delete io_service_thread;
	io_service_thread = NULL;

	Timer::stop();
}

void PeriodicBroadcast::publishService(string name, string info1, string info2, string info3){
	Service service(name, info1, info2, info3) ;

	boost::mutex::scoped_lock lock( localServicesMutex );
	if(name.empty()) return;

	localServices.insert( std::make_pair(name, service) );
}

void PeriodicBroadcast::revokeService(string name){
	boost::mutex::scoped_lock lock( localServicesMutex );
	if(name.empty()) return;

	ServiceList::iterator i = localServices.find( name );
	if( i != localServices.end() ) localServices.erase( name );
}

void PeriodicBroadcast::eventFunction(){
	sendLocalServices();
	updateRemoteServices();
}

void PeriodicBroadcast::sendLocalServices(){
	boost::mutex::scoped_lock lock( localServicesMutex );

	ServiceList::iterator i = localServices.begin();
	ServiceList::iterator iend = localServices.end();

	for( ; i != iend; i++)
		server.sendservice( i->second );
}

void PeriodicBroadcast::updateRemoteServices(){

	// cleanup the services that timed out
	// so they are seen of as new after timeout
	{
		boost::mutex::scoped_lock lock( remoteServicesMutex );
		bool deleted;

		do {
			deleted = false;

			ServiceList::iterator i = remoteServices.begin();
			ServiceList::iterator iend = remoteServices.end();

			for( ; i != iend; i++ ){

				if( time(NULL) > (i->second.getLastseen() + servicetimeout) ){
					remoteServices.erase( i );
					deleted = true;
					break;
				}
			}

		} while(deleted);
	}

	// check if we received new services:
	// check remoteServices against newRemoteServices
	{
		boost::mutex::scoped_lock lock( newRemoteServicesMutex );
		typedef std::pair<string,Service> mapitem;

		BOOST_FOREACH( mapitem item, newRemoteServices ){

			string name = item.first;
			Service service = item.second;

			ServiceList::iterator i = remoteServices.find( name );
			if( i != remoteServices.end() ) {
				// update the item lastseen time
				i->second.setLastseen( service.getLastseen() );
				continue;
			}

			{
				// insert the new item as new, lastseen has been set in the
				// receive function, as this timer only runs in intervals
				boost::mutex::scoped_lock lock2( remoteServicesMutex );
				remoteServices.insert( std::make_pair(name, service) );
			}

			callback->onBootstrapServiceFound(name,
					service.getInfo1(), service.getInfo2(), service.getInfo3());
		}

		// we have checked and transfered all new items
		newRemoteServices.clear();
	}
}

}} //namespace ariba, utility
