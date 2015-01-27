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
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
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

#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <string>
#include "ariba/utility/serialization.h"

#include "_namespace.h"
NAMESPACE_BEGIN;

using std::string;
using_serialization;

/**
 * An abstract address class
 * This class serves as the base for all address classes
 *
 * @author Timo Rohrberg
 */
class Address : public VSerializeable {
	VSERIALIZEABLE;
public:
	Address();

    /**
     * Default destructor
     */
    virtual ~Address();

    /**
     * Returns a textual representation of the address
     */
    virtual string toString() const;
};

NAMESPACE_END

sznBeginDefault( ariba::utility::Address, X ) {
} sznEnd();

#endif /* IADDRESS_H_ */
