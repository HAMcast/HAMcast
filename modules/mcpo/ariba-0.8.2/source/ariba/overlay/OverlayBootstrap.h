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

#ifndef __OVERLAY_BOOTSTRAP_H
#define __OVERLAY_BOOTSTRAP_H

#include <string>
#include <sstream>
#include <ctime>
#include <deque>
#include <vector>
#include <algorithm>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/types.h"
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/bootstrap/BootstrapManager.h"
#include "ariba/utility/bootstrap/BootstrapInformationCallback.h"
#include "ariba/communication/EndpointDescriptor.h"
#include "ariba/utility/system/SystemEventListener.h"
#include "ariba/utility/system/SystemQueue.h"
#include "ariba/utility/system/SystemEvent.h"
#include "ariba/utility/system/SystemEventType.h"

using std::swap;
using std::deque;
using std::string;
using std::vector;
using std::pair;
using std::ostringstream;
using ariba::utility::SpoVNetID;
using ariba::utility::NodeID;
using ariba::utility::BootstrapManager;
using ariba::utility::BootstrapInformationCallback;
using ariba::communication::EndpointDescriptor;
using ariba::utility::SystemEventType;
using ariba::utility::SystemEvent;
using ariba::utility::SystemQueue;
using ariba::utility::Timer;
using ariba::utility::SystemEventListener;

namespace ariba {
namespace overlay {

class BaseOverlay;

class OverlayBootstrap : public BootstrapInformationCallback, public SystemEventListener {
	use_logging_h(OverlayBootstrap);
public:
	OverlayBootstrap();
	virtual ~OverlayBootstrap();

	void start(
			BaseOverlay* _overlay,
			const SpoVNetID& _spovnetid,
			const NodeID& _nodeid,
			vector<pair<BootstrapManager::BootstrapType,string> > modules
			);
	void stop();

	void publish( const EndpointDescriptor& _ep );
	void revoke();

	void recordJoin(const EndpointDescriptor& _ep);
	bool haveOverlayConnections();

protected:
	virtual void handleSystemEvent(const SystemEvent& event);
	virtual void onBootstrapServiceFound(string name, string info1, string info2, string info);

private:
	class JoinData {
	public:
		JoinData() : spovnetid(), nodeid(), endpoint() {
		}

		JoinData& operator=( const JoinData& rhs) {
			spovnetid = rhs.spovnetid;
			nodeid = rhs.nodeid;
			endpoint = rhs.endpoint;
			return *this;
		}

		SpoVNetID spovnetid;
		NodeID nodeid;
		EndpointDescriptor endpoint;
	};

	BootstrapManager& manager;
	SpoVNetID spovnetid;
	NodeID nodeid;
	BaseOverlay* overlay;
	string randname;

	class WatchdogTimer : public Timer {
	public:
		WatchdogTimer(OverlayBootstrap* _obj);
		void startWatchdog();
		void stopWatchdog();
	protected:
		virtual void eventFunction();
	private:
		OverlayBootstrap* obj;
	};

	typedef deque<JoinData> JoinStack;
	JoinStack lastJoines;
	boost::mutex lastJoinesMutex;
	WatchdogTimer watchtimer;
	void checkOverlayStatus();


	bool haveOverlayConnection;
	boost::mutex haveOverlayConnectionMutex;
};

}} // namespace ariba, overlay

#endif // __OVERLAY_BOOTSTRAP_H
