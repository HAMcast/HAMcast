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

/**
 * @file MCPOCluster.cc
 * @author Christian Huebsch
 */


#include "MCPOCluster.h"

namespace ariba {
namespace services {
namespace mcpo {

/*******************************************************************************
 * Constructor
 ******************************************************************************/
MCPOCluster::MCPOCluster()
{

	setLeader(NodeID::UNSPECIFIED);

} // MCPOCluster

/*******************************************************************************
 * Adds member to a cluster
 * @param member Peer to add
 ******************************************************************************/
void MCPOCluster::add( const NodeID& member ) {

	cluster.insert(member);

} // add


/*******************************************************************************
 * Clears all cluster contents
 ******************************************************************************/
void MCPOCluster::clear() {

	cluster.clear();
	setLeader(NodeID::UNSPECIFIED);

} // clear


/*******************************************************************************
 * Checks is cluster contains specific member
 * @param member Peer to check for
 * @return boolean that indicates contain relation
 ******************************************************************************/
bool MCPOCluster::contains( const NodeID& member ) {

    ConstClusterIterator it = cluster.find(member);

	if (it != cluster.end())
		return true;

	return false;

} // contains


/*******************************************************************************
 * Returns current cluster size
 * @return Cluster size
 ******************************************************************************/
int MCPOCluster::getSize() {

	return cluster.size();

} // getSize


/*******************************************************************************
 * Returns nodeid of specific member
 * @param i index of member
 * @return NodeID of member
 ******************************************************************************/
const NodeID& MCPOCluster::get( int i ) {

	ConstClusterIterator it = cluster.begin();

	for(int j = 0; j < i; j++)
		it++;

	return *it;

} // get


/*******************************************************************************
 * Removes member from cluster
 * @param member Peer to remove
 ******************************************************************************/
void MCPOCluster::remove(const NodeID& member) {

	cluster.erase(member);

} // remove


/*******************************************************************************
 * Sets leader for this cluster
 * @param leader Cluster leader
 ******************************************************************************/
void MCPOCluster::setLeader(const NodeID& leader) {

	this->leader = leader;

} // setLeader


/*******************************************************************************
 * Returns cluster leader
 * @return Cluster leader
 ******************************************************************************/
const NodeID& MCPOCluster::getLeader() {

	return leader;

} // getLeader


/*******************************************************************************
 * Returns last point in time when leader transfer was received -> robustness enhancement
 * @return Last point in time when leader transfer was received
 ******************************************************************************/
unsigned long MCPOCluster::get_Last_LT()
{

	return last_LT;

} // get_Last_LT


/*******************************************************************************
 * Sets last point in time when leader transfer was received -> robustness enhancement
 ******************************************************************************/
void MCPOCluster::set_Last_LT() {

	last_LT = ariba::utility::Helper::getElapsedMillis();

} // set_Last_LT

}}}
