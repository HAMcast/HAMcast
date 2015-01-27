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

#include "Identifier.h"
#include "ariba/utility/misc/sha1.h"

namespace ariba {
namespace utility {

uint Identifier::keyLength = MAX_KEYLENGTH;

uint Identifier::aSize = Identifier::keyLength / (8 * sizeof(mp_limb_t))
		+ (Identifier::keyLength % (8 * sizeof(mp_limb_t)) != 0 ? 1 : 0);

mp_limb_t Identifier::GMP_MSB_MASK = (Identifier::keyLength % GMP_LIMB_BITS)
		!= 0 ? (((mp_limb_t) 1 << (Identifier::keyLength % GMP_LIMB_BITS)) - 1)
		: (mp_limb_t) -1;

/* virtual serialization */
vsznDefault(Identifier);
use_logging_cpp( Identifier );

//--------------------------------------------------------------------
// constants
//--------------------------------------------------------------------

// predefined keys
const Identifier Identifier::UNSPECIFIED_KEY;
const Identifier Identifier::ZERO((uint32_t) 0);
const Identifier Identifier::ONE((uint32_t) 1);

// hex crap
const char* HEX = "0123456789abcdef";
gmp_randstate_t randstate;

void Identifier::clearAddress() {
	for (size_t i = 0; i < array_size; i++)
		key[i] = 0;
	isUnspec = false;
	trim();
}

bool Identifier::setAddress(string address) {
	return false;
}

string Identifier::getAddress() const {
	return toString();
}

//--------------------------------------------------------------------
// construction and destruction
//--------------------------------------------------------------------

// default construction: create a unspecified node key
Identifier::Identifier() {
	seed();
	isUnspec = true;
	trim();
}

// create a key out of an normal integer
Identifier::Identifier(uint32_t num) {
	seed();
	clearAddress();
	key[0] = (uint32_t) num;
	trim();
}

// create a key out of a buffer
Identifier::Identifier(const unsigned char* buf, uint size) {
	seed();
	int trimSize, offset;
	clearAddress();
	trimSize = (int) std::min((uint) (aSize * sizeof(mp_limb_t)), size);
	offset = aSize * sizeof(mp_limb_t) - trimSize;
	memcpy(((char*) key) + offset, buf, trimSize);
	trim();
}

// create a key out of an string with the given base
Identifier::Identifier(const std::string& str, uint base) {
	seed();

	if ((base < 2) || (base > 16)) {
		logging_error( "Identifier::Identifier(): Invalid base!" );
		return;
	}

	string s(str);
	clearAddress();

	for (uint i = 0; i < s.size(); i++) {
		if ((s[i] >= '0') && (s[i] <= '9')) {
			s[i] -= '0';
		} else if ((s[i] >= 'a') && (s[i] <= 'f')) {
			s[i] -= ('a' - 10);
		} else if ((s[i] >= 'A') & (s[i] <= 'F')) {
			s[i] -= ('A' - 10);
		} else {
			logging_error( "Identifier::Identifier(): "
					"Invalid character in string!" );
			return;
		}
	}

	mpn_set_str((mp_limb_t*) this->key, (const unsigned char*) s.c_str(),
			str.size(), base);
	trim();
}

// copy constructor
Identifier::Identifier(const Identifier& rhs) {
	seed();
	(*this) = rhs;
}

// default destructur
Identifier::~Identifier() {
}

void Identifier::seed(){
	static bool isSeeded = false;
	if( isSeeded ) return;

	gmp_randinit_default( randstate );
	gmp_randseed_ui( randstate, time(NULL) );

	isSeeded = true;
}

//--------------------------------------------------------------------
// string representations & node key attributes
//--------------------------------------------------------------------

void Identifier::setKeyLength(uint length) {
	if ((length < 1) || (length > Identifier::keyLength)) {

		logging_error( "Identifier::setKeyLength(): length must be <= " <<
				MAX_KEYLENGTH << " and setKeyLength() must not be " <<
				"called twice with different length!" );
		return;
	}

	keyLength = length;

	aSize = keyLength / (8 * sizeof(mp_limb_t)) + (keyLength % (8
			* sizeof(mp_limb_t)) != 0 ? 1 : 0);

	GMP_MSB_MASK = (keyLength % GMP_LIMB_BITS) != 0 ? (((mp_limb_t) 1
			<< (keyLength % GMP_LIMB_BITS)) - 1) : (mp_limb_t) -1;
}

// returns the length in bits
uint Identifier::getLength() {
	return Identifier::keyLength;
}

bool Identifier::isUnspecified() const {
	return isUnspec;
}

std::string Identifier::toString(uint base) const {
	if ((base < 2) || (base > 16)) {
		logging_error( "Identifier::Identifier(): Invalid base!" );
		return "";
	}

	if (isUnspec) return std::string("<unspec>");

	char temp[250];
	if (base == 16) {
		int k = 0;
		for (int i = (keyLength - 1) / 4; i >= 0; i--, k++) {
			temp[k] = HEX[this->get(4 * i, 4)];
		}
		temp[k] = 0;
		return std::string((const char*) temp);
	} else if (base == 2) {
		int k = 0;
		for (int i = MAX_KEYLENGTH - 1; i >= 0; i -= 1, k++)
			temp[k] = HEX[this->get(i, 1)];
		temp[k] = 0;
		return std::string((const char*) temp);
	}
	//#endif
#if 0
	size_t log2[33] = {0};
	log2[2] = 1;
	log2[4] = 2;
	log2[8] = 3;
	log2[16] = 4;
	log2[32] = 5;
	size_t sh = (sizeof(mp_limb_t)*8/log2[base]);
	//		key[array_size] = ~0;
	mp_size_t last = mpn_get_str((unsigned char*) temp, base,
			(mp_limb_t*) this->key, aSize+1);
	for (int i = 0; i < last-sh; i++) {
		temp[i] = HEX[temp[i+sh]];
	}
	temp[last-sh] = 0;
	return std::string((const char*) temp);
}
#endif
	return "<not supported>";
}

//--------------------------------------------------------------------
// operators
//--------------------------------------------------------------------

// assignment operator
Identifier& Identifier::operator=(const Identifier& rhs) {
	isUnspec = rhs.isUnspec;
	memcpy(key, rhs.key, aSize * sizeof(mp_limb_t));
	return *this;
}

// sub one prefix operator
Identifier& Identifier::operator--() {
	return (*this -= ONE);
}

// sub one postfix operator
Identifier Identifier::operator--(int) {
	Identifier clone = *this;
	*this -= ONE;
	return clone;
}

// add one prefix operator
Identifier& Identifier::operator++() {
	return (*this += ONE);
}

// sub one postfix operator
Identifier Identifier::operator++(int) {
	Identifier clone = *this;
	*this += ONE;
	return clone;
}

// add assign operator
Identifier& Identifier::operator+=(const Identifier& rhs) {
	mpn_add_n((mp_limb_t*) key, (mp_limb_t*) key, (mp_limb_t*) rhs.key, aSize);
	trim();
	isUnspec = false;
	return *this;
}

// sub assign operator
Identifier& Identifier::operator-=(const Identifier& rhs) {
	mpn_sub_n((mp_limb_t*) key, (mp_limb_t*) key, (mp_limb_t*) rhs.key, aSize);
	trim();
	isUnspec = false;
	return *this;
}

// add operator
Identifier Identifier::operator+(const Identifier& rhs) const {
	Identifier result = *this;
	result += rhs;
	return result;
}

// sub operator
Identifier Identifier::operator-(const Identifier& rhs) const {
	Identifier result = *this;
	result -= rhs;
	return result;
}

// compare operators
bool Identifier::operator<(const Identifier& compKey) const {
	return compareTo(compKey) < 0;
}
bool Identifier::operator>(const Identifier& compKey) const {
	return compareTo(compKey) > 0;
}
bool Identifier::operator<=(const Identifier& compKey) const {
	return compareTo(compKey) <= 0;
}
bool Identifier::operator>=(const Identifier& compKey) const {
	return compareTo(compKey) >= 0;
}
bool Identifier::operator==(const Identifier& compKey) const {

	if( this->isUnspecified() && compKey.isUnspecified() )
		return true;
	else
		return compareTo(compKey) == 0;
}
bool Identifier::operator!=(const Identifier& compKey) const {
	return compareTo(compKey) != 0;
}

// bitwise xor
Identifier Identifier::operator^(const Identifier& rhs) const {
	Identifier result = *this;
	for (uint i = 0; i < aSize; i++) {
		result.key[i] ^= rhs.key[i];
	}

	return result;
}

// bitwise or
Identifier Identifier::operator|(const Identifier& rhs) const {
	Identifier result = *this;
	for (uint i = 0; i < aSize; i++) {
		result.key[i] |= rhs.key[i];
	}

	return result;
}

// bitwise and
Identifier Identifier::operator&(const Identifier& rhs) const {
	Identifier result = *this;
	for (uint i = 0; i < aSize; i++) {
		result.key[i] &= rhs.key[i];
	}

	return result;
}

// complement
Identifier Identifier::operator~() const {
	Identifier result = *this;
	for (uint i = 0; i < aSize; i++) {
		result.key[i] = ~key[i];
	}
	result.trim();

	return result;
}

// bitwise shift right
Identifier Identifier::operator>>(uint num) const {
	Identifier result = ZERO;
	int i = num / GMP_LIMB_BITS;

	num %= GMP_LIMB_BITS;

	if (i >= (int) aSize) return result;

	for (int j = 0; j < (int) aSize - i; j++) {
		result.key[j] = key[j + i];
	}
	mpn_rshift(result.key, result.key, aSize, num);
	result.isUnspec = false;
	result.trim();

	return result;
}

// bitwise shift left
Identifier Identifier::operator<<(uint num) const {
	Identifier result = ZERO;
	int i = num / GMP_LIMB_BITS;

	num %= GMP_LIMB_BITS;

	if (i >= (int) aSize) return result;

	for (int j = 0; j < (int) aSize - i; j++) {
		result.key[j + i] = key[j];
	}
	mpn_lshift(result.key, result.key, aSize, num);
	result.isUnspec = false;
	result.trim();

	return result;
}

// get bit
IdentifierBit Identifier::operator[](uint n) {
	return IdentifierBit(get(n, 1), n, this);
}

Identifier& Identifier::setBitAt(uint pos, bool value) {
	mp_limb_t digit = 1;
	digit = digit << (pos % GMP_LIMB_BITS);

	if (value) {
		key[pos / GMP_LIMB_BITS] |= digit;
	} else {
		//key[pos / GMP_LIMB_BITS] = key[pos / GMP_LIMB_BITS] & ~digit;
		key[pos / GMP_LIMB_BITS] &= ~digit;
	}

	return *this;
}
;

//--------------------------------------------------------------------
// additional math
//--------------------------------------------------------------------

// returns a sub integer
uint32_t Identifier::get(uint p, uint n) const {
	int i = p / GMP_LIMB_BITS, // index of starting bit
			f = p % GMP_LIMB_BITS, // position of starting bit
			f2 = f + n - GMP_LIMB_BITS; // how many bits to take from next index

	if (p + n > Identifier::keyLength) {
		logging_error( "Identifier::get: Invalid range (index too large!)" );
		return 0;
	}

	return ((key[i] >> f) | // get the bits of key[i]
			(f2 > 0 ? (key[i + 1] << (GMP_LIMB_BITS - f)) : 0)) & // the extra bits from key[i+1]
			(((uint32_t) (~0)) >> (GMP_LIMB_BITS - n)); // delete unused bits
}

// fill suffix with random bits
Identifier Identifier::randomSuffix(uint pos) const {
	Identifier newKey = *this;
	int i = pos / GMP_LIMB_BITS, j = pos % GMP_LIMB_BITS;
	mp_limb_t m = ((mp_limb_t) 1 << j) - 1;
	mp_limb_t rnd;

	mpn_random(&rnd, 1);
	newKey.key[i] &= ~m;
	newKey.key[i] |= (rnd & m);
	mpn_random(newKey.key, i);
	newKey.trim();

	return newKey;
}

// fill prefix with random bits
Identifier Identifier::randomPrefix(uint pos) const {
	Identifier newKey = *this;
	int i = pos / GMP_LIMB_BITS, j = pos % GMP_LIMB_BITS;
	mp_limb_t m = ((mp_limb_t) 1 << j) - 1;
	mp_limb_t rnd;

	mpn_random(&rnd, 1);

	newKey.key[i] &= m;
	newKey.key[i] |= (rnd & ~m);
	for (int k = aSize - 1; k != i; k--) {
		mpn_random(&newKey.key[k], 1);
	}
	newKey.trim();

	return newKey;
}

// calculate shared prefix length
uint Identifier::sharedPrefixLength(const Identifier& compKey) const {
	if (compareTo(compKey) == 0) return keyLength;

	uint length = 0;
	int i;
	uint j;
	bool msb = true;

	// count equal limbs first:
	for (i = aSize - 1; i >= 0; --i) {
		if (this->key[i] != compKey.key[i]) {
			// XOR first differing limb for easy counting of the bits:
			mp_limb_t d = this->key[i] ^ compKey.key[i];
			if (msb) d <<= (GMP_LIMB_BITS - (keyLength % GMP_LIMB_BITS));
			for (j = GMP_LIMB_BITS - 1; d >>= 1; --j)
				;
			length += j;
			break;
		}
		length += GMP_LIMB_BITS;
		msb = false;
	}

	return length;
}

// calculate log of base 2
int Identifier::log_2() const {
	int16_t i = aSize - 1;

	while (i >= 0 && key[i] == 0) {
		i--;
	}

	if (i < 0) {
		return -1;
	}

	mp_limb_t j = key[i];
	i *= GMP_LIMB_BITS;
	while (j != 0) {
		j >>= 1;
		i++;
	}

	return i - 1;
}

// returns a simple hash of the key
size_t Identifier::hash() const {
	return (size_t) key[0];
}

// returns true, if this key is element of the interval (keyA, keyB)
bool Identifier::isBetween(const Identifier& keyA, const Identifier& keyB) const {
	if (isUnspec || keyA.isUnspec || keyB.isUnspec) return false;

	if (*this == keyA) return false;
	else if (keyA < keyB) return ((*this > keyA) && (*this < keyB));
	else return ((*this > keyA) || (*this < keyB));
}

// returns true, if this key is element of the interval (keyA, keyB]
bool Identifier::isBetweenR(const Identifier& keyA, const Identifier& keyB) const {
	if (isUnspec || keyA.isUnspec || keyB.isUnspec) return false;

	if ((keyA == keyB) && (*this == keyA)) return true;
	else if (keyA <= keyB) return ((*this > keyA) && (*this <= keyB));
	else return ((*this > keyA) || (*this <= keyB));
}

// returns true, if this key is element of the interval [keyA, keyB)
bool Identifier::isBetweenL(const Identifier& keyA, const Identifier& keyB) const {
	if (isUnspec || keyA.isUnspec || keyB.isUnspec) return false;

	if ((keyA == keyB) && (*this == keyA)) return true;
	else if (keyA <= keyB) return ((*this >= keyA) && (*this < keyB));
	else return ((*this >= keyA) || (*this < keyB));
}

// returns true, if this key is element of the interval [keyA, keyB]
bool Identifier::isBetweenLR(const Identifier& keyA, const Identifier& keyB) const {
	if (isUnspec || keyA.isUnspec || keyB.isUnspec) return false;

	if ((keyA == keyB) && (*this == keyA)) return true;
	else if (keyA <= keyB) return ((*this >= keyA) && (*this <= keyB));
	else return ((*this >= keyA) || (*this <= keyB));
}

//----------------------------------------------------------------------
// statics and globals
//----------------------------------------------------------------------

// default console output
std::ostream& operator<<(std::ostream& os, const Identifier& c) {
	os << c.toString(16);
	return os;
}
;

// returns a key filled with bit 1
Identifier Identifier::max() {
	Identifier newKey;

	for (uint i = 0; i < aSize; i++) {
		newKey.key[i] = ~0;
	}
	newKey.isUnspec = false;
	newKey.trim();

	return newKey;
}

// generate random number
Identifier Identifier::random() {
	Identifier newKey = ZERO;
	newKey.clearAddress();

	//as mpn_random has no seeding function
	// we mess aroung here a little to achive some randomness
	// using the rand function that _is_ seeded in the
	// StartupWrapper::initSystem function

	Identifier keyRandom = ZERO;
	mpn_random(keyRandom.key, aSize);
	Identifier keyrnd( rand() );
	mpn_mul_1( newKey.key, keyRandom.key, aSize, *keyrnd.key );

	newKey.trim();
	assert(!newKey.isUnspecified());

	return newKey;
}

Identifier Identifier::sha1(const string& input) {
	Identifier newKey;
	newKey.clearAddress();
	uint8_t temp[40];
	for (int i=0; i<40; i++) temp[i] = 0;
	const char* c_str = input.c_str();

	CSHA1 sha;
	sha.Reset();
	sha.Update( (uint8_t*)c_str, (uint32_t)input.size() );
	sha.Final();
	sha.GetHash(temp);
	mpn_set_str(newKey.key, (const uint8_t*)temp,
				 (int)aSize * sizeof(mp_limb_t), 256);
	newKey.isUnspec = false;
	newKey.trim();

	return newKey;
}

Identifier Identifier::sha1(const uint8_t* value, size_t length ) {
	Identifier newKey;
	uint8_t temp[40];
	for (int i=0; i<40; i++) temp[i] = 0;

	CSHA1 sha;
	sha.Reset();
	sha.Update( const_cast<uint8_t*>(value), (uint32_t)length );
	sha.Final();
	sha.GetHash(temp);
	mpn_set_str( newKey.key, (const uint8_t*)temp, (int)aSize * sizeof(mp_limb_t), 256);
	newKey.isUnspec = false;
	newKey.trim();
	return newKey;
}

// generate a key=2**exponent
Identifier Identifier::pow2(uint exponent) {
	Identifier newKey = ZERO;

	newKey.key[exponent / GMP_LIMB_BITS] = (mp_limb_t) 1 << (exponent
			% GMP_LIMB_BITS);

	return newKey;
}

//--------------------------------------------------------------------
// private methods (mostly inlines)
//--------------------------------------------------------------------

// trims a key after each operation
inline void Identifier::trim() {
	key[array_size] = ~0;
	key[array_size - 1] &= GMP_MSB_MASK;
}

// compares this key to any other
int Identifier::compareTo(const Identifier& compKey) const {

	if( compKey.isUnspec == false && isUnspec == false )
		return mpn_cmp( key, compKey.key, aSize );
	else if( compKey.isUnspec == true && isUnspec == true )
		return 0;
	else
		return -1;
}

}} // namespace ariba, common
