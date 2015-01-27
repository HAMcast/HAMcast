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

#include "MCPOMsg.h"
namespace ariba {
namespace services {
namespace mcpo {

use_logging_cpp(MCPOMsg);

vsznDefault(MCPOMsg);

/******************************************************************************
 * Constructor
 * @param _type Type of message
 * @param _srcNode NodeID of message sender
 *****************************************************************************/
MCPOMsg::MCPOMsg(MESSAGE_TYPE _type, const NodeID _srcNode, int _layer, const ServiceID _groupID)
    : type( (uint8_t)_type), srcNode( _srcNode ), layer ( _layer+1 ), groupID ( _groupID ),me(""), blub(0) {

    if(type== MCPOMsg::MCPO_LOOKUP_RP){
    logging_info("creating message with type MCPO_LOOKUP_RP: " + this->toString() );
    blub=0xaabbccddee;
    }else if(type== MCPOMsg::MCPO_APPDATA){
        blub=0xDEADBEEFDEADBEEF;
    }
	//logging_info("Build MCPOMsg with layer: " << _layer);

} // MCPOMsg


/******************************************************************************
 * Destructors
 *****************************************************************************/
MCPOMsg::~MCPOMsg(){

} // ~MCPOMsg


/******************************************************************************
 * Return the message type
 * @return Type of Message
 *****************************************************************************/
MCPOMsg::MESSAGE_TYPE MCPOMsg::getType(){

	return (MESSAGE_TYPE)type;

} // getType


/******************************************************************************
 * Returns the source NodeID
 * @return NodeID of source node
 *****************************************************************************/
const NodeID& MCPOMsg::getSrcNode(){

	return srcNode;

} // getSrcNode


/******************************************************************************
 * Returns the layer
 * @return Layer the message is related to
 *****************************************************************************/
const int16_t MCPOMsg::getLayer(){

	return layer-1;

} // getSrcNode


/******************************************************************************
 * Returns the groupID
 * @return groupID the message is dedicated for
 *****************************************************************************/
const ServiceID& MCPOMsg::getGroupID(){

	return groupID;

} // getGroupID


/******************************************************************************
 * Checks message type
 * @param _type Message type to check for
 * @return True, if message type matches
 *****************************************************************************/
bool MCPOMsg::isType(MESSAGE_TYPE _type){

	return (MESSAGE_TYPE)type == _type;

} // isType


/******************************************************************************
 * returns Message as string
 * @param uint32_t base , base for identifier, default ist 16
 * @return string concatinated string with information
 *****************************************************************************/
std::string MCPOMsg::toString(const uint base){
    if(me.empty()){
    me =
            ariba::utility::Identifier(type).toString(base)
            +srcNode.toString(base)
            +ariba::utility::Identifier(layer).toString(base)
            +groupID.toString()


            ;


    }

    return  me;
}
}}} // spovnet::services::ariba
