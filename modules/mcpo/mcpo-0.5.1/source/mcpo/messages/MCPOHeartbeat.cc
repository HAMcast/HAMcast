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

#include "MCPOHeartbeat.h"

namespace ariba {
namespace services {
namespace mcpo {

vsznDefault(MCPOHeartbeatMsg);

/******************************************************************************
 * Constructor
 *****************************************************************************/
MCPOHeartbeatMsg::MCPOHeartbeatMsg() {

} // MCPOHeartbeatMsg


/******************************************************************************
 * Destructor
 *****************************************************************************/
MCPOHeartbeatMsg::~MCPOHeartbeatMsg(){

} // ~MCPOHeartbeatMsg


/******************************************************************************
 * Set Sequence Number
 * @param _seqNo Sequence number
 *****************************************************************************/
void MCPOHeartbeatMsg::setSeqNo( uint32_t _seqNo ){

	seqNo = _seqNo;

} // setSeqNo


/******************************************************************************
 * Get Sequence Number
 * @result Sequence number
 *****************************************************************************/
uint32_t MCPOHeartbeatMsg::getSeqNo(){

	return seqNo;

} // getSeqNo


/******************************************************************************
 * Set Response Sequence Number
 * @param response sequence number
 *****************************************************************************/
void MCPOHeartbeatMsg::setSeqRspNo( uint32_t _seqRspNo ){

	seqRspNo = _seqRspNo;

} // setSeqRspNo


/******************************************************************************
 * Get Response Sequence Number
 * @result Response sequence number
 *****************************************************************************/
uint32_t MCPOHeartbeatMsg::getSeqRspNo(){

	return seqRspNo;

} // getSeqrspNo


/******************************************************************************
 * Set Heartbeat Delay
 * @param _hb_delay Heartbeat delay
 *****************************************************************************/
void MCPOHeartbeatMsg::setHb_delay( uint32_t _hb_delay ) {

	hb_delay = _hb_delay;

} // setHb_Delay


/******************************************************************************
 * Get Heartbeat Delay
 * @result Heartbeat delay
 *****************************************************************************/
uint32_t MCPOHeartbeatMsg::getHb_delay() {

	return hb_delay;

} // getHb_Delay


/******************************************************************************
 * Inserts distance into vector
 * @param distance to neighbor in cluster
 ******************************************************************************/
void MCPOHeartbeatMsg::insertDistance( uint32_t distance ) {

	distances.push_back( distance+1 );

} // insertDistance


/******************************************************************************
 * Returns list of distances as vector
 * @return Vector of distances
 ******************************************************************************/
MCPOHeartbeatMsg::DistanceList MCPOHeartbeatMsg::getDistanceList() {

	return distances;

} // getDistanceList


/******************************************************************************
 * Inserts neighbor NodeID into vector
 * @param neighbor NodeID of neighbor in cluster
 *****************************************************************************/
void MCPOHeartbeatMsg::insert( NodeID* member ){

	members.push_back( member );

} // insert


/******************************************************************************
 * Returns list of neighbors as vector
 * @return Vector of neighbor NodeIDs
 *****************************************************************************/
MCPOHeartbeatMsg::MemberList MCPOHeartbeatMsg::getList() {

	return members;

} // getList


}}} // ariba::service::mcpo
