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

#include "MCPOTimer.h"

namespace ariba {
namespace services {
namespace mcpo {

use_logging_cpp(MCPOTimer)
use_logging_cpp(BootstrapTimer)
use_logging_cpp(HeartbeatTimer)
use_logging_cpp(RefineTimer)
use_logging_cpp(BackoffTimer)


/******************************************************************************
 * Constructor for MCPOTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
MCPOTimer::MCPOTimer( MCPO* mcpo ) : mcpo (mcpo)
{

} // MCPOTimer


/******************************************************************************
 * Destructor for MCPOTimer
 *****************************************************************************/
MCPOTimer::~MCPOTimer()
{

} // ~MCPOTimer

void MCPOTimer::eventFunction()
{
	logging_error("empty event function");
}


/******************************************************************************
 * Constructor for BootstrapTimer
 * @param bo BaseOverlay
 * @param mcp Calling MCPO Instance
 * @param cluster Layer 0 cluster for node entry
 *****************************************************************************/
BootstrapTimer::BootstrapTimer(	Node* _node,
					MCPO* mcpo,
					MCPOCluster* cluster )
	: MCPOTimer( mcpo ), node (_node), cluster (cluster)
{

} // BootstrapTimer


/******************************************************************************
 * Destructor for BootstrapTimer
 *****************************************************************************/
BootstrapTimer::~BootstrapTimer()
{

} // ~BootstrapTimer


/******************************************************************************
 * EventFunction for Bootstrap Timer, called when no Rendevouz Point is found
 *****************************************************************************/
void BootstrapTimer::eventFunction()
{

	/** No RP response, assuming I am first overlay node */
	mcpo->setRendevouz( node->getNodeId() );

 	mcpo->changeState(BOOTSTRAP);

} // BootstrapTimer::eventFunction


/******************************************************************************
 * Constructor for HeartbeatTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
HeartbeatTimer::HeartbeatTimer( MCPO* mcpo )
	: MCPOTimer ( mcpo )
{
	logging_info("creating HeartbeatTimer");
} // HeartbeatTimer


/******************************************************************************
 * Destructor for HeartbeatTimer
 *****************************************************************************/
HeartbeatTimer::~HeartbeatTimer()
{

} // ~HeartbeatTimer


/******************************************************************************
 * EventFunction for HeartbeatTimer Timer, called periodically
 *****************************************************************************/
void HeartbeatTimer::eventFunction()
{
	logging_info("HeartbeatTimer eventFunction");
	mcpo->sendHeartbeats();

} // HeartbeatTimer::eventFunction


/******************************************************************************
 * Constructor for RefineTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
RefineTimer::RefineTimer( MCPO* mcpo )
	: MCPOTimer ( mcpo )
{

} // RefineTimer


/******************************************************************************
 * Destructor for RefineTimer
 *****************************************************************************/
RefineTimer::~RefineTimer()
{

} // ~RefineTimer


/******************************************************************************
 * EventFunction for Refine Timer, called periodically
 *****************************************************************************/
void RefineTimer::eventFunction()
{

	mcpo->maintenance();

} // RefineTimer::eventFunction


/******************************************************************************
 * Constructor for QueryTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
QueryTimer::QueryTimer( MCPO* mcpo )
	: MCPOTimer ( mcpo )
{

} // QueryTimer


/******************************************************************************
 * Destructor for QueryTimer
 *****************************************************************************/
QueryTimer::~QueryTimer()
{

} // ~QueryTimer


/******************************************************************************
 * EventFunction for QueryTimer, called periodically
 *****************************************************************************/
void QueryTimer::eventFunction()
{

	//mcpo->refinement();

} // QueryTimer::eventFunction


/******************************************************************************
 * Constructor for StructureTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
StructureTimer::StructureTimer( MCPO* mcpo )
	: MCPOTimer ( mcpo )
{

} // StructureTimer


/******************************************************************************
 * Destructor for StructureTimer
 *****************************************************************************/
StructureTimer::~StructureTimer()
{

} // ~QueryTimer


/******************************************************************************
 * EventFunction for StructureTimer, called periodically
 *****************************************************************************/
void StructureTimer::eventFunction()
{

	mcpo->reconnectToStructure();

} // StructureTimer::eventFunction


/******************************************************************************
 * Constructor for BackoffTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
BackoffTimer::BackoffTimer( MCPO* mcpo )
	: MCPOTimer ( mcpo )
{

} // StructureTimer


/******************************************************************************
 * Destructor for StructureTimer
 *****************************************************************************/
BackoffTimer::~BackoffTimer()
{

} // ~QueryTimer


/******************************************************************************
 * EventFunction for StructureTimer, called periodically
 *****************************************************************************/
void BackoffTimer::eventFunction()
{

	mcpo->initializeMCPO();

} // StructureTimer::eventFunction


/******************************************************************************
 * Constructor for RPCollisionTimer
 * @param mcp Calling MCPO Instance
 *****************************************************************************/
RPCollisionTimer::RPCollisionTimer( MCPO* mcpo )
	: MCPOTimer ( mcpo )
{

} // RPCollisionTimer


/******************************************************************************
 * Destructor for RPCollisionTimer
 *****************************************************************************/
RPCollisionTimer::~RPCollisionTimer()
{

} // ~RPCollisionTimer


/******************************************************************************
 * EventFunction for RPCollisionTimer, called periodically
 *****************************************************************************/
void RPCollisionTimer::eventFunction()
{

	mcpo->checkforRP();

} // RPCollisionTimer::eventFunction


}}} // namespace spovnet, services, mcpo
