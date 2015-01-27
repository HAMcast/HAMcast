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

#ifndef __BLUETOOTH_SDP_H
#define __BLUETOOTH_SDP_H

#include "ariba/config.h"

#include <iostream>
#include <string>
#include <ctime>
#include <cerrno>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "ariba/utility/bootstrap/modules/BootstrapModule.h"
#include "ariba/utility/logging/Logging.h"

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
  #include <bluetooth/bluetooth.h>
  #include <bluetooth/sdp.h>
  #include <bluetooth/sdp_lib.h>
  #include <bluetooth/hci.h>
  #include <bluetooth/hci_lib.h>
#endif

using std::string;

namespace ariba {
  namespace overlay {
    class OverlayBootstrap;
  }
}

using ariba::overlay::OverlayBootstrap;

namespace ariba {
namespace utility {

class BluetoothSdp : public BootstrapModule {
	use_logging_h(BluetoothSdp);
public:
	static OverlayBootstrap* CONNECTION_CHECKER;

	BluetoothSdp(BootstrapInformationCallback* _callback, string info);
	virtual ~BluetoothSdp();

	virtual void start();
	virtual void stop();

	virtual string getName();
	virtual string getInformation();
	virtual bool isFunctional();
	virtual void publishService(string name, string info1, string info2, string info3);
	virtual void revokeService(string name);

private:

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
	void bt_scan();
	void sdp_search(bdaddr_t target, string devicename);
	string ba2string(bdaddr_t* ba);
	string ba2name(bdaddr_t* ba, int sock);

	sdp_session_t *sdp_session_;
	uint8_t channel_;

	bool haveConnections();
#endif // HAVE_BLUETOOTH_BLUETOOTH_H

	boost::asio::io_service io_service_;
	boost::asio::deadline_timer scan_timer_;
	boost::thread t_;
};

}} //namespace ariba, utility

#endif // __BLUETOOTH_SDP_H
