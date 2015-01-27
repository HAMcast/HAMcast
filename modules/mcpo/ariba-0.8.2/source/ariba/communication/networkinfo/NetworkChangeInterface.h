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

#ifndef __NETWORK_CHANGE_INTERFACE_H
#define __NETWORK_CHANGE_INTERFACE_H

#include <string>
#include <iostream>
#include "ariba/communication/networkinfo/NetworkInterface.h"

using std::string;
using std::ostream;
using ariba::communication::NetworkInterface;

namespace ariba {
namespace communication {

class NetworkChangeInterface {
protected:

	typedef enum _EventType {
		EventTypeInvalid,			// INVALID
		EventTypeInterfaceUp,		// network interface came up
		EventTypeInterfaceDown,		// network interface went down
	} EventType;

	static string eventString(EventType ev){
		switch( ev ){
			case EventTypeInvalid: 			return "invalid type";
			case EventTypeInterfaceUp: 		return "interface up";
			case EventTypeInterfaceDown: 	return "interface down";
			default:						return "unknown event";
		}
	}

	typedef struct _NetworkChangeInfo {
		EventType type;
		NetworkInterface interface;

		_NetworkChangeInfo(){
			type = EventTypeInvalid;
			interface = NetworkInterface::UNDEFINED;
		}
	} NetworkChangeInfo;

	virtual void onNetworkChange(const NetworkChangeInfo& info) = 0;

	friend class NetworkChangeDetection;
	friend ostream& operator<<(ostream& o, const NetworkChangeInterface::NetworkChangeInfo& info);
};

inline ostream& operator<<( ostream& o, const NetworkChangeInterface::NetworkChangeInfo& info ){
	return (o << 	"interface: " << info.interface.name <<
			", interface index: " << info.interface.index<<
			", type: " << NetworkChangeInterface::eventString(info.type)
	);
}

}} // namespace ariba, communication

#endif // __NETWORK_CHANGE_INTERFACE_H
