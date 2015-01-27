// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Karlsruhe Institute of Technology (KIT)
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

#ifndef MCPO_LAYER_MSG_H__
#define MCPO_LAYER_MSG_H__

#include "ariba/ariba.h"

namespace ariba {
namespace services {
namespace mcpo {

using_serialization;

/******************************************************************************
 * Message Class extending Base Class by adding Layer Info
 *****************************************************************************/
class MCPOLayerMsg : public Message {
	VSERIALIZEABLE;

public:

	/**
	 * Constructor
	 * @param _layer CLuster layer
	 **/
	MCPOLayerMsg( int _layer = 0 );

	/** Destructor **/
	virtual ~MCPOLayerMsg();

	/**
	 * Sets cluster layer
	 * @param _layer Cluster layer
	 **/
	void setLayer( int _layer );

	/**
	 * Returns cluster layer
	 * @return Cluster layer
	 **/
	int getLayer();

private:

	/** Cluster layer **/
	uint8_t layer;

};

}}} // spovnet::services::mcpo

//** Serialization of Layer Message **/
sznBeginDefault( ariba::services::mcpo::MCPOLayerMsg, X ) {

	X && layer;

} sznEnd();

#endif // MCPO_LAYER_MSG_H__
