// vint_big.hpp, created on 04.11.2008 by Sebastian Mies
//
// [The FreeBSD Licence]
// Copyright (c) 2008
//     Sebastian Mies, Institute of Telematics, Universität Karlsruhe (TH)
//     All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of the author nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#ifndef VINT_BIG_HPP_
#define VINT_BIG_HPP_

#include <memory>

namespace _vint {
namespace detail {
	//forward declarations
	template<size_t l, bool s>
	class vint_big;
}} // namespace _vint::detail

template<size_t l, bool s>
std::ostream& operator<<(std::ostream&, const _vint::detail::vint_big<l,s>& v);

// utility includes
#include "helper.hpp"
#include "../varray.hpp"

// std lib includes
#include <cmath>
#include <string.h>
#include <typeinfo>
#include <iostream>
#include <sstream>

// gnu mp library include
#include <gmp.h>

namespace _vint { namespace detail {

/**
 * This class implements an adaptive integer type.
 *
 * SIGNED values are currently unsupported!!!
 *
 * @author Sebastian Mies
 */
template<size_t __length = 0, bool __sign = false>
class vint_big {
	friend class vint_big<> ;
	friend std::ostream& operator<< <__length,__sign>
		(std::ostream&, const vint_big<__length,__sign>&);
private:

	/* own type */
	typedef vint_big<__length> _self;
	static const size_t limb_size = sizeof(mp_limb_t) * 8;

	/* adaptive identifier storage */
	varray<mp_limb_t, __length> mp;

	/* this method masks off the most significant bits which are not needed */
	finline void trim() {
		const size_t bitp = length() & (sizeof(mp_limb_t) * 8 - 1);
		if (bitp != 0) {
			const size_t indexp = length() / (sizeof(mp_limb_t) * 8);
			array()[indexp] &= (((mp_limb_t) 1) << bitp) - 1;
		}
	}

public:
	/* returns the array length */
	finline size_t array_length() const {
		return mp.array_size();
	}

	/* returns the limb array */
	finline mp_limb_t* array() {
		return mp.array();
	}

	/* returns the limb array */
	finline const mp_limb_t* array() const {
		return mp.array_const();
	}
	/* returns reference to a limb */
	finline mp_limb_t& array(int index) {
		return mp.array()[index];
	}

	/* returns reference to a limb */
	finline mp_limb_t array(int index) const {
		return mp.array_const()[index];
	}

	/* sets all bits to zero */
	finline void clear() {
		for (size_t i = 0; i < array_length(); i++)
			array( i) = 0;
	}

public:
	/* construct an zero integer */
	inline vint_big() {
		clear();
	}

	template<typename T>
	inline vint_big( const T value, if_integral(T) ) {
		set_length( sizeof(T)*8 );
		clear();
		array(0) = value;
	}

	/* construct a variable integer from a given string */
	inline vint_big(const char* text, int base = 10) {
		assign(text, base);
	}

	//-- conversion -----------------------------------------------------------

	std::string to_string(int base = 10, int str_size = 0, char str_fill = '0') const {

		// alloc buffers
		size_t size = mp.array_size();
		mp_limb_t* buffer = new mp_limb_t[size * 2];
		mp_limb_t* _div = buffer;
		mp_limb_t* _val = buffer + size;

		// copy value
		memcpy(_val, mp.array_const(), size * sizeof(mp_limb_t));

		// convert to another base
		std::string s = "\0";
		int last_non_zero = 0;
		int last = 0;

		while (size != 0) {
			// evaluate highest non-zero limb
			while (size != 0 && _val[size - 1] == 0)
				size--;

			// divide by base
			mp_limb_t rem = mpn_divrem_1(_div, 0, _val, size, base);

			// set char
			s += (char) rem;
			if (rem != 0) last_non_zero = last;
			last++;

			// divided value as new
			_val = _div;
		}
		last = last_non_zero + 1;

		// reverse
		for (int i = 0; i < last / 2; i++) {
			char dummy = s[i];
			s[i] = s[last - i - 1];
			s[last - i - 1] = (int) dummy;
		}

		// convert string
		s.resize(last);
		for (int i = 0; i < last; i++)
			s[i] = ("0123456789ABCDEF")[(int) s[i]];

		// free buffer
		delete buffer;

		// add leading zeros
		if (last < str_size) {
			char zeros[str_size - last+1];
			memset(zeros, str_fill, str_size - last);
			zeros[str_size - last] = 0;
			return std::string(zeros) + s;
		} else {
			return s;
		}
	}

	std::string to_debug_string(int base = 10, int str_size = 0, char str_fill =
			'0') const {
		std::ostringstream str;
		str << "vint_big(" << (mp.is_dynamic() ? "dynamic" : "static ") << ",";
		str << "length=" << length() << ",";
		str << "mem=" << mp.get_memory_consumption() << ",";
		str << "val=" << to_string(base, str_size, str_fill) << ")";
		return str.str();
	}

	template<typename T>
	inline void convert_to(T& value, if_integral(T) ) const {
		value = (T)array(0);
	}

	template<size_t _l, bool _s>
	inline void convert_to(vint_big<_l,_s>& value) const {
		value.assign(*this);
	}

	//-- sub integers ---------------------------------------------------------
public:

	inline bool get_bit( size_t index ) const {
		size_t aidx = index / limb_size;
		size_t bidx = index % limb_size;
		return (array()[aidx] >> bidx) & 1;
	}

	inline void set_bit( size_t index, bool value ) {
		size_t aidx = index / limb_size;
		size_t bidx = index % limb_size;
		array()[aidx] &= ~(1 << bidx);
		array()[aidx] |=  (value << bidx);
	}

	inline void set_subint( _self& value, size_t index ) {

	}

	template<typename X>
	inline void set_subint( X& value, size_t index ) {

	}

	inline _self get_subint( size_t index, size_t length ) const {
		return 0;
	}

	inline uintmax_t get_subint( size_t index ) const {
		return 0;
	}

	//-- assignment -----------------------------------------------------------

public:

	template<size_t _len, bool _sign>
	inline void assign(const vint_big<_len,_sign>& cpy) {
		vint_big<_len,_sign>& copy = const_cast<vint_big<_len,_sign>&> (cpy);
		mp.resize( copy.length() );
		clear();
		size_t len = std::min(array_length(), copy.array_length());
		for (size_t i = 0; i < len; i++)
			array(i) = copy.array(i);
		trim();
	}

	template<typename T>
	inline void assign( const T& value, if_integral(T) ) {
		set_length( sizeof(T) * 8 );
		clear();
		bitcpy(value, 0, array(), 0, sizeof(T) * 8);
	}

	inline void assign( const std::string& text, int base = 10) {
		std::string s = text;
		size_t blen = ::log2(base) * s.size() + 1;
		if ((blen % 8) != 0) blen += 8 - (blen % 8);
		set_length(blen);
		clear();
		for (size_t i = 0; i < s.size(); i++) {
			if ((s[i] >= '0') && (s[i] <= '9')) s[i] -= '0';
			else if ((s[i] >= 'a') && (s[i] <= 'z')) s[i] -= ('a' - 10);
			else if ((s[i] >= 'A') && (s[i] <= 'Z')) s[i] -= ('A' - 10);
		}
		mpn_set_str(array(), (unsigned char*) s.c_str(), s.size(), base);
	}

	//-- compare operations ---------------------------------------------------

	inline int compare_to(const _self& v) const {
		return mpn_cmp(array(), v.array(), array_length());
	}

	template<class T>
	inline int compare_to(const T& v, if_integral(T) ) const {
		int i = array_length();
		while (i != 0 && array(i - 1) == 0)
			i--;
		return (i > 1) ? 1 : (int) (array(0) - v);
	}

	//-- arithmetic operations ------------------------------------------------

public:

	finline void add(const _self& v) {
		mpn_add_n(array(), array(), const_cast<_self&> (v).array(),
				array_length());
		trim();
	}

	template<class T>
	finline void add( const T& v, if_uint(T) ) {
		mpn_add_1(array(), array(), array_length(), v);
		trim();
	}

	finline void sub(const _self& v) {
		mpn_sub_n(array(), array(), const_cast<_self&> (v).array(),
				array_length());
		trim();
	}


	template<class T>
	finline T mod(const T& v, if_uint(T) ) {
		return (T) mpn_mod_1(array(), array_length(), v);
	}

private:

	finline vint_big<__length*2> mul(const _self& v) const {
		vint_big<__length * 2> result;
		result.set_length( length() );
		mpn_mul_n(result.array(), mp.array_const(), v.mp.array_const(),
				array_length());
		return result;
	}

public:

	inline uintmax_t log2() const {
		int size = array_length();
		while (size != 0 && array(size-1) == 0)	size--;
		if (size == 0) return 0;
		else return (size - 1) * sizeof(mp_limb_t) * 8 + ::log2(array(size-1));
	}

	//-- logical bit operations -----------------------------------------------

	inline void or_(const _self& v) {
		for (size_t i = 0; i < array_length(); i++)
			array()[i] |= v.array()[i];
	}

	inline void and_(const _self& v) {
		for (size_t i = 0; i < array_length(); i++)
			array()[i] &= v.array()[i];
	}

	inline void xor_(const _self& v) {
		for (size_t i = 0; i < array_length(); i++)
			array()[i] ^= v.array()[i];
	}

	inline void complement() {
		for (size_t i = 0; i < array_length(); i++)
			array()[i] = ~array()[i];
		trim();
	}

	inline void lshift(size_t steps) {
		size_t i = steps / GMP_LIMB_BITS;
		steps %= GMP_LIMB_BITS;
		if (i >= array_length()) clear();
		else {
			for (size_t j = 0; j < array_length() - i; j++)
				array()[j + i] = array()[j];
			mpn_lshift(array(), array(), array_length(), steps);
			trim();
		}
	}

	inline void rshift(size_t steps) {
		size_t i = steps / GMP_LIMB_BITS;
		steps %= GMP_LIMB_BITS;
		if (i >= array_length()) clear();
		else {
			for (size_t j = 0; j < array_length() - i; j++)
				array()[j] = array()[j + i];
			mpn_rshift(array(), array(), array_length(), steps);
			trim();
		}
	}

	//-- integer dynamic ------------------------------------------------------

	inline vint_big<> normalized() const {
		vint_big<> v;
		int width = log2();
		int a = (sizeof(mp_limb_t)*8);
		if ((width % a)!=0) width += a-(width%a);
		v.set_length( width );
		for (size_t i=0; i<v.array_length(); i++) v.array(i) = array(i);
		return v;
	}

	//-- general information --------------------------------------------------

	finline size_t length() const {
		return mp.size();
	}

	finline void set_length(size_t length) {
		mp.resize(length);
	}

	finline bool is_unspecified() {
		return (length() == 0);
	}

	/**
	 * This method returns the maximum number according to its size in bits.
	 *
	 * @return The maximum number according to its size in bits.
	 */
	finline _self max() const {
		_self v;
		v.set_length(length());
		for (size_t i=0; i<v.array_length(); i++) v.array(i) = ~0;
		v.trim();
		return v;
	}

	finline _self zero() const {
		_self v;
		v.set_length(length());
		for (size_t i=0; i<v.array_length(); i++) v.array(i) = 0;
		v.trim();
		return v;
	}

	finline _self min() const {
		return zero();
	}
};

}}

template<size_t __length, bool __sign>
std::ostream& operator<<(std::ostream& os, const _vint::detail::vint_big<__length,__sign>& v) {
	return os << v.to_string();
}


#endif /* VINTBIG_HPP_ */
