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

#include "MCPOLeaderHeartbeat.h"

namespace ariba {
namespace services {
namespace mcpo {

vsznDefault(MCPOLeaderHeartbeatMsg);

/******************************************************************************
 * Constructor
 *****************************************************************************/
MCPOLeaderHeartbeatMsg::MCPOLeaderHeartbeatMsg() {

} // MCPOLeaderHeartbeatMsg


/******************************************************************************
 * Destructor
 *****************************************************************************/
MCPOLeaderHeartbeatMsg::~MCPOLeaderHeartbeatMsg(){

} // ~MCPOLeaderHeartbeatMsg


/******************************************************************************
 * Inserts neighbor NodeID into vector
 * @param neighbor NodeID of neighbor in cluster
 *****************************************************************************/
void MCPOLeaderHeartbeatMsg::insert( NodeID* member ){

	members.push_back( member );

} // insert


/******************************************************************************
 * Returns list of neighbors as vector
 * @return Vector of neighbor NodeIDs
 *****************************************************************************/
MCPOLeaderHeartbeatMsg::MemberList MCPOLeaderHeartbeatMsg::getList() {

	return members;

} // getList


/******************************************************************************
 * Sets the super cluster leader
 * @param scLeader Super cluster leader
 *****************************************************************************/
void MCPOLeaderHeartbeatMsg::setScLeader( NodeID _scLeader ) {

	scLeader = _scLeader;

} // setScLeader


/******************************************************************************
 * Returns super cluster leader
 * @return NodeID of super cluster leader
 *****************************************************************************/
NodeID MCPOLeaderHeartbeatMsg::getScLeader() {

	return scLeader;

} // getScLeader


/******************************************************************************
 * Inserts sc member NodeID into vector
 * @param sc member
 *****************************************************************************/
void MCPOLeaderHeartbeatMsg::insertSc( NodeID* scmember ) {

	scMembers.push_back( scmember );

} // insertSc


/*****************************************************************************
 * Returns list of super cluster members
 * @return Vector of NodeIDs
 *****************************************************************************/
MCPOLeaderHeartbeatMsg::MemberList MCPOLeaderHeartbeatMsg::getScList() {

	return scMembers;

} // getScList


/****************************************************************************
 * Inserts distance into vector
 * @param distance to neighbor in cluster
 ***************************************************************************/
void MCPOLeaderHeartbeatMsg::insertDistance( long distance ) {

	distances.push_back( distance+1 );

} // insertDistance


/****************************************************************************
 * Returns list of distances as vector
 * @return Vector of distances
 ****************************************************************************/
MCPOLeaderHeartbeatMsg::DistanceList MCPOLeaderHeartbeatMsg::getDistanceList() {

	return distances;

} // getDistanceList


/******************************************************************************
 * Set Sequence Number
 * @param _seqNo Sequence number to set
 *****************************************************************************/
void MCPOLeaderHeartbeatMsg::setSeqNo( uint32_t _seqNo ){

	seqNo = _seqNo;

} // setSeqNo


/******************************************************************************
 * Get Sequence Number
 * @return Sequence number
 *****************************************************************************/
uint32_t MCPOLeaderHeartbeatMsg::getSeqNo(){

	return seqNo;

} // getSeqNo


/******************************************************************************
 * Set Response Sequence Number
 * @param _seqRspNo Sequence number of response
 *****************************************************************************/
void MCPOLeaderHeartbeatMsg::setSeqRspNo( uint32_t _seqRspNo ){

	seqRspNo = _seqRspNo;

} // setSeqRspNo


/******************************************************************************
 * Get Response Sequence Number
 * @return Response Sequence number
 *****************************************************************************/
uint32_t MCPOLeaderHeartbeatMsg::getSeqRspNo(){

	return seqRspNo;

} // getSeqRspNo


/******************************************************************************
 * Sets the Heartbeat delay
 * @param _hb_delay The Hearbeat delay
 *****************************************************************************/
void MCPOLeaderHeartbeatMsg::setHb_delay( uint32_t _hb_delay ) {

	hb_delay = _hb_delay;

} // setHb_Delay


/******************************************************************************
 * Returns the Heartbeat delay
 * @return Heartbeat delay
 *****************************************************************************/
uint32_t MCPOLeaderHeartbeatMsg::getHb_delay() {

	return hb_delay;

} // getHb_Delay


}}} // ariba::service::mcpo
