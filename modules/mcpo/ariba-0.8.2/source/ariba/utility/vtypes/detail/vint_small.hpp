// vint_small.hpp, created on 01.12.2008 by Sebastian Mies
// [The FreeBSD Licence]
//
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

#ifndef VINT_SMALL_HPP_
#define VINT_SMALL_HPP_

#include<boost/mpl/if.hpp>
#include<boost/integer.hpp>

#include "helper.hpp"
#include <cmath>

#define NO_INT64

using boost::mpl::if_;
using boost::mpl::bool_;

namespace _vint {
namespace detail {
	template<size_t l, bool s> class vint_small;
}} // namespace _vint::detail

template<size_t l, bool s>
std::ostream& operator<<(std::ostream& os, const _vint::detail::vint_small<l, s>& v);

namespace _vint {
namespace detail {

/**
 * This class implements a integer wrapper for native integer types
 * and adapts it to integers which do not have a length divideable by
 * a power of 2. In this case bits are masked out to mimic the behaviour
 * of a real variable small integer.
 *
 * SIGNED values are currently unsupported!!!
 *
 * @autor Sebastian Mies
 */
template<size_t __length = 0, bool __sign = false>
class vint_small {
	friend std::ostream& operator<< <__length, __sign> (std::ostream&,
			const vint_small<__length, __sign>&);
private:
	/* selection of integer type (beware: nasty mpl stuff :) */
typedef
#ifndef NO_INT64
	typename if_<
	bool_<(__length> 32 && __length <= 64)>,
	typename if_<bool_<__sign>, boost::int64_t, boost::uint64_t>::type,
#endif
#ifndef NO_INT32
	typename if_<
	bool_<(__length> 16 && __length <= 32)>,
	typename if_<bool_<__sign>, boost::int32_t, boost::uint32_t>::type,
#endif
	typename if_<
	bool_<(__length> 8 && __length <= 16)>,
	typename if_<bool_<__sign>, boost::int16_t, boost::uint16_t>::type,
	typename if_<
	bool_<(__length> 0 && __length <= 8)>,
	typename if_<bool_<__sign>, boost::int8_t, boost::uint8_t>::type,
	void
>	::type
#ifndef NO_INT32
>	::type
#endif
#ifndef NO_INT64
>	::type
#endif
>	::type type;

	/* THE actual value */
	type value;

	/* this method trims an integer if neccessary */
	finline void trim() {
		value &= (((type)1) << __length) - 1;
	}

	/* our internal small int */
	typedef vint_small<__length,__sign> _self;
public:
	finline vint_small() {
		value = 0;
	}

	finline vint_small( const type& value ) {
		this->value = value;
		trim();
	}

	finline vint_small(const char* text, int base = 10) {
		assign(std::string(text),base);
	}

	//-- conversion -----------------------------------------------------------

	template<class T>
	finline void convert_to( T& v, if_integral(T) ) const {
		v = (T)value;
	}

	finline std::string to_string(int base = 10,
			int str_size = 0, char str_fill = '0') const {

		// convert to another base
		std::string s = "\0";
		int last_non_zero = 0;
		int last = 0;
		type _val = value;

		while (_val != 0) {
			// divide by base
			type rem = _val % base;
			type _div = _val / base;

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

	std::string to_debug_string(int base = 10,
			int str_size = 0, char str_fill = '0') const {
		std::ostringstream str;
		str << "vint_small(static,";
		str << "length=" << __length << ",";
		str << "mem=" << sizeof(*this) << ",";
		str << "val=" << to_string(base, str_size, str_fill) << ")";
		return str.str();
	}

	//-- sub integers ---------------------------------------------------------

	finline bool get_bit(size_t index) const {
		return false;
	}

	finline void set_bit(size_t index, bool v) {
	}

	finline void set_subint(_self& v, size_t index) {
	}

	finline void set_subint(uintmax_t v, size_t index) {
	}

	finline _self get_subint(size_t index, size_t length) const {
	}

	finline uintmax_t get_subint(size_t index) const {
		return 0;
	}

	//-- assignment -----------------------------------------------------------

	template<typename T>
	finline void assign(const T& v, if_integral(T) ) {
		value = v;
	}

	finline void assign(const std::string& text, int base = 10) {
		int x;
		sscanf(text.c_str(), "%d", &x);
		value = x;
	}

	//-- compare operations ---------------------------------------------------

	finline int compare_to(const _self& v) const {
		return (int)(value-v.value);
	}

	template<class T>
	finline int compare_to(const T& v, if_integral(T) ) const {
		return (int)(value-v);
	}

	//-- integer arithmetic ---------------------------------------------------

	// addition
	template<class T>
	finline void add(const T& rhs, if_integral(T) )	{
		value += rhs; trim();
	}

	finline void add(const _self& rhs) {
		value += rhs.value; trim();
	}

	// subtraction
	template<class T>
	finline void sub(const T& rhs, if_integral(T) ) {
		value -= rhs; trim();
	}

	finline void sub(const _self& rhs) {
		value -= rhs.value; trim();
	}

	// multiplication
	template<class T>
	finline void mul(const T& rhs, if_integral(T) ) {
		value *= rhs; trim();
	}

	finline void mul(const _self& rhs) {
		value *= rhs.value; trim();
	}

	finline void mul(_self& res_l, _self& res_h, _self& rhs) {
		res_l.value = rhs.value * value;
		res_h.value = (rhs.value >> __length) * value;
	}

	// modulo
	template<class T>
	finline void mod(const T& rhs, if_integral(T) ) {
		value %= rhs; trim();
	}

	finline void mod( const _self& rhs ) {
		value %= rhs.value; trim();
	}

	finline void mod( _self& res, const _self& rhs ) {
		res.value = value % rhs.value; res.trim();
	}

	// exception to modulo
	template<class T>
	finline void mod( T& res, const T& rhs, if_integral(T) ) const {
		res = value % rhs;
	}

	// division
	template<class T>
	finline void div( const T& rhs, if_integral(T) ) {
		value /= rhs; trim();
	}

	finline void div( const _self& rhs ) {
		value /= rhs.value; trim();
	}

	finline void div( _self& res, const _self& rhs ) {
		res.value = value / rhs.value; res.trim();
	}

	// division with remainder
	template<class T>
	finline void divrem(_self& res_div, _self& res_rem, const T& rhs, if_integral(T)) {
		res_div.value = value / rhs;
		res_rem.value = value % rhs;
	}

	finline void divrem(_self& res_rem, const _self& rhs) {
		value = value / rhs.value;
		res_rem.value = value % rhs.value;
	}

	finline void divrem(_self& res_div, _self& res_rem, const _self& rhs) {
		res_div.value = value / rhs.value;
		res_rem.value = value % rhs.value;
	}

	// exception to division with remainder
	template<class T>
	finline void divrem(_self& res_div, T& res_rem, const T& rhs, if_integral(T)) {
		res_div.value = value / rhs;
		res_rem = value % rhs;
	}

	// logarithm to the base of 2
	finline uintmax_t log2() const {
		return ::log2(value);
	}

	//-- logical bit operations -----------------------------------------------

	finline void or_ (const _self& rhs) {value |= rhs.value;}
	finline void and_(const _self& rhs) {value &= rhs.value;}
	finline void xor_(const _self& rhs) {value ^= rhs.value;}
	finline void complement() {value = ~value; trim();}
	finline void lshift(size_t steps) {value <<= steps; trim();}
	finline void rshift(size_t steps) {value >>= steps; trim();}

	//-- integer dynamic ------------------------------------------------------

	//	inline vint<0, __sign> normalized() const { return value.normalized(); }

	//-- general information --------------------------------------------------

	finline size_t length() const {return __length;}
	finline void set_length(size_t length) {}
	finline bool is_unspecified() {return false;}

	finline _self max() const {return ~(type)0;}
	finline _self min() const {return (type)0;}
	finline _self zero() const {return (type)0;}
};

}} // namespace _vint::detail

template<size_t l, bool s>
std::ostream& operator<<(std::ostream& os, const _vint::detail::vint_small<l, s>& v) {
	return os << v.to_string();
}

#endif /* VINT_SMALL_HPP_ */
