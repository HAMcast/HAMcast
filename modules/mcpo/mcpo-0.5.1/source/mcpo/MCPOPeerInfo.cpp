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
 * @file NICEPeerInfo.cc
 * @author Christian Huebsch
 */


#include "MCPOPeerInfo.h"

namespace ariba {
namespace services {
namespace mcpo {


/******************************************************************************
 * Constructor
 * @param _parent Pointer to parent class
 *****************************************************************************/
MCPOPeerInfo::MCPOPeerInfo(MCPO* _parent)
	: parent (_parent)
{

	distance_estimation_start = -1;
	distance = -1;
	last_sent_HB = 1;
	last_recv_HB = 0;
	backHBPointer = false;
	last_HB_arrival = 0;

	activity = ariba::utility::Helper::getElapsedMillis();

} // MCPOPeerInfo


/******************************************************************************
 * Destructor
 *****************************************************************************/
MCPOPeerInfo::~MCPOPeerInfo()
{

} // ~MCPOPeerInfo


/******************************************************************************
 * Sets time for start of distance estimation
 * @param value Point in time
 *****************************************************************************/
void MCPOPeerInfo::set_distance_estimation_start(long value)
{

	distance_estimation_start = value;

} // set_distance_estimation_start


/******************************************************************************
 * Returns time for start of distance estimation
 * @return Time for start of distance estimation
 *****************************************************************************/
long MCPOPeerInfo::getDES()
{

	return distance_estimation_start;

} // getDES


/******************************************************************************
 * Sets distance to this peers
 * @param Distance to this peers
 *****************************************************************************/
void MCPOPeerInfo::set_distance(long value)
{

	distance = value;

} // set_distance


/******************************************************************************
 * Returns distance to this peers
 * @return Distance to this peers
 *****************************************************************************/
long MCPOPeerInfo::get_distance()
{

	return distance;

} // get_distance


/******************************************************************************
 * Updates distance from this peers to another
 * @param member Other peer to set distance to
 * @param distance Distance from this peer to the other
 *****************************************************************************/
void MCPOPeerInfo::updateDistance(NodeID member, long distance)
{
	//get member out of map
	std::map<NodeID, long>::iterator it = distanceTable.find(member);

	if (it != distanceTable.end()) {

		it->second = distance;

	} else {

		distanceTable.insert(std::make_pair(member, distance));

	}


} // updateDistance


/******************************************************************************
 * Returns distance from this peers to another
 * @param member Other peer to get distance to
 * @return distance Distance from this peer to the other
 *****************************************************************************/
long MCPOPeerInfo::getDistanceTo(NodeID member)
{


	//get member out of map
	std::map<NodeID, long>::iterator it = distanceTable.find(member);

	if (it != distanceTable.end()) {


		return it->second;

	} else {


		return -1;

	}

} // getDistanceTo


/******************************************************************************
 * Returns the last sent Heartbeat sequence number
 * @return Last sent Heartbeat sequence number
 *****************************************************************************/
unsigned int MCPOPeerInfo::get_last_sent_HB()
{

	return last_sent_HB;

} // get_last_sent_HB


/******************************************************************************
 * Sets the last sent Heartbeat sequence number
 * @param seqNo Last sent Heartbeat sequence number
 *****************************************************************************/
void MCPOPeerInfo::set_last_sent_HB(unsigned int seqNo)
{

	last_sent_HB = seqNo;

} // set_last_sent_HB


/******************************************************************************
 * Returns the last received Heartbeat sequence number from this node
 * @return The last received Heartbeat sequence number from this node
 *****************************************************************************/
unsigned int MCPOPeerInfo::get_last_recv_HB()
{

	return last_recv_HB;

} // get_last_recv_HB


/******************************************************************************
 * Sets the last received Heartbeat sequence number from this node
 * @param seqNo The last received Heartbeat sequence number from this node
 *****************************************************************************/
void MCPOPeerInfo::set_last_recv_HB(unsigned int seqNo)
{

	last_recv_HB = seqNo;

} // set_last_recv_HB


/******************************************************************************
 * Returns the last received Heartbeat timestamp
 * @return The last received Heartbeat timestamp
 *****************************************************************************/
long MCPOPeerInfo::get_last_HB_arrival()
{

	return last_HB_arrival;

} // get_last_HB_arrival


/******************************************************************************
 * Sets the last received Heartbeat timestamp
 * @param arrival The last received Heartbeat timestamp
 *****************************************************************************/
void MCPOPeerInfo::set_last_HB_arrival(long arrival)
{

	last_HB_arrival = arrival;

} // set_last_HB_arrival


/******************************************************************************
 * Returns the backHBPointer --> robustness
 * @return The backHBPointer
 *****************************************************************************/
bool MCPOPeerInfo::get_backHBPointer()
{

	return backHBPointer;

} // get_backHBPointer


/******************************************************************************
 * Sets the backHBPointer --> robustness
 * @param _backHBPointer The backHBPointer
 *****************************************************************************/
void MCPOPeerInfo::set_backHBPointer(bool _backHBPointer)
{

	backHBPointer = _backHBPointer;

} // set_backHBPointer


/******************************************************************************
 * Sets the backHBPointer --> robustness
 * @param _backHBPointer The backHBPointer
 * @param seqNo The sequence number
 * @param time A timestamp
 *****************************************************************************/
void MCPOPeerInfo::set_backHB(bool backHBPointer, unsigned int seqNo, long time)
{

	backHB[backHBPointer].first = seqNo;
	backHB[backHBPointer].second = time;

} // set_backHB


/******************************************************************************
 * Returns the timestamp of a given backHB--> robustness
 * @param seqNo Sequence number
 * @return Timestamp
 *****************************************************************************/
long MCPOPeerInfo::get_backHB(unsigned int seqNo)
{

	long time = 0;

	if (backHB[0].first == seqNo)
		time = backHB[0].second;
	else if (backHB[1].first == seqNo)
		time = backHB[1].second;

	return time;

} // get_backHB


/******************************************************************************
 * Returns the sequence number of a given backHB--> robustness
 * @param index Boolean flag
 * @return Sequence number of backHB
 *****************************************************************************/
unsigned int MCPOPeerInfo::get_backHB_seqNo(bool index)
{

	return backHB[index].first;

} // get_backHB_seqNo


/******************************************************************************
 * Marks a peer as stil active
 *****************************************************************************/
void MCPOPeerInfo::touch()
{

	activity = ariba::utility::Helper::getElapsedMillis();

} // touch


/******************************************************************************
 * Returns the point in time when peer was indicated active last
 *****************************************************************************/
long MCPOPeerInfo::getActivity()
{

	return activity;

} // getActivity


}}} //namespace


