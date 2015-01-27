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

#ifndef DISCOVERY_H__
#define DISCOVERY_H__

#include <vector>

#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"
#include "ariba/utility/types/NodeID.h"
#include "ariba/communication/EndpointDescriptor.h"

using std::pair;
using std::make_pair;
using std::vector;
using ariba::utility::Message;
using ariba::utility::NodeID;
using ariba::communication::EndpointDescriptor;

namespace ariba {
namespace overlay {

using_serialization;

class Discovery : public Message {
	VSERIALIZEABLE;
public:
	enum type_ {
		invalid = 0,
		normal = 1,
		successor = 2,
		predecessor = 3
	};

	Discovery( const Discovery& msg ) : type(msg.type), ttl(msg.ttl),
		endpoint(msg.endpoint) {
	}
	Discovery( type_ type = invalid, uint8_t ttl = 4,
		const EndpointDescriptor& endpoint = EndpointDescriptor::UNSPECIFIED() )
	: type(type),  ttl(ttl), endpoint(endpoint) {
	}
	virtual ~Discovery();

	inline type_ getType() const {
		return (type_)type;
	}

	inline void setType( type_ type ) {
		this->type = type;
	}

	inline uint8_t getTTL() const {
		return ttl;
	}

	inline void setTTL( uint8_t ttl ) {
		this->ttl = ttl;
	}

	inline const EndpointDescriptor& getEndpoint() const {
		return endpoint;
	}

	inline void setEndpoint( const EndpointDescriptor& endpoint ) {
		this->endpoint = endpoint;
	}

private:
	uint8_t type;
	uint8_t ttl;
	EndpointDescriptor endpoint;
};

}} // ariba::overlay

sznBeginDefault( ariba::overlay::Discovery, X ) {
	X && type && ttl && endpoint;
} sznEnd();

#endif // DISCOVERY_H__
