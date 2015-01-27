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

#ifndef __BOOTSTRAP_MANAGER_H
#define __BOOTSTRAP_MANAGER_H

#include <map>
#include <algorithm>
#include <vector>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/bootstrap/modules/BootstrapModule.h"
#include "ariba/utility/bootstrap/BootstrapInformationCallback.h"

using std::string;
using std::map;
using std::find;
using std::vector;

namespace ariba {
namespace utility {

class BootstrapManager : private boost::noncopyable, private BootstrapInformationCallback {
	use_logging_h(BootstrapManager);
	friend class BootstrapModule;
public:

	static BootstrapManager& instance() {
		static BootstrapManager _inst;
		return _inst;
	}

	enum BootstrapType {
		BootstrapTypeMulticastDns,			// use mDNS bootstrapping
		BootstrapTypePeriodicBroadcast, 	// stupid periodic broadcasting
		BootstrapTypeBluetoothSdp,			// bluetooth service discovery
	};

	enum RegistrationResult {
		RegistrationSucceeded,
		RegistrationNotSupported,
		RegistrationFailed,
	};

	RegistrationResult registerAllModules();
	RegistrationResult registerModule(BootstrapType type, string info);
	RegistrationResult unregisterAllModules();
	RegistrationResult unregisterModule(BootstrapType type);
	bool isModuleRegistered(BootstrapType type);

	void registerCallback(BootstrapInformationCallback* _callback);
	void unregisterCallback(BootstrapInformationCallback* _callback);

	void publish(string name, string info1, string info2, string info3);
	void revoke(string name);

protected:
	virtual void onBootstrapServiceFound(string name, string info1, string info2, string info3);

private:
	BootstrapManager();
	~BootstrapManager();

	typedef map<BootstrapType, BootstrapModule*> ModuleMap;
	ModuleMap modules;
	boost::mutex modulesMutex;

	typedef vector<BootstrapInformationCallback*> Callbacks;
	Callbacks callbacks;
};

}} //namespace ariba, utility

#endif // __BOOTSTRAP_MANAGER_H
