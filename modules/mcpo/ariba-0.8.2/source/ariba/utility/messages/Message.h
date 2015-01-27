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

#ifndef MESSAGE_H_
#define MESSAGE_H_

// library includes
#include<new>
#include<string>
#include<iostream>
#include<boost/shared_array.hpp>

// forward declaration
#include "_namespace.h"
NAMESPACE_BEGIN
class Message;
typedef size_t seqnum_t;
NAMESPACE_END

// library includes
#include <cassert>
#include<boost/shared_array.hpp>
#include<boost/cstdint.hpp>

// common includes
#include "ariba/utility/types/Address.h"
#include "ariba/utility/serialization.h"

std::ostream& operator<<(std::ostream& stream, const ariba::utility::Message& msg );

#include "_namespace.h"
NAMESPACE_BEGIN

using_serialization;
using ariba::utility::Address;
using boost::shared_array;

/**
 * This class implements an abstract message format.
 *
 * @author Sebastian Mies
 */
class Message: public VSerializeable {
	VSERIALIZEABLE;

protected:
	friend std::ostream& ::operator<<(std::ostream& stream, const ariba::utility::Message& msg );

	// root binary data
	shared_array<uint8_t> root;

	// payload
	bool releasePayload;
	Data payload; //< messages binary data

	// addresses and control info
	const Address* srcAddr;
	const Address* destAddr;

public:
	/**
	 * Constructor initializing name of message to the given one
	 */
	inline Message() :
		root(), releasePayload(true), payload(), srcAddr(NULL),destAddr(NULL) {
	}

	/**
	 * Constructs a new "root" message by copying the data described by
	 * data.
	 */
	explicit inline Message( const Data& data ) :
		releasePayload(true), srcAddr(NULL),destAddr(NULL) {
		this->payload = data.clone();
//		this->root = shared_array<uint8_t>((uint8_t*)data.getBuffer());
	}

	inline void dropPayload() {
		if (this->releasePayload) payload.release();
	}

	inline void setReleasePayload( bool release ) {
		this->releasePayload = release;
	}

	inline Data getPayload() const {
		return payload;
	}

	inline void setPayload( const Data& payload ) {
		this->payload = payload;
	}

	/**
	 * Default destructor.
	 */
	virtual ~Message();

	std::string toString() const;

	/**
	 * Sets the destination address
	 *
	 * @param An abstract address representation
	 */
	inline void setDestinationAddress(const Address* addr) {
		destAddr = addr;
	}

	/**
	 * Returns the optional abstract destination address or NULL
	 *
	 * @return the abstract destination address
	 */
	inline const Address* getDestinationAddress() const {
		return destAddr;
	}

	/**
	 * Set the source address of the message
	 *
	 * @param addr The abstract source address
	 */
	inline void setSourceAddress(const Address* addr) {
		srcAddr = addr;
	}

	/**
	 * Returns the optional abstract source address or NULL
	 *
	 * @return The abstract source address
	 */
	inline const Address* getSourceAddress() const {
		return srcAddr;
	}

	/**
	 * Returns a short human-readable description of this message
	 *
	 * @return A short human-readable description of this message
	 */
	virtual const char* getDescription() const;

	/**
	 * Returns a return message, that can be used to send a message
	 * back to the recipient or NULL if no message can be returned.
	 * The default implementation returns NULL.
	 *
	 * @return Return message.
	 */
	virtual Message* createReturnMessage() const;

	/**
	 * Encapsulate a message into the payload.
	 *
	 * @param message The message to be encapsulated.
	 */
	inline void encapsulate( Message* message, int variant = DEFAULT_V ) {
		if ( !payload.isUnspecified() ) throw "Error: Message already encapsulated";
		payload = data_serialize( message, variant );
		message->dropPayload();
	}

	/**
	 * Decapsulates message. In case the message
	 * has not been deserialized, this method class
	 * serialization to get an object.
	 *
	 * @return The message object or NULL if a deserialization
	 */
	template<class T>
	inline T* decapsulate() {
		if (!payload.isUnspecified()) {
			T* payloadMsg = new T();
			data_deserialize( payloadMsg, payload );
			return payloadMsg;
		}
		return NULL;
	}

	/**
	 * The same as decapsulate, but this function
	 * is used in the samples to make the semantics easier
	 * to understand. The semantics is shown to be: you get
	 * a message and convert it to your type. Not as: you
	 * get a message and have to extract your message from it.
	 */
	template<class T>
	inline T* convert() {
		return decapsulate<T>();
	}

protected:
	/**
	 * This class implements an explicit serializer for
	 * the message's payload.
	 */
	class PayloadSerializer: public ExplicitSerializer {
	private:
		Message* msg;
		size_t len;
	public:
		finline PayloadSerializer(Message* msg, size_t length = ~0) {
			this->msg = msg;
			this->len = length;
		}

		sznMethodBegin(X) {
			if (X.isSerializer()) {
				if (!msg->payload.isUnspecified()) X && msg->payload;
			} else {
				if (msg->payload.isUnspecified()) {
					size_t l = ((len == ~(size_t)0) ? X.getRemainingLength() : len);
					msg->payload = X.getRemainingData(l);
					msg->releasePayload = false;
				}
			}
		}
		sznMethodEnd();
	};

	/**
	 * Returns a serializer of the messages payload/encapsulated
	 * message.
	 *
	 * @param length The length of the payload
	 * @return A explicit payload serializer
	 */
	finline PayloadSerializer Payload( size_t length = ~0 ) {
		return PayloadSerializer( this, length );
	}

};

NAMESPACE_END

sznBeginDefault(ariba::utility::Message, X) {
	X && Payload();
} sznEnd();

#endif /* MESSAGE_H_ */
