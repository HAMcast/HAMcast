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

#ifndef SPOVNETPROPERTIES_H_
#define SPOVNETPROPERTIES_H_

#include <string>
#include <iostream>
#include <streambuf>

using std::string;

namespace ariba {
// forward declaration
class SpoVNetProperties;
}

#include "Identifiers.h"
#include "Name.h"

namespace ariba {

/**
 * \addtogroup public
 * @{
 *
 * This class implements a container that holds all properties of a
 * SpoVNet instance. It may evolve with new features when new features
 * are introduced.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 * @author Christoph Mayer <mayer@tm.uka.de>
 */
class SpoVNetProperties {
public:
	/**
	 * Different types of overlays that are supported
	 */
	enum OverlayType {
		ONE_HOP_OVERLAY = 0,
		CHORD_OVERLAY = 1,
	};

	/**
	 * This object holds the default settings for a newly created spovnet
	 * instance.
	 */
	static const SpoVNetProperties DEFAULT;

	/**
	 * Constructs a new default SpoVnet property object.
	 */
	SpoVNetProperties();

	/**
	 * Copy constructor.
	 */
	SpoVNetProperties(const SpoVNetProperties& copy);

	/**
	 * Destructor.
	 */
	virtual ~SpoVNetProperties();

	/**
	 * Returns the overlay type.
	 */
	const OverlayType getBaseOverlayType() const;

	void setBaseOverlayType( OverlayType type ) {
		this->type = type;
	}

	/**
	 * Returns a human readable string representation of the SpoVNet properties
	 *
	 * @return A human readable string representation of the SpoVNet properties
	 */
	std::string toString() const;

private:
	uint8_t type;
};

} // namespace ariba

/** @} */

#endif /* SPOVNETPROPERTIES_H_ */
