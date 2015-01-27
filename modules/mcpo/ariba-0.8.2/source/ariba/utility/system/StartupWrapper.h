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

#ifndef __STARTUP_WRAPPER_H
#define __STARTUP_WRAPPER_H

#include <string>
#include <queue>
#include "SystemQueue.h"
#include "StartupInterface.h"
#include "ariba/utility/configuration/Configuration.h"
#include "BlockingMethod.h"

#ifdef UNDERLAY_OMNET
namespace ariba {
namespace communication {
  class AribaOmnetModule;
}}

using ariba::communication::AribaOmnetModule;
#endif

using std::queue;
using std::string;
using ariba::utility::SystemQueue;
using ariba::utility::SystemEvent;
using ariba::utility::SystemEventType;
using ariba::utility::SystemEventListener;
using ariba::utility::StartupInterface;
using ariba::utility::Configuration;

namespace ariba {
namespace utility {

class StartupWrapper : public SystemEventListener {
public:
	static void startSystem();
	static void stopSystem();

	static void initConfig(string filename);
	static void startup(StartupInterface* service, bool block = true);
	static void shutdown(StartupInterface* service, bool block = true);

#ifdef UNDERLAY_OMNET
	static void insertCurrentModule( AribaOmnetModule* mod );
	static AribaOmnetModule* getCurrentModule();
#endif

protected:
	StartupWrapper(StartupInterface* _service);
	virtual ~StartupWrapper();

	void handleSystemEvent( const SystemEvent& event );

private:
	typedef queue<string> ConfigurationList;
	static ConfigurationList configurations;

#ifdef UNDERLAY_OMNET
	typedef queue<AribaOmnetModule*> ModuleList;
	static ModuleList modules;
#endif

	void waitForExit();
	StartupInterface* service;

	class AsyncShutdown : public BlockingMethod {
	public:
		AsyncShutdown(StartupInterface* _service);
	protected:
		virtual void dispatchFunction();
		virtual void blockingFunction(); // unused
	private:
		StartupInterface* service;
	};

};

}} // namespace ariba, common

#endif // __STARTUP_WRAPPER_H
