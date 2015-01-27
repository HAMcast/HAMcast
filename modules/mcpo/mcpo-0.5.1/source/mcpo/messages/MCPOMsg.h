// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Karlsruhe Institute of Technology (KIT)
//
// Institute of Telematics
// Karlsruhe Institute of Technology (KIT)
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

#ifndef MCPO_MSG_H__
#define MCPO_MSG_H__

#include "ariba/ariba.h"

using namespace ariba;

namespace ariba {
namespace services {
namespace mcpo {

using_serialization;

class MCPOMsg : public Message {
	VSERIALIZEABLE;

use_logging_h(MCPOMsg);

public:

	/** All message types in MCPO **/
	typedef enum _MESSAGE_TYPE {
		MCPO_QUERY              		= 0,
		MCPO_QUERY_RESPONSE				= 1,
		MCPO_JOIN_CLUSTER				= 2,
		MCPO_HEARTBEAT					= 3,
		MCPO_LEADERHEARTBEAT			= 4,
		MCPO_LEADERTRANSFER 			= 5,
		MCPO_JOINEVAL 					= 6,
		MCPO_JOINEVAL_RESPONSE			= 7,
		MCPO_REMOVE						= 8,
		MCPO_PING_PROBE					= 9,
		MCPO_PING_PROBE_RESPONSE 		= 10,
		MCPO_CLUSTER_MERGE_REQUEST		= 11,
		MCPO_PEER_TEMPORARY				= 12,
		MCPO_PEER_TEMPORARY_RELEASE		= 13,
		MCPO_POLL_RP					= 14,
		MCPO_POLL_RP_RESPONSE			= 15,
		MCPO_INVALID					= 16,
		MCPO_LOOKUP_RP					= 17,
		MCPO_LOOKUP_RP_REPLY			= 18,
		MCPO_APPDATA					= 19,
		MCPO_BROADCAST_RP				= 20,
	} MESSAGE_TYPE;

	/**
	 * Constructor
	 * @param _type Type of message
	 * @param _srcNode NodeID of message sender
	 * @param _layer The intended hierarchy layer
	 * @param _groupID The ntended groupID
	 **/
	MCPOMsg( 	MESSAGE_TYPE _type = MCPO_INVALID,
				const NodeID _srcNode = NodeID::UNSPECIFIED,
				int _layer = 0,
				const ServiceID _groupID = ServiceID::UNSPECIFIED);

	/** Destructor **/
	virtual ~MCPOMsg();

	/**
	 * Checks message type
	 * @param _type Message type to check for
	 * @return True, if message type matches
	 **/
	bool isType(MESSAGE_TYPE _type);

	/**
	 * Return the message type
	 * @return Type of Message
	 **/
	MESSAGE_TYPE getType();

	/**
	 * Returns the source NodeID
	 * @return NodeID of source node
	 **/
	const NodeID& getSrcNode();

	/**
	 * Return the layer
	 * @return Layer
	 **/
	const int16_t getLayer();

	/**
	 * Return the groupID
	 * @return groupID
	 **/
	const ServiceID& getGroupID();
    std::string toString(const uint base=16);
private:

	/** Message type **/
	uint8_t type;

	/** NodeID od source node **/
	NodeID srcNode;

	/** layer in hierarchy **/
	uint16_t layer;

	/** group **/
	ServiceID groupID;
    uint64_t blub;
    std::string me;

};

}}} // spovnet::services::mcpo


/** Serialization of MCPOMsg **/
sznBeginDefault( ariba::services::mcpo::MCPOMsg, X ) {

    X && type && &srcNode && layer && &groupID && blub && Payload();

} sznEnd();


#endif // MCPO_MSG_H__
