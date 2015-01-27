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

#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include <memory>
#include <string>
#include <cassert>
#include <iostream>
#include <gmp.h>
#include <boost/cstdint.hpp>
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/types/Address.h"
#include "ariba/utility/serialization.h"

/**< maximum length of the key */
#define MAX_KEYLENGTH 192

namespace ariba {
namespace utility {

using_serialization;

class IdentifierBit;

/**
 * An abstract address class for identifier.<br/>
 *
 * This class is the base for all identifier classes.
 */
class Identifier: public Address {
	VSERIALIZEABLE;
	use_logging_h( Identifier );
public:

	//-------------------------------------------------------------------------
	// virtual function from Address
	//-------------------------------------------------------------------------

	virtual void clearAddress();
	virtual bool setAddress(string address);
	virtual string getAddress() const;

	//-------------------------------------------------------------------------
	// constants
	//-------------------------------------------------------------------------

	static const Identifier UNSPECIFIED_KEY; /**< Identifier without defined key */
	static const Identifier ZERO; /**< Identifier with key initialized as 0 */
	static const Identifier ONE; /**< Identifier with key initialized as 1 */

	//-------------------------------------------------------------------------
	// construction and destruction
	//-------------------------------------------------------------------------

	/**
	 * Default constructor
	 *
	 * Contructs an unspecified overlay key
	 */
	Identifier();

	/**
	 * Constructs an overlay key initialized with a common integer
	 *
	 * @param num The integer to initialize this key with
	 */
	Identifier(uint32_t num);

	/**
	 * Constructs a key out of a buffer
	 * @param buffer Source buffer
	 * @param size Buffer size (in bytes)
	 */
	Identifier(const unsigned char* buffer, uint size);

	/**
	 * Constructs a key out of a string number.
	 */
	Identifier(const std::string& str, uint base = 16);

	/**
	 * Copy constructor.
	 *
	 * @param rhs The key to copy.
	 */
	Identifier(const Identifier& rhs);

	/**
	 * Default destructor.
	 *
	 * Does nothing ATM.
	 */
	virtual ~Identifier();

	//-------------------------------------------------------------------------
	// string representations & node key attributes
	//-------------------------------------------------------------------------

	/**
	 * Returns a string representation of this key
	 *
	 * @return String representation of this key
	 */
	virtual std::string toString(uint base = 16) const;

	/**
	 * Common stdc++ console output method
	 */
	friend std::ostream& operator<<(std::ostream& os, const Identifier& c);

	/**
	 * Returns true, if the key is unspecified
	 *
	 * @return Returns true, if key is unspecified
	 */
	bool isUnspecified() const;

	//-------------------------------------------------------------------------
	// operators
	//-------------------------------------------------------------------------

	/**
	 * compares this to a given Identifier
	 *
	 * @param compKey the the Identifier to compare this to
	 * @return true if compKey->key is smaller than this->key, else false
	 */
	bool operator<(const Identifier& compKey) const;

	/**
	 * compares this to a given Identifier
	 *
	 * @param compKey the the Identifier to compare this to
	 * @return true if compKey->key is greater than this->key, else false
	 */
	bool operator>(const Identifier& compKey) const;

	/**
	 * compares this to a given Identifier
	 *
	 * @param compKey the the Identifier to compare this to
	 * @return true if compKey->key is smaller than or equal to this->key, else false
	 */
	bool operator<=(const Identifier& compKey) const;

	/**
	 * compares this to a given Identifier
	 *
	 * @param compKey the the Identifier to compare this to
	 * @return true if compKey->key is greater than or equal to this->key, else false
	 */
	bool operator>=(const Identifier& compKey) const;

	/**
	 * compares this to a given Identifier
	 *
	 * @param compKey the the Identifier to compare this to
	 * @return true if compKey->key is equal to this->key, else false
	 */
	bool operator==(const Identifier& compKey) const;

	/**
	 * compares this to a given Identifier
	 *
	 * @param compKey the the Identifier to compare this to
	 * @return true if compKey->key is not equal to this->key, else false
	 */
	bool operator!=(const Identifier& compKey) const;

	/**
	 * Unifies all compare operations in one method
	 *
	 * @param compKey key to compare with
	 * @return int -1 if smaller, 0 if equal, 1 if greater
	 */
	int compareTo(const Identifier& compKey) const;

	/**
	 * assigns Identifier of rhs to this->key
	 *
	 * @param rhs the Identifier with the defined key
	 * @return this Identifier object
	 */
	Identifier& operator=(const Identifier& rhs);

	/**
	 * substracts 1 from this->key
	 *
	 * @return this Identifier object
	 */
	Identifier& operator--();

	/**
	 * adds 1 to this->key
	 *
	 * @return this Identifier object
	 */
	Identifier& operator++();

	/**
	 * adds rhs->key to this->key
	 *
	 * @param rhs the Identifier with the defined key
	 * @return this Identifier object
	 */
	Identifier& operator+=(const Identifier& rhs);

	/**
	 * substracts rhs->key from this->key
	 *
	 * @param rhs the Identifier with the defined key
	 * @return this Identifier object
	 */
	Identifier& operator-=(const Identifier& rhs);

	/**
	 * adds rhs->key to this->key
	 *
	 * @param rhs the Identifier with the defined key
	 * @return this Identifier object
	 */
	Identifier operator+(const Identifier& rhs) const;

	/**
	 * substracts rhs->key from this->key
	 *
	 * @param rhs the Identifier with the defined key
	 * @return this Identifier object
	 */
	Identifier operator-(const Identifier& rhs) const;

	/**
	 * substracts 1 from this->key
	 *
	 * @return this Identifier object
	 */
	Identifier operator--(int);

	/**
	 * adds 1 to this->key
	 *
	 * @return this Identifier object
	 */
	Identifier operator++(int);

	/**
	 * bitwise shift right
	 *
	 * @param num number of bits to shift
	 * @return this Identifier object
	 */
	Identifier operator>>(uint num) const;

	/**
	 * bitwise shift left
	 *
	 * @param num number of bits to shift
	 * @return this Identifier object
	 */
	Identifier operator<<(uint num) const;

	/**
	 * bitwise AND of rhs->key and this->key
	 *
	 * @param rhs the Identifier AND is calculated with
	 * @return this Identifier object
	 */
	Identifier operator&(const Identifier& rhs) const;

	/**
	 * bitwise OR of rhs->key and this->key
	 *
	 * @param rhs the Identifier OR is calculated with
	 * @return this Identifier object
	 */
	Identifier operator|(const Identifier& rhs) const;

	/**
	 * bitwise XOR of rhs->key and this->key
	 *
	 * @param rhs the Identifier XOR is calculated with
	 * @return this Identifier object
	 */
	Identifier operator^(const Identifier& rhs) const;

	/**
	 * bitwise NOT of this->key
	 *
	 * @return this Identifier object
	 */
	Identifier operator~() const;

	/**
	 * returns the n-th bit of this->key
	 *
	 * @param n the position of the returned bit
	 * @return the bit on position n in this->key
	 */
	IdentifierBit operator[](uint n);

	/**
	 * sets a bit of this->key
	 *
	 * @param pos the position of the bit to set
	 * @param value new value for bit at position pos
	 * @return *this
	 */
	Identifier& setBitAt(uint pos, bool value);

	//-------------------------------------------------------------------------
	// additional math
	//-------------------------------------------------------------------------

	/**
	 * Returns a sub integer at position p with n-bits. p is counted starting
	 * from the least significant bit of the key as bit 0. Bit p of the key
	 * becomes bit 0 of the returned integer.
	 *
	 * @param p the position of the sub-integer
	 * @param n the number of bits to be returned (max.32)
	 * @return The sub-integer.
	 */
	uint32_t get(uint p, uint n) const;

	/**
	 * Returns a hash value for the key
	 *
	 * @return size_t The hash value
	 */
	size_t hash() const;

	/**
	 * Returns the position of the msb in this key, which represents
	 * just the logarithm to base 2.
	 *
	 * @return The logarithm to base 2 of this key.
	 */
	int log_2() const;

	/**
	 * Fills the suffix starting at pos with random bits to lsb.
	 *
	 * @param pos
	 * @return Identifier
	 */
	Identifier randomSuffix(uint pos) const;

	/**
	 * Fills the prefix starting at pos with random bits to msb.
	 *
	 * @param pos
	 * @return Identifier
	 */
	Identifier randomPrefix(uint pos) const;

	/**
	 * Calculates the number of equal bits from the left with another
	 * Key (shared prefix length)
	 *
	 * @param compKey the Key to compare with
	 * @return length of shared prefix
	 */
	uint sharedPrefixLength(const Identifier& compKey) const;

	/**
	 * Returns true, if this key is element of the interval (keyA, keyB)
	 * on the ring.
	 *
	 * @param keyA The left border of the interval
	 * @param keyB The right border of the interval
	 * @return True, if the key is element of the interval (keyA, keyB)
	 */
	bool isBetween(const Identifier& keyA, const Identifier& keyB) const;

	/**
	 * Returns true, if this key is element of the interval (keyA, keyB]
	 * on the ring.
	 *
	 * @param keyA The left border of the interval
	 * @param keyB The right border of the interval
	 * @return True, if the key is element of the interval (keyA, keyB]
	 */
	bool isBetweenR(const Identifier& keyA, const Identifier& keyB) const;

	/**
	 * Returns true, if this key is element of the interval [keyA, keyB)
	 * on the ring.
	 *
	 * @param keyA The left border of the interval
	 * @param keyB The right border of the interval
	 * @return True, if the key is element of the interval [keyA, keyB)
	 */
	bool isBetweenL(const Identifier& keyA, const Identifier& keyB) const;

	/**
	 * Returns true, if this key is element of the interval [keyA, keyB]
	 * on the ring.
	 *
	 * @param keyA The left border of the interval
	 * @param keyB The right border of the interval
	 * @return True, if the key is element of the interval [keyA, keyB]
	 */
	bool isBetweenLR(const Identifier& keyA, const Identifier& keyB) const;

	//-------------------------------------------------------------------------
	// static methods
	//-------------------------------------------------------------------------

	/**
	 * Set the length of an Identifier
	 *
	 * @param length keylength in bits
	 */
	static void setKeyLength(uint length);

	/**
	 * Returns the length in number of bits.
	 *
	 * @return The length in number of bits.
	 */
	static uint getLength();

	/**
	 * Returns a random key.
	 *
	 * @return A random key.
	 */
	static Identifier random();

	/**
	 * Returns the maximum key, i.e. a key filled with bit 1
	 *
	 * @return The maximum key, i.e. a key filled with bit 1
	 */
	static Identifier max();


	// README: due to some problems with serialization the keylength
	// was changed to 192 bit! As none of the hashing functions
	// can provide 192 bit output, and we currently don't use any
	// hashing for identifiers in the demo at all ... this is all commented out!!
	/**
	 * Returns a key with the SHA1 cryptographic hash of a
	 * string
	 *
	 * @param value A string value.
	 * @return SHA1 of value
	 */
	static Identifier sha1(const string& value);

	static Identifier sha1(const uint8_t* value, size_t length );

	/**
	 * Returns a key 2^exponent.
	 *
	 * @param exponent The exponent.
	 * @return Key=2^exponent.
	 */
	static Identifier pow2(uint exponent);

private:
	// private constants

	static uint keyLength; /**< actual length of the key */
	static uint aSize; /**< number of needed machine words to hold the key*/
	static mp_limb_t GMP_MSB_MASK; /**< bits to fill up if key does not
	 exactly fit in one or more machine words */

	// private fields
	bool isUnspec; /**< is this->key unspecified? */

	void seed();

	static const size_t array_size = MAX_KEYLENGTH / (8 * sizeof(mp_limb_t))
			+ (MAX_KEYLENGTH % (8 * sizeof(mp_limb_t)) != 0 ? 1 : 0);

	/** the overlay key this object represents */
	mp_limb_t key[array_size + 1];

	// private "helper" methods
	/**
	 * trims key after key operations
	 */
	void trim();

	/**
	 * set this->key to 0 and isUnspec to false
	 */
	void clear();
};

/**
 * An auxiliary class for single bits in OverlayKey
 *
 * Allows statements like "key[n] = true"
 */
class IdentifierBit {
public:

	IdentifierBit(bool value, uint pos, Identifier* key) :
		bit(value), pos(pos), key(key) {
	}

	/** Converts to a boolean value */
	inline operator bool() {
		return bit;
	}

	/** Sets the corresponding bit to a boolean value
	 * @param value value to set to
	 */
	inline IdentifierBit& operator=(bool value) {
		key->setBitAt(pos, value);
		return *this;
	}

	inline IdentifierBit& operator^=(bool value) {
		key->setBitAt(pos, (*key)[pos] ^ value);
		return *this;
	}

private:

	bool bit;
	uint pos;
	Identifier* key;
};

}
} // namespace ariba, common

/* serializers */
sznBeginDefault( ariba::utility::Identifier, X ) {

	// calculate length of key
	uint16_t len = array_size*sizeof(mp_limb_t);
	uint8_t unspec = isUnspec;

	// only serialize the lower 16 bits of keyLength
	X && unspec && len;

	// serialize the identifier
	for (int i=array_size-1; i>=0; i--) X && key[i];

	// when deserializing set unspec flag
	if (X.isDeserializer()){
		isUnspec = unspec;
	}

} sznEnd();

#endif /* IDENTIFIER_H_ */
