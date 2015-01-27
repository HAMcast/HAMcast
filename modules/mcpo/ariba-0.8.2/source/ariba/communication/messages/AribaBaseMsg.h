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

#ifndef ARIBA_BASE_MSG_H__
#define ARIBA_BASE_MSG_H__

#include <string>
#include <boost/cstdint.hpp>
#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"
#include "ariba/utility/types/LinkID.h"
#include "ariba/utility/types/Address.h"
#include "ariba/utility/types/ServiceID.h"

#include "../EndpointDescriptor.h"

using std::string;
using ariba::utility::Address;
using ariba::utility::LinkID;
using ariba::utility::Message;
using ariba::utility::ServiceID;

namespace ariba {
namespace communication {

using_serialization;

class AribaBaseMsg : public Message {
	VSERIALIZEABLE;
public:
	enum type_ {
		typeData = 0,
		typeLinkRequest = 1,
		typeLinkReply = 2,
		typeLinkClose = 3,
		typeLinkUpdate = 4
	};

	AribaBaseMsg( type_ type = typeData,
			const LinkID& localLink = LinkID::UNSPECIFIED,
			const LinkID& remoteLink = LinkID::UNSPECIFIED );

	virtual ~AribaBaseMsg();

	const string getTypeString() const;

	const type_ getType() const {
		return (type_)type;
	}

	const LinkID& getLocalLink() const {
		return localLink;
	}

	const LinkID& getRemoteLink() const {
		return remoteLink;
	}

	EndpointDescriptor& getLocalDescriptor() {
		return localDescriptor;
	}

	EndpointDescriptor& getRemoteDescriptor() {
		return remoteDescriptor;
	}

private:
	uint8_t type;		// the link message type

	// remote and local link ids
	LinkID localLink;	// the local link id
	LinkID remoteLink;	// the remote link id

	// remote and local endpoint descriptors
	EndpointDescriptor localDescriptor;
	EndpointDescriptor remoteDescriptor;
};

}} // ariba::communication

sznBeginDefault( ariba::communication::AribaBaseMsg, X ) {
	X && type && &localLink && &remoteLink;
	if (type == typeLinkReply || type == typeLinkRequest)
		X && localDescriptor && remoteDescriptor;
	X && Payload();
} sznEnd();

#endif // ARIBA_BASE_MSG_H__
