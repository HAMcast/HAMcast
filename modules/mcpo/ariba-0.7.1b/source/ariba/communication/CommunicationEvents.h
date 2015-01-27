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

#ifndef __COMMUNICATION_EVENTS_H
#define __COMMUNICATION_EVENTS_H

#include "ariba/utility/types/LinkID.h"
#include "ariba/utility/types/QoSParameterSet.h"
#include "ariba/utility/addressing/addressing.hpp"

namespace ariba {
namespace communication {

using ariba::utility::LinkID;
using ariba::utility::QoSParameterSet;
using namespace ariba::addressing;

class CommunicationEvents {
	friend class BaseCommunication;

public:
	CommunicationEvents();
	virtual ~CommunicationEvents();

protected:

	/**
	 * This method is called when a link request is received.
	 * If this method returns true, a the link request is processed
	 * and a new link will be established.
	 *
	 * @param id The provisional link identifier of the new link
	 * @return True, if the link should be established
	 */
	virtual bool onLinkRequest(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * This method is called when a link is established and can
	 * be used to send messages.
	 *
	 * @param id The link id of the established link
	 */
	virtual void onLinkUp(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * This method is called when a link is dropped.
	 *
	 * @param id The link identifier of the dropped link
	 */
	virtual void onLinkDown(const LinkID& id, const address_v* local,
		const address_v* remote);

	/**
	 * This method is called when a link has been changed because
	 * of interface failure, mobility, etc. in this case messages
	 * can still be sent over the link -- connectivity is still provided
	 * with different link properties.
	 *
	 * @param id The link identifier of the changed link
	 */
	virtual void onLinkChanged(const LinkID& id,
		const address_v* oldlocal,  const address_v* newlocal,
		const address_v* oldremote, const address_v* newremote
	);

	virtual void onLinkFail(const LinkID& id, const address_v* local,
		const address_v* remote);

	virtual void onLinkQoSChanged(const LinkID& id, const address_v* local,
		const address_v* remote, const QoSParameterSet& qos);
};

}} // namespace ariba, communication

#endif //__COMMUNICATION_EVENTS_H
