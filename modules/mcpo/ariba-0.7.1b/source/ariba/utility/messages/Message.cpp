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

#include "Message.h"
#include "ariba/utility/serialization/Data.hpp"
#include "ariba/utility/serialization/DataStream.hpp"

NAMESPACE_BEGIN

vsznDefault(Message); // virtual serialization

/*
Message::Message() :
	root(), payload(), srcAddr(NULL), destAddr(NULL), ctrlInfo(NULL) {
}
*/

Message::~Message() {
	dropPayload();
//	if ( srcAddr != NULL) delete srcAddr;
//	if ( destAddr != NULL) delete destAddr;
//	if ( ctrlInfo != NULL ) delete ctrlInfo;
}

/* Returns a short human-readable description of this message
 * @return A short human-readable description of this message
 */
const char* Message::getDescription() const {
	return "<DEFAULT_MESSAGE>";
}

/* Returns a return message, that can be used to send a message
 * back to the recipient or NULL if no message can be returned.
 * The default implementation returns NULL.
 * @return Return message.
 */
Message* Message::createReturnMessage() const {
	return NULL;
}

std::string Message::toString() const {
	std::stringstream buffer;
	buffer << *this;
	return buffer.str();
}

NAMESPACE_END

std::ostream& operator<<(std::ostream& stream, const ariba::utility::Message& msg ) {
	using_serialization;
	stream << "msg(type=" << typeid(msg).name();
	stream << "len=" << (data_length(&msg)/8) << ",";
	Data data = data_serialize(&msg);
	stream << ",data=" << data;
	data.release();
	stream << ")";
	return stream;
}

