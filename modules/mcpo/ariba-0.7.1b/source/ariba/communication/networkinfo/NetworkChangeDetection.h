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

#ifndef __NETWORK_CHANGE_DETECTION_H
#define __NETWORK_CHANGE_DETECTION_H

#include <cerrno>
#include <csignal>
#include <vector>
#include <algorithm>
#include <string>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include "ariba/communication/networkinfo/NetworkChangeInterface.h"
#include "ariba/communication/networkinfo/NetworkInformation.h"
#include "ariba/utility/system/SystemQueue.h"
#include "ariba/utility/logging/Logging.h"

using std::string;
using std::vector;
using std::find;
using ariba::utility::SystemQueue;
using ariba::utility::SystemEvent;
using ariba::utility::SystemEventType;
using ariba::utility::SystemEventListener;
using ariba::communication::NetworkInformation;

namespace ariba {
namespace communication {

class NetworkChangeDetection : public SystemEventListener {
	use_logging_h(NetworkChangeDetection);
public:
	NetworkChangeDetection();
	virtual ~NetworkChangeDetection();

	void registerNotification( NetworkChangeInterface* callback );
	void unregisterNotification( NetworkChangeInterface* callback );

protected:
	void handleSystemEvent( const SystemEvent& event );

private:
	typedef vector<NetworkChangeInterface*> RegistrationList;
	RegistrationList registrations;

	void startMonitoring();
	void stopMonitoring();

	volatile bool running;
	boost::thread* monitoringThread;
	static void monitoringThreadFunc( NetworkChangeDetection* obj );

	int routingSocket;
	NetworkInformation networkInformation;

	NetworkChangeInterface::NetworkChangeInfo extractAddressEvent( struct nlmsghdr* header );
};

}} // namespace ariba, communication

#endif // __NETWORK_CHANGE_DETECTION_H
