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

#ifndef MODULE_H_
#define MODULE_H_

#include <string>
#include <vector>

// usings
using std::vector;
using std::string;

namespace ariba {

/**
 * \addtogroup public
 * @{
 *
 * This class serves as base class for generic modules that
 * can be initialized, started, stopped and configured using standard
 * human-readable properties.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class Module {
public:
	Module();
	virtual ~Module();

	/**
	 * This method (re-)initializes this module.
	 */
	virtual void initialize();

	/**
	 * This method (re-)starts this module
	 */
	virtual void start();

	/**
	 * This method stops this module
	 */
	virtual void stop();

	/**
	 * Returns the name of this module
	 *
	 * @return The name of this module
	 */
	virtual string getName() const;

	/**
	 * Sets a property in this module
	 *
	 * @param key The key of the property
	 * @param value The value of the property
	 */
	virtual void setProperty( string key, string value );

	/**
	 * Returns the value of a specified property
	 *
	 * @param key The key of the property
	 * @return The value of the property
	 */
	virtual const string getProperty( string key ) const;

	/**
	 * Returns a vector containing all possible property keys in a
	 * human-readable form
	 *
	 * @return A vector containing all possible property keys
	 */
	virtual const vector<string> getProperties() const;
};

} // namespace ariba

/** @} */

#endif /* MODULE_H_ */
