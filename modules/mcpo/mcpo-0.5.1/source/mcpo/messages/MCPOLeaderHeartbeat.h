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

#ifndef MCPO_LEADERHEARTBEAT_MSG_H__
#define MCPO_LEADERHEARTBEAT_MSG_H__

#include "ariba/ariba.h"

#include <vector>

namespace ariba {
namespace services {
namespace mcpo {

using_serialization;

/******************************************************************************
 * Periodic Heartbeat Message from Cluster Leaders
 *****************************************************************************/
class MCPOLeaderHeartbeatMsg : public Message {
	VSERIALIZEABLE;

public:

	/**
	 * Constructor
	 **/
	MCPOLeaderHeartbeatMsg();

	/** Destructor **/
	virtual ~MCPOLeaderHeartbeatMsg();

	/** Vector of distances, used for cluster maintenance **/
	typedef vector<NodeID*> MemberList;

	/** Vector of distances, used for cluster maintenance **/
	typedef vector<uint32_t> DistanceList;

	/**
	 * Inserts neighbor NodeID into vector
	 * @param neighbor NodeID of neighbor in cluster
	 **/
	void insert( NodeID* member );

	/**
	 * Returns list of neighbors as vector
	 * @return Vector of neighbor NodeIDs
	 **/
	MemberList getList();

	/**
	 * Inserts distance into vector
	 * @param distance to neighbor in cluster
	 **/
	void insertDistance( long distance );

	/**
	 * Returns list of distances as vector
	 * @return Vector of distances
	 **/
	DistanceList getDistanceList();

	/**
	 * Inserts neighbor NodeID into vector
	 * @param neighbor NodeID of neighbor in cluster
	 **/
	void insertSc( NodeID* scmember );

	/**
	 * Returns list of neighbors as vector
	 * @return Vector of neighbor NodeIDs
	 **/
	MemberList getScList();

	void setScLeader( NodeID _scLeader );

	NodeID getScLeader();

	void setSeqNo( uint32_t _seqNo );

	uint32_t getSeqNo();

	void setSeqRspNo( uint32_t _seqRspNo );

	uint32_t getSeqRspNo();

	void setHb_delay( uint32_t _hb_delay );

	uint32_t getHb_delay();


private:

	/** Sequence Number **/
	uint32_t seqNo;

	/** Response to Sequence Number **/
	uint32_t seqRspNo;

	/** Delay since last HB **/
	uint32_t hb_delay;

	/** Vector of member NodeIDs **/
	vector<NodeID*> members;

	/** Vector of distances **/
	vector<uint32_t> distances;

	/** NodeID of supercluster leader **/
	NodeID scLeader;

	/** Vector of sc member NodeIDs **/
	vector<NodeID*> scMembers;

};

}}} // ariba::services::mcpo

/** Serialization of Heartbeat message **/
sznBeginDefault( ariba::services::mcpo::MCPOLeaderHeartbeatMsg, X ) {

	X && seqNo && seqRspNo && hb_delay && &scLeader;

	uint16_t len1 = X.isSerializer() ? members.size() : 0;
	X && len1;
	if (X.isDeserializer()) members.resize(len1);
	for (int i=0; i<len1; i++) X && VO(members[i]);

	uint16_t len2 = X.isSerializer() ? distances.size() : 0;
	X && len2;
	if (X.isDeserializer()) distances.resize(len2);
	for (int i=0; i<len2; i++) X && distances[i];

	uint16_t len3 = X.isSerializer() ? scMembers.size() : 0;
	X && len3;
	if (X.isDeserializer()) scMembers.resize(len3);
	for (int i=0; i<len3; i++) X && VO(scMembers[i]);

} sznEnd();

#endif // MCPO_LEADERHEARTBEAT_MSG_H__
