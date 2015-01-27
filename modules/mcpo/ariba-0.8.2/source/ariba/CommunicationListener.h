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
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OF TELEMATICS OR
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

#ifndef COMMUNICATIONLISTENER_H_
#define COMMUNICATIONLISTENER_H_

#include "Message.h"
#include "Identifiers.h"
#include "LinkProperties.h"
#include "DataMessage.h"

namespace ariba {

// forward decl
namespace overlay {
	class BaseOverlay;
}

/** \addtogroup public
 * @{
 * Listener for communication events on links.
 */
class CommunicationListener {

	friend class ariba::overlay::BaseOverlay;
	friend class Node;

public:
	static CommunicationListener DEFAULT; //< default implementation

protected:

	/**
	 * Construct a communication listener
	 */
	CommunicationListener();

	/**
	 * Destruct a communication listener
	 */
	virtual ~CommunicationListener();

	// --- link events ---

	/**
	 * Event called when a link goes up
	 * @param lnk The id of the link
	 * @param remote The remote node where the link ends
	 */
	virtual void onLinkUp(const LinkID& lnk, const NodeID& remote);

	/**
	 * Event called when a link goes down
	 * @param lnk The id of the link
	 * @param remote The remote node where the link ends
	 */
	virtual void onLinkDown(const LinkID& lnk, const NodeID& remote);

	/**
	 * Event called when a link has changed,
	 * e.g. through mobility
	 * @param lnk The id of the link
	 * @param remote The remote node where the link ends
	 */
	virtual void onLinkChanged(const LinkID& lnk, const NodeID& remote);

	/**
	 * Event called when a link has failed
	 * @param lnk The id of the link
	 * @param remote The remote node where the link ends
	 */
	virtual void onLinkFail(const LinkID& lnk, const NodeID& remote);

	/**
	 * Request from remote node to open up a link
	 * @param remote The remote node that requests the new link
	 */
	virtual bool onLinkRequest(const NodeID& remote);

	// --- general receive method ---

	/**
	 * Called when a message is incoming
	 * @param msg The data message that is received
	 * @param remote The remote node that sent the message
	 * @param lnk The link id of the link where the message is received
	 */
	virtual void onMessage(const DataMessage& msg, const NodeID& remote,
			const LinkID& lnk = LinkID::UNSPECIFIED);

	// --- dht functionality ---

	/**
	 * Called when a key has been resolved in the DHT
	 * @param key The key that was requested
	 * @param value the data items the key was resolved to
	 */
	virtual void onKeyValue( const Data& key, const vector<Data>& value );

};

} // namespace ariba

/** @} */

#endif /* COMMUNICATIONLISTENER_H_ */
