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

#ifndef NAME_H_
#define NAME_H_

#include <iostream>
#include <memory.h>
#include <string>

using std::string;

// forward declaration
namespace ariba { class Name; }

std::ostream& operator<<( std::ostream&, const ariba::Name& );

#include "Identifiers.h"

namespace ariba {

/**
 * \addtogroup public
 * @{
 *
 * This class is a wrapper for canonical names.
 * Currently only human readable names are supported.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class Name {
	friend std::ostream& operator<<( std::ostream&, const ::ariba::Name& );
public:
	static const Name UNSPECIFIED;

	/**
	 * Constructs a new, yet unspecified name.
	 */
	Name();

	/**
	 * Constructs a new name. If no length is specified, a human-readable
	 * name is assumed.
	 *
	 * @param name The name
	 * @param len The optional name length, if binary data is used as name
	 * @param copy A flag, whether the name's memory needs to be copied
	 */
	Name(const char* name, int len = -1, bool copy = false);

	/**
	 * Constructs a new name out of a human readable string.
	 *
	 * @param name A human readable name
	 */
	Name(string name);

	/**
	 * The copy constructor.
	 */
	Name(const Name& name);

	/**
	 * Destroys the name and releases underlying memory if this name is a copy.
	 */
	virtual ~Name();

	/**
	 * Returns the binary bytes of the name
	 *
	 * @return The binary data
	 */
	const uint8_t* bytes() const;

	/**
	 * Returns the length of the name in bytes.
	 *
	 * @return The length of the name
	 */
	const size_t length() const;

	/**
	 * The common assign operator
	 */
	Name& operator=( const Name& name );

	/**
	 * The common implementation of the "equal" operator.
	 */
	bool operator==(const Name& name) const;

	/**
	 * The common implementation of the "unequal" operator.
	 */
	bool operator!=(const Name& name) const;

	/**
	 * Returns true, if the name is yet unspecified
	 */
	bool isUnspecified() const;

	/**
	 * Returns a random name.
	 */
	static Name random();

	/**
	 * Returns a human-readable representation of this name
	 */
	string toString() const;

	// hack: to be changed!
	NodeID toNodeId() const;

	// hack: to be changed!
	SpoVNetID toSpoVNetId() const;

private:
	uint8_t* _bytes; //< internal pointer
	int _length; //< length of internal pointer
	bool _copy; //< is the buffer a real copy
	bool _hreadable; //< is the name human readable

	void init(const char* name, int len, bool copy, bool hreadable);
};

} // namespace ariba

/** @} */

#endif /* NAME_H_ */
