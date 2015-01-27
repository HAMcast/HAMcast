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


#include "BootstrapManager.h"
#include "ariba/utility/bootstrap/modules/multicastdns/MulticastDns.h"
#include "ariba/utility/bootstrap/modules/periodicbroadcast/PeriodicBroadcast.h"
#include "ariba/utility/bootstrap/modules/bluetoothsdp/BluetoothSdp.h"

namespace ariba {
namespace utility {

use_logging_cpp(BootstrapManager);

BootstrapManager::BootstrapManager(){
}

BootstrapManager::~BootstrapManager(){
}

BootstrapManager::RegistrationResult BootstrapManager::registerModule(
		BootstrapManager::BootstrapType type, string info){

	boost::mutex::scoped_lock lock( modulesMutex );

	ModuleMap::iterator i = modules.find(type);
	if( i != modules.end() ){
		logging_error("bootstrap module " << i->second->getName() << " already registered");
		return RegistrationFailed;
	}

	BootstrapModule* module = NULL;

	switch(type){
	case BootstrapTypeMulticastDns:
		module = new MulticastDns(this, info);
		break;
	case BootstrapTypePeriodicBroadcast:
		module = new PeriodicBroadcast(this, info);
		break;
	case BootstrapTypeBluetoothSdp:
		module = new BluetoothSdp(this, info);
		break;
	}

	if( module == NULL){
		logging_error("bootstrap module for type " << type << " not found");
		return RegistrationNotSupported;
	}

	if( !module->isFunctional() ){
		logging_error("bootstrap module " << module->getName() << " is not functional");
		delete module;
		return RegistrationFailed;
	}

	logging_debug("bootstrap module " << module->getName() << " registered");

	module->start();
	modules.insert( std::make_pair(type, module) );

	return RegistrationSucceeded;
}

bool BootstrapManager::isModuleRegistered(BootstrapType type){
	boost::mutex::scoped_lock lock( modulesMutex );

	ModuleMap::iterator i = modules.find(type);
	return  (i != modules.end());
}

BootstrapManager::RegistrationResult BootstrapManager::unregisterModule(
		BootstrapManager::BootstrapType type){

	boost::mutex::scoped_lock lock( modulesMutex );

	ModuleMap::iterator i = modules.find(type);
	if( i == modules.end() ) return RegistrationFailed;

	BootstrapModule* module = i->second;
	module->stop();
	delete module;
	modules.erase(i);

	logging_debug("bootstrap module " << module->getName() << " unregistered");

	return RegistrationSucceeded;
}

BootstrapManager::RegistrationResult BootstrapManager::registerAllModules(){
	RegistrationResult result = RegistrationSucceeded;

	{ // multicast dns
		RegistrationResult resone = RegistrationSucceeded;
		resone = registerModule(BootstrapTypeMulticastDns, "");

		if(resone != RegistrationSucceeded)
			result = resone;
	}

	{ // periodic broadcast
		RegistrationResult resone = RegistrationSucceeded;
		resone = registerModule(BootstrapTypePeriodicBroadcast, "");

		if(resone != RegistrationSucceeded)
			result = resone;
	}

	{ // bluetooth sdp
			RegistrationResult resone = RegistrationSucceeded;
			resone = registerModule(BootstrapTypeBluetoothSdp, "");

			if(resone != RegistrationSucceeded)
				result = resone;
		}

	{ // todo
		/*  ...   */
	}

	return result;
}

BootstrapManager::RegistrationResult BootstrapManager::unregisterAllModules(){
	unregisterModule(BootstrapTypeMulticastDns);
	unregisterModule(BootstrapTypePeriodicBroadcast);
	unregisterModule(BootstrapTypeBluetoothSdp);
	/*  todo  ...  */

	return RegistrationSucceeded;
}

void BootstrapManager::onBootstrapServiceFound(string name, string info1, string info2, string info3){

	BOOST_FOREACH( BootstrapInformationCallback* callback, callbacks ){
		callback->onBootstrapServiceFound(name, info1, info2, info3);
	}
}

void BootstrapManager::registerCallback(BootstrapInformationCallback* _callback){
	Callbacks::iterator i = find( callbacks.begin(), callbacks.end(), _callback );
	if( i == callbacks.end() )
		callbacks.push_back(_callback);
}

void BootstrapManager::unregisterCallback(BootstrapInformationCallback* _callback){
	Callbacks::iterator i = find( callbacks.begin(), callbacks.end(), _callback );
	if( i != callbacks.end() )
		callbacks.erase(i);
}

void BootstrapManager::publish(string name, string info1, string info2, string info3){
	boost::mutex::scoped_lock lock( modulesMutex );

	ModuleMap::iterator i = modules.begin();
	ModuleMap::iterator iend = modules.end();

	for( ; i != iend; i++ ){
		logging_info("bootstrap manager publishing service "
				<< name << " on module " << i->second->getName());
		i->second->publishService(name, info1, info2, info3);
	}
}

void BootstrapManager::revoke(string name){
	boost::mutex::scoped_lock lock( modulesMutex );

	ModuleMap::iterator i = modules.begin();
	ModuleMap::iterator iend = modules.end();

	for( ; i != iend; i++ ){
		logging_info("bootstrap manager revoking service "
				<< name << " on module " << i->second->getName());
		i->second->revokeService(name);
	}

}

}} //namespace ariba, utility
