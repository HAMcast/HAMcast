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

#ifndef MCPO_LEADER_TRANSFER_MSG_H__
#define MCPO_LEADER_TRANSFER_MSG_H__

#include "ariba/ariba.h"

#include <vector>

namespace ariba {
namespace services {
namespace mcpo {

using_serialization;
using std::vector;

/******************************************************************************
 * Message Class for Leader Transfer Indication
 *****************************************************************************/
class MCPOLeaderTransferMsg : public Message {
	VSERIALIZEABLE;

public:

	/**
	 * Constructor
	 * @param _layer Cluster layer
	 * @param _newLeader NodeID of cluster leader
	 * @param _scLeader NodeID of super cluster leader
	 **/
	MCPOLeaderTransferMsg( 	int _layer = 0,
							NodeID _newLeader = NodeID::UNSPECIFIED_KEY,
							NodeID _scLeader = NodeID::UNSPECIFIED_KEY );

	/** Destructor **/
	virtual ~MCPOLeaderTransferMsg();

	/** Vector of member NodeIDs**/
	typedef vector<NodeID*> MemberList;

	/**
	 * Sets the Cluster layer
	 * @param _layer Cluster layer
	 **/
	void setLayer( int _layer );

	/**
	 * Returns cluster layer
	 * @return Cluster layer
	 **/
	int getLayer();

	/**
	 * Inserts member NodeID into member vector
	 * @param member Member NodeID to add
	 **/
	void insertMember( NodeID* member );

	/**
	 * Returns Vector of member NodeIDs
	 * @return Vector of NodeIDs
	 **/
	MemberList getMembers();

	/**
	 * Inserts member NodeID into SC member vector
	 * @param member Member NodeID to add
	 **/
	void insertSCMember( NodeID* member );

	/**
	 * Returns Vector of SC member NodeIDs
	 * @return Vector of NodeIDs
	 **/
	MemberList getSCMembers();

	/**
	 * Sets cluster leader NodeID
	 * @param _leader NodeID of cluster leader
	 **/
	void setNewLeader( NodeID _leader );

	/**
	 * Returns cluster leader NodeID
	 * @return NodeID of cluster leader
	 **/
	NodeID getNewLeader();

	/**
	 * Sets SC cluster leader NodeID
	 * @param _leader NodeID of SC cluster leader
	 **/
	void setSCLeader( NodeID _leader );

	/**
	 * Returns SC cluster leader NodeID
	 * @return NodeID of SC cluster leader
	 **/
	NodeID getSCLeader();

private:

	/** Cluster layer **/
	uint8_t layer;

	/** NodeID of cluster leader **/
	NodeID newLeader;

	/** NodeID of SC cluster leader **/
	NodeID scLeader;

	/** Vector of cluster members **/
	vector<NodeID*> members;

	/** Vector of SC cluster members **/
	vector<NodeID*> scmembers;

};

}}} // ariba::services::mcpo

/** Serialization of Leader Transfer Message **/
sznBeginDefault( ariba::services::mcpo::MCPOLeaderTransferMsg, X ) {

	X && layer && &newLeader && &scLeader;
	uint16_t len1 = X.isSerializer() ? members.size() : 0;
	X && len1;
	if (X.isDeserializer()) members.resize(len1);
	for (int i=0; i<len1; i++) X && VO(members[i]);
	uint16_t len2 = X.isSerializer() ? scmembers.size() : 0;
	X && len2;
	if (X.isDeserializer()) scmembers.resize(len2);
	for (int i=0; i<len2; i++) X && VO(scmembers[i]);

} sznEnd();

#endif // MCPO_LEADER_TRANSFER_MSG_H__
