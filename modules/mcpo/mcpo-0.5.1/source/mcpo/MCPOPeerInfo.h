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
 * @file NICEPeerInfo.h
 * @author Christian Huebsch
 */

#ifndef __MCPOPEERINFO_H_
#define __MCPOPEERINFO_H_

#include "ariba/ariba.h"
#include <set>
#include "MCPO.h"
using std::set;

namespace ariba {
namespace services {
namespace mcpo {

typedef std::pair<unsigned int, long> HeartbeatEvaluator;

class MCPOPeerInfo {

	public:

		MCPOPeerInfo(MCPO* _parent);
		~MCPOPeerInfo();

		void set_distance_estimation_start(long value);
		long getDES();
		void set_distance(long value);
		long get_distance();

		void updateDistance(NodeID member, long distance);
		long getDistanceTo(NodeID member);

		unsigned int get_last_sent_HB();
		void set_last_sent_HB(unsigned int seqNo);

		unsigned int get_last_recv_HB();
		void set_last_recv_HB(unsigned int seqNo);

		long get_last_HB_arrival();
		void set_last_HB_arrival(long arrival);

		bool get_backHBPointer();
		void set_backHBPointer(bool _backHBPointer);

		void set_backHB(bool backHBPointer, unsigned int seqNo, long time);
		long get_backHB(unsigned int seqNo);
		unsigned int get_backHB_seqNo(bool index);

		void touch();
		long getActivity();

	private:

		MCPO* parent;
		long distance_estimation_start;
		long distance;
		std::map<NodeID, long> distanceTable;

		long activity;

		HeartbeatEvaluator backHB[2];
		bool backHBPointer;

		unsigned int last_sent_HB;
		unsigned int last_recv_HB;
		long last_HB_arrival;


}; // MCPOPeerInfo

}}} //namespace

#endif /* _MCPOEERINFO_H_ */
