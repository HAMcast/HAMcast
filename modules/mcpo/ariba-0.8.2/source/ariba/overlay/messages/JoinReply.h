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

#ifndef JOIN_REPLY_H__
#define JOIN_REPLY_H__

#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"
#include "ariba/utility/types/SpoVNetID.h"
#include "ariba/utility/types/NodeID.h"
#include "ariba/utility/types/OverlayParameterSet.h"
#include "ariba/communication/EndpointDescriptor.h"

using ariba::utility::OverlayParameterSet;
using ariba::utility::Message;
using ariba::utility::SpoVNetID;
using ariba::utility::NodeID;
using ariba::communication::EndpointDescriptor;

namespace ariba {
namespace overlay {

using_serialization;

class JoinReply : public Message {
	VSERIALIZEABLE;
private:
	SpoVNetID spovnetid; //< the spovnet instance i want to join
	OverlayParameterSet param; //< overlay parameters
	bool joinAllowed; //< join successfull or access denied
	EndpointDescriptor bootstrapEp; //< the endpoint for bootstrapping the overlay interface

public:
	JoinReply(
		const SpoVNetID _spovnetid = SpoVNetID::UNSPECIFIED,
		const OverlayParameterSet _param = OverlayParameterSet::DEFAULT,
		bool _joinAllowed = false,
		const EndpointDescriptor _bootstrapEp = EndpointDescriptor::UNSPECIFIED()
	);

	virtual ~JoinReply();

	const SpoVNetID& getSpoVNetID();
	const OverlayParameterSet& getParam();
	bool getJoinAllowed();
	const EndpointDescriptor& getBootstrapEndpoint();
};

}} // ariba::overlay

sznBeginDefault( ariba::overlay::JoinReply, X ) {
	uint8_t ja = joinAllowed;
	X && &spovnetid && param && bootstrapEp && ja;
	if (X.isDeserializer()) joinAllowed = ja;
} sznEnd();

#endif // JOIN_REPLY_H__
