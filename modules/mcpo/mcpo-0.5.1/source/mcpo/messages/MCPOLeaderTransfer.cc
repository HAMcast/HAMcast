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

#include "MCPOLeaderTransfer.h"

namespace ariba {
namespace services {
namespace mcpo {

vsznDefault(MCPOLeaderTransferMsg);

/******************************************************************************
 * Constructor
 * @param _layer Cluster layer
 * @param _newLeader NodeID of cluster leader
 * @param _scLeader NodeID of super cluster leader
 *****************************************************************************/
MCPOLeaderTransferMsg::MCPOLeaderTransferMsg( 	int _layer,
												NodeID _newLeader,
												NodeID _scLeader )
	: layer( _layer ), newLeader( _newLeader ), scLeader( _scLeader ) {

} // MCPOLeaderTransferMsg


/******************************************************************************
 * Destructor
 *****************************************************************************/
MCPOLeaderTransferMsg::~MCPOLeaderTransferMsg(){

} // ~MCPOLeaderTransferMsg


/******************************************************************************
 * Sets the Cluster layer
 * @param _layer Cluster layer
 *****************************************************************************/
void MCPOLeaderTransferMsg::setLayer( int _layer ) {

	layer = _layer;

} // setLayer


/******************************************************************************
 * Returns cluster layer
 * @return Cluster layer
 *****************************************************************************/
int MCPOLeaderTransferMsg::getLayer() {

	return layer;

} // getLayer


/******************************************************************************
 * Inserts member NodeID into member vector
 * @param member Member NodeID to add
 *****************************************************************************/
void MCPOLeaderTransferMsg::insertMember( NodeID* member ) {

	members.push_back( member );

} // insertMember


/******************************************************************************
 * Returns Vector of member NodeIDs
 * @return Vector of NodeIDs
 *****************************************************************************/
MCPOLeaderTransferMsg::MemberList MCPOLeaderTransferMsg::getMembers() {

	return members;

} // getMembers


/******************************************************************************
 * Inserts member NodeID into SC member vector
 * @param member Member NodeID to add
 *****************************************************************************/
void MCPOLeaderTransferMsg::insertSCMember( NodeID* member ) {

	scmembers.push_back( member );

} // insertSCMember


/******************************************************************************
 * Returns Vector of SC member NodeIDs
 * @return Vector of NodeIDs
 *****************************************************************************/
MCPOLeaderTransferMsg::MemberList MCPOLeaderTransferMsg::getSCMembers() {

	return scmembers;

} // getSCMembers


/******************************************************************************
 * Sets cluster leader NodeID
 * @param _leader NodeID of cluster leader
 *****************************************************************************/
void MCPOLeaderTransferMsg::setNewLeader( NodeID _leader ) {

	newLeader = _leader;

} // setNewLeader


/******************************************************************************
 * Returns cluster leader NodeID
 * @return NodeID of cluster leader
 *****************************************************************************/
NodeID MCPOLeaderTransferMsg::getNewLeader() {

	return newLeader;

} // getNewLeader


/******************************************************************************
 * Sets SC cluster leader NodeID
 * @param _leader NodeID of SC cluster leader
 *****************************************************************************/
void MCPOLeaderTransferMsg::setSCLeader( NodeID _leader ) {

	scLeader = _leader;

} // setSCLeader


/******************************************************************************
 * Returns SC cluster leader NodeID
 * @return NodeID of SC cluster leader
 *****************************************************************************/
NodeID MCPOLeaderTransferMsg::getSCLeader() {

	return scLeader;

} // getSCLeader


}}} // ariba::service::mcpo

