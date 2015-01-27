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

#include "StartupWrapper.h"
#include "ariba/config.h"

#ifdef HAVE_LOG4CXX_LOGGER_H
	#include <log4cxx/logger.h>
	#include <log4cxx/basicconfigurator.h>
#endif // HAVE_LOG4CXX_LOGGER_H

namespace ariba {
namespace utility {

StartupWrapper::ConfigurationList StartupWrapper::configurations;
#ifdef UNDERLAY_OMNET
StartupWrapper::ModuleList StartupWrapper::modules;
#endif

SystemEventType StartupWrapperEventStartup("StartupWrapperEventStartup");

StartupWrapper::StartupWrapper(StartupInterface* _service) : service( _service ){
}

StartupWrapper::~StartupWrapper(){
}

#ifdef UNDERLAY_OMNET
void StartupWrapper::insertCurrentModule(AribaOmnetModule* mod){
	modules.push( mod );
}
#endif

#ifdef UNDERLAY_OMNET
AribaOmnetModule* StartupWrapper::getCurrentModule(){
	assert( modules.size() > 0 );

	AribaOmnetModule* ret = modules.front();
	modules.pop();

	return ret;
}
#endif

void StartupWrapper::startSystem(){

	//
	// having seeded the pseudo rng is always good
	//

	srand( time(NULL) );

	//
	// init the system queue
	//

	if( ! SystemQueue::instance().isRunning() )
		SystemQueue::instance().run();

	//
	// init the logging system
	//

#ifdef HAVE_LOG4CXX_LOGGER_H
	log4cxx::BasicConfigurator::configure();
#endif //HAVE_LOG4CXX_LOGGER_H

	//
	// configure the default logging level to info
	//

	logging_rootlevel_info();
}

void StartupWrapper::stopSystem(){
	SystemQueue::instance().cancel();
}

void StartupWrapper::initConfig(string filename){
	configurations.push( filename );
	Configuration::setConfigFilename( filename );
}

void StartupWrapper::handleSystemEvent(const SystemEvent& event){

	if( event.getType() == StartupWrapperEventStartup ){

		if(!configurations.empty()){
			string config = configurations.front();
			configurations.pop();
			Configuration::setConfigFilename( config );
		}

		//
		// start the actual application
		//

		// TODO: im falle von omnet ist service = null, da von SpoVNetOmnetModule so übergeben
		// wie wird im Falle von omnet die anwendung erstellt?

		service->startup();

	}

}

void StartupWrapper::startup(StartupInterface* service, bool block){

	StartupWrapper* startup = new StartupWrapper(service);
	service->wrapper = startup;

	SystemQueue::instance().scheduleEvent(
		SystemEvent( startup, StartupWrapperEventStartup, NULL), 0 );

#ifndef UNDERLAY_OMNET
	if( block ) getchar();
#endif
}

void StartupWrapper::shutdown(StartupInterface* service, bool block){

	if( service == NULL || service->wrapper == NULL ) return;

#ifdef UNDERLAY_OMNET
	//TODO: service->shutdown();
#endif

	if(block){
		// call directly
		service->shutdown();
	}else{
		// call async, but not using systemqueue! // TODO: mem leak
		AsyncShutdown* async = new AsyncShutdown(service);
		async->runBlockingMethod();
	}
}

StartupWrapper::AsyncShutdown::AsyncShutdown(StartupInterface* _service)
	: service(_service){
}

void StartupWrapper::AsyncShutdown::blockingFunction(){
	service->shutdown();
}

void StartupWrapper::AsyncShutdown::dispatchFunction(){
	//unused
}

}} // namespace ariba, utility
