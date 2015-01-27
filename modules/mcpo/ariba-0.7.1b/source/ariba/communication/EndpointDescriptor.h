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

#ifndef ENDPOINTDESCRIPTOR_H_
#define ENDPOINTDESCRIPTOR_H_

#include <string>
#include <set>
#include "ariba/utility/serialization.h"
#include "ariba/utility/types/PeerID.h"
#include "ariba/utility/addressing/endpoint_set.hpp"

namespace ariba {
namespace communication {

using_serialization;
using namespace std;
using namespace ariba::addressing;
using ariba::utility::PeerID;

class EndpointDescriptor: public VSerializeable { VSERIALIZEABLE
	friend class BaseCommunication;

public:
	/// creates an empty endpoint descriptor with zero endpoints
	EndpointDescriptor();

	/// destructor.
	virtual ~EndpointDescriptor();

	/// copy constructor
	EndpointDescriptor(const EndpointDescriptor& rh);

	/// construct end-points from an endpoint set
	EndpointDescriptor(const endpoint_set& endpoints );

	/// construct end-points from a string
	EndpointDescriptor(const string& str);

	/// convert end-points to string
	string toString() const {
		return endpoints.to_string();
	}

	static EndpointDescriptor& UNSPECIFIED() {
		static EndpointDescriptor* unspec = NULL;
		if(unspec == NULL) unspec = new EndpointDescriptor();

		return *unspec;
	}

	/// returns true, if this object is the unspecified object
	bool isUnspecified() const {
		return (this == &UNSPECIFIED());
	}

	/// create endpoint
	static EndpointDescriptor* fromString(string str) {
		return new EndpointDescriptor(str);
	}

	bool operator==(const EndpointDescriptor& rh) const {
		if (rh.isUnspecified() && isUnspecified()) return true;
		if (rh.isUnspecified() ^  isUnspecified()) return false;

		assert( (!rh.isUnspecified()) && (!isUnspecified()) );
		return endpoints == rh.endpoints;
	}

	bool operator!=(const EndpointDescriptor& rh) const {
		return ( !operator==(rh) );
	}

	EndpointDescriptor& operator=( const EndpointDescriptor& rhs) {
		endpoints = rhs.endpoints;
		return *this;
	}

	/// returns the end-points of this descriptor
	endpoint_set& getEndpoints() {
		return endpoints;
	}

	/// returns the end-points of this descriptor
	const endpoint_set& getEndpoints() const {
		return endpoints;
	}

	/// returns a reference to the peer id
	PeerID& getPeerId() {
		return peerId;
	}


	/// returns a reference to the constant peer id
	const PeerID& getPeerId() const {
		return peerId;
	}
private:
	endpoint_set endpoints;
	PeerID peerId;
};

}} // namespace ariba, communication

sznBeginDefault( ariba::communication::EndpointDescriptor, X ){

	// serialize peer id
	X && &peerId;

	// serialize end-points
	uint16_t len = endpoints.to_bytes_size();
	X && len;
	uint8_t* buffer = X.bytes( len );
	if (buffer!=NULL) {
		if (X.isDeserializer()) endpoints.assign(buffer,len);
		else endpoints.to_bytes(buffer);
	}
}sznEnd();

#endif /*ENDPOINTDESCRIPTOR_H_*/
