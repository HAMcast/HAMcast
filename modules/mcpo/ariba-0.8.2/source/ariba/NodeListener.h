// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
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

#ifndef NODELISTENER_H_
#define NODELISTENER_H_

#include "Identifiers.h"

namespace ariba {

// forward decl
namespace overlay {
	class BaseOverlay;
}

/**
 * \addtogroup public
 * @{
 *
 * This class is used to inform a listener about node changes.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 * @author Christoph Mayer <mayer@tm.uka.de>
 */
class NodeListener {
	friend class ariba::overlay::BaseOverlay;
public:
	NodeListener();
	virtual ~NodeListener();

protected:
	/**
	 * This event method is called, when a node has completed its join
	 * procedure.
	 *
	 * @param vid The spovnet id
	 * @param nid The node id
	 */
	virtual void onJoinCompleted( const SpoVNetID& vid );

	/**
	 * This event method is called, when a node failed to join a spovnet.
	 *
	 * @param vid The spovnet id
	 * @param nid The node id
	 */
	virtual void onJoinFailed( const SpoVNetID& vid );

	/**
	 * This event method is called, when a node succeeded to leave a spovnet.
	 *
	 * @param vid The spovnet id
	 * @param nid The node id
	 */
	virtual void onLeaveCompleted( const SpoVNetID& vid );

	/**
	 * This event method is called, when a node failed to leave a spovnet.
	 *
	 * @param vid The spovnet id
	 * @param nid The node id
	 */
	virtual void onLeaveFailed( const SpoVNetID& vid );
};

} // namespace ariba

/** @} */

#endif /* NODELISTENER_H_ */
