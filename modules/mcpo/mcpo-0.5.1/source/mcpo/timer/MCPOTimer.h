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

#ifndef __MCPOTIMER_H_
#define __MCPOTIMER_H_

namespace ariba {
namespace services {
namespace mcpo {

class MCPOTimer;
class BootstrapTimer;
class HeartbeatTimer;
class RefineTimer;
class QueryTimer;
class StructureTimer;
class BackoffTimer;
class RPCollisionTimer;

}}}

#include "ariba/ariba.h"

#include "ariba/utility/system/Timer.h"

#include "../MCPO.h"

using namespace ariba;
using ariba::utility::Timer;

namespace ariba {
namespace services {
namespace mcpo {


/******************************************************************************
 * Provides a generic timer for MCPO
 *****************************************************************************/
class MCPOTimer: public Timer {

	use_logging_h(MCPOTimer)

	public:
		MCPOTimer( MCPO* mcpo );
		virtual ~MCPOTimer();

	protected:
		virtual void eventFunction();
		MCPO* mcpo;

}; // MCPOTimer


/******************************************************************************
 * Provides a timer for bootstrap-timeouts
 *****************************************************************************/
class BootstrapTimer: public MCPOTimer {

	use_logging_h(BootstrapTimer)

	public:
		BootstrapTimer( Node* _node, MCPO* mcpo, MCPOCluster* cluster );
		virtual ~BootstrapTimer();

	protected:
		virtual void eventFunction();

	private:
		Node* node;
		MCPOCluster* cluster;

}; // BootstrapTimer


/******************************************************************************
 * Provides a timer for periodic heartbeat sending
 *****************************************************************************/
class HeartbeatTimer: public MCPOTimer {

	use_logging_h(HeartbeatTimer)

	public:
		HeartbeatTimer( MCPO* mcpo );
		virtual ~HeartbeatTimer();

	protected:
		virtual void eventFunction();

}; // HeartbeatTimer


/******************************************************************************
 * Provides a timer for periodic refinements
 *****************************************************************************/
class RefineTimer: public MCPOTimer {

	use_logging_h(RefineTimer)

	public:
		RefineTimer( MCPO* mcpo );
		virtual ~RefineTimer();

	protected:
		virtual void eventFunction();

}; // RefineTimer


/******************************************************************************
 * Provides a timer for query timeout detection
 *****************************************************************************/
class QueryTimer: public MCPOTimer {

	use_logging_h(QueryTimer)

	public:
		QueryTimer( MCPO* mcpo );
		virtual ~QueryTimer();

	protected:
		virtual void eventFunction();

}; // QueryTimer


/******************************************************************************
 * Provides a timer for partition detection
 *****************************************************************************/
class StructureTimer: public MCPOTimer {

	use_logging_h(StructureTimer)

	public:
		StructureTimer( MCPO* mcpo );
		virtual ~StructureTimer();

	protected:
		virtual void eventFunction();

}; // StructureTimer


/******************************************************************************
 * Provides a timer for delayed start of MCPO
 *****************************************************************************/
class BackoffTimer: public MCPOTimer {

	use_logging_h(BackoffTimer)

	public:
		BackoffTimer( MCPO* mcpo );
		virtual ~BackoffTimer();

	protected:
		virtual void eventFunction();

}; // BackoffTimer


/******************************************************************************
 * Provides a timer for RP collision checking
 *****************************************************************************/
class RPCollisionTimer: public MCPOTimer {

	use_logging_h(RPCollisionTimer)

	public:
		RPCollisionTimer( MCPO* mcpo );
		virtual ~RPCollisionTimer();

	protected:
		virtual void eventFunction();

}; // RPCollisionTimer


}}} // namespace spovnet, services, mcpo

#endif // __MCPOTIMER_H_

