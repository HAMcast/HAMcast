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
#ifndef VINT_HPP_
#define VINT_HPP_

#include <boost/mpl/if.hpp>

#include "detail/helper.hpp"
#include "detail/vint_big.hpp"
#include "detail/vint_small.hpp"

using namespace _vint::detail;

template<size_t l, bool s> class vint;
template<size_t l, bool s>
std::ostream& operator<<(std::ostream&, const vint<l, s>& v);

/**
 * This class implements an adaptive, scalable integer type.<br />
 *
 * Key features include:<br />
 * <ul>
 * 	<li>Transparent type switching according to native types</li>
 * 	<li>Optimal memory consumption and optimization properties</li>
 * 	<li>Automatic big integer arithmetic using gmp or other libraries</li>
 * 	<li>Unconventional integer types (e.g. length not a power of 2)</li>
 * 	<li>Additional math, like intervals, log2, divrem, gcd, ...</li>
 * </ul>
 *
 * ... short: a real scalable integer type at compile-time cost!<br />
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
template<size_t __length = 0, bool __sign = false>
class vint {
	// stream operator
	friend std::ostream& operator<< <__length, __sign> (std::ostream&,
			const vint<__length, __sign>&);

public:
	// select proper type
	typedef	typename boost::mpl::if_<
		boost::mpl::bool_<(__length> 64 || __length == 0)>,
		vint_big<__length, __sign>, vint_small<__length, __sign>
		>::type type;

	// selected value type
	type value;

	// internal self-template
	typedef vint<__length, __sign> _self;

public:
	/* construct an zero integer */
	finline vint() : value() {}

	/* construct an integer from a constant */
	template<typename T> finline vint(const T v, if_integral(T) ) : value(v) {}

	/* construct a variable integer from a given string */
	finline vint(const char* text, int base = 10) : value(text,base) {}

	/* copy constructor */
	finline vint( const type& v ) {value = v;}

	/* copy constructor */
	finline vint( const _self& v ) {value = v.value;}

	//-- conversion -----------------------------------------------------------

	template<size_t ___length, bool ___sign>
	finline void convert_to(vint<___length, ___sign>& v ) const {
		return value.convert_to(v.value);
	}

	template<class T>
	finline void convert_to( T& v, if_integral(T) ) const {
		return value.convert_to(v);
	}

	finline std::string to_string(int base = 10,
			int str_size = 0, char str_fill = '0') const {
		return value.to_string(base,str_size,str_fill);
	}

	std::string to_debug_string(int base = 10,
			int str_size = 0, char str_fill = '0') const {
		return value.to_debug_string(base,str_size,str_fill);
	}

	//-- sub integers ---------------------------------------------------------

	finline bool get_bit(size_t index) const {
		return value.get_bit(index);
	}

	finline void set_bit(size_t index, bool v) {
		value.set_bit(index, v);
	}

	finline void set_subint(_self& v, size_t index) {
		value.set_subint(v.value,index);
	}

	finline void set_subint(uintmax_t v, size_t index) {
		value.set_subint(v,index);
	}

	finline _self get_subint(size_t index, size_t length) const {
		return value.get_subint(index,length);
	}

	finline uintmax_t get_subint(size_t index) const {
		return value.get_subint(index);
	}

	//-- assignment -----------------------------------------------------------

public:

	template<size_t _len, bool _sign>
	finline void assign( const vint<_len,_sign>& v ) {
		value.assign(v.value);
	}

	template<typename T>
	finline void assign(const T& v, if_integral(T) ) {
		value.assign(v);
	}

	finline void assign(const std::string& text, int base = 10) {
		value.assign( text, base );
	}

	void assign(const char*& text, int base = 10) {
		assign(std::string(text), base);
	}

	//-- compare operations ---------------------------------------------------

	finline int compare_to(const _self& v) const {
		return value.compare_to(v.value);
	}

	template<class T>
	finline int compare_to(const T& v, if_integral(T) ) const {
		return value.compare_to(v);
	}

	//-- arithmetic operations ------------------------------------------------

	// addition
	template<class T>
	finline void add(const T& rhs, if_integral(T) ) {value.add(rhs.value);}
	finline void add(const _self& rhs) {value.add(rhs.value);}

	// subtraction
	template<class T>
	finline void sub(const T& rhs, if_integral(T) ) {value.sub(rhs.value);}
	finline void sub(const _self& rhs) {value.sub(rhs.value);}

	// multiplication
	template<class T>
	finline void mul( const T& rhs, if_integral(T) ) {value.mul(rhs);}
	finline void mul( _self& rhs) {value.mul(rhs.value);}
	finline void mul( _self& res, const _self& rhs ) const {value.mul(res.value,rhs.value);}

	// division
	template<class T>
	finline void div( const T& rhs, if_integral(T) ) {value.div(rhs);}
	finline void div(_self& rhs) {value.div(rhs.value);}
	finline void div(_self& res, const _self& rhs ) const {value.div(res.value, rhs);}

	// modulo
	template<class T>
	finline void mod( const T& rhs, if_integral(T) ) {value.mod(rhs);}
	finline void mod(_self& rhs) {value.mod(rhs.value);}
	finline void mod(_self& res, const _self& rhs ) const {value.mod(res.value, rhs);}

	// exception of modulo: result may be of integral type
	template<class T>
	finline void mod( T& res, const T& rhs, if_integral(T) ) const {value.mod(res,rhs);}

	// division with remainder
	template<class T>
	finline void divrem(_self& res_div, T& res_rem, const T& rhs, if_integral(T) ) const {
		value.divrem(res_div.value, res_rem, rhs );
	}
	finline void divrem(_self& res_div, _self& res_rem, const _self& rhs ) const {
		value.divrem(res_div.value, res_rem.value, rhs.value);
	}
	template<class T>
	finline void divrem( T& res_rem, const T& rhs, if_integral(T) ) {
		value.div(res_rem, rhs.value);
	}
	finline void divrem( _self& res_rem, const _self& rhs ) {
		value.div(res_rem.value, rhs.value);
	}

	// logarithm to the base of 2
	finline uintmax_t log2() const {return value.log2();}

	//-- logical bit operations -----------------------------------------------

	finline void or_ (const _self& v) {value.or_(v.value);}
	finline void and_(const _self& v) {value.and_(v.value);}
	finline void xor_(const _self& v) {value.xor_(v.value);}
	finline void complement() {value.complement();}
	finline void lshift(size_t steps) {value.lshift(steps);}
	finline void rshift(size_t steps) {value.rshift(steps);}

	//-- integer dynamic ------------------------------------------------------

	inline _self normalized() const {return value.normalized();}

	//-- general information --------------------------------------------------

	finline size_t length() const {return value.length();}
	finline void set_length(size_t length) {value.set_length(length);}
	finline bool is_unspecified() {return value.is_unspecified();}

	finline _self max() const {_self v; v.value = value.max(); return v;}
	finline _self min() const {_self v; v.value = value.min(); return v;}
	finline _self zero() const {_self v; v.value = value.zero(); return v;}

	//-- general extensions ---------------------------------------------------

	finline void convert_to( std::string& str ) const {
		str = to_string();
	}

	/**
	 * Returns true, if the integer is inside the interval [a,b]
	 */
	finline bool is_between(const _self& a, const _self& b) const {
		return (a <= b) ?
		((*this >= a) && (*this <= b))
		:
		((*this <= a) && (*this >= b))
		;
	}

	/**
	 * Returns true, if the integer is inside the interval [a,b] on
	 * a ring.
	 */
	finline bool is_between_ring(const _self& a, const _self& b) const {
		return (b<a) ?
		(is_between(a, a.max())||is_between(b.zero(), b))
		:
		((*this >= a) && (*this <= b))
		;
	}

	//-- convenience operator overloads ---------------------------------------

	// conversion operators
	template<typename T> finline T convert() const {T v;convert_to(v);return v;}
	template<typename T> finline operator T () const {return convert<T>();}

	// assign operators
	template<class T>
	finline _self& operator=(const T& copy) {assign(copy);return *this;}

	// add operators
	template<class T>
	finline _self& operator+=(const T& v) {add(v);return *this;}
	template<class T>
	finline _self& operator++() {add(1);return *this;}
	template<class T>
	finline _self operator+ (const T& v) const {_self x=*this;x.add(v);return x;}

	// substract operators
	template<class T>
	finline _self& operator-=(const T& v) {sub_model(v);return *this;}
	finline _self& operator--() {sub(1);return *this;}
	template<class T>
	finline _self operator- (const T& v) const {
		_self x(*this); x.sub_model(v); return x;
	}

	// multiply operators
	template<class T> finline _self& operator*=(const T& v) {
		value.mul(v);
		return *this;
	}
	template<class T> finline _self operator*(const T& v) const {
		_self res = *this;res.mul(v); return res;
	}

	// bit shift operators convenience
	finline _self operator<< (size_t steps) {_self x=*this;return(x<<=steps);}
	finline _self& operator>>=(size_t steps) {rshift(steps);return *this;}
	finline _self operator>> (size_t steps) {_self x=*this;return(x<<=steps);}
	finline _self& operator<<=(size_t steps) {lshift(steps);return *this;}

	// bit operators convenience
	finline _self& operator&=(const _self& v) {and_(v); return *this;}
	finline _self operator & (const _self& v) {_self x=*this;return(x&=v);}
	finline _self& operator|=(const _self& v) {or_(v); return *this;}
	finline _self operator | (const _self& v) {_self x=*this;return(x|=v);}
	finline _self& operator^=(const _self& v) {xor_(v);return *this;}
	finline _self operator^ (const _self& v) {_self x=*this;return(x ^= v);}
	finline _self operator~() {_self x=*this;return x.complement();}

	// compare operators convenience
	template<class T>
	finline bool operator< (const T& v) const {return compare_to(v)< 0;}
	template<class T>
	finline bool operator> (const T& v) const {return compare_to(v)> 0;}
	template<class T>
	finline bool operator<=(const T& v) const {return compare_to(v)<=0;}
	template<class T>
	finline bool operator>=(const T& v) const {return compare_to(v)>=0;}
	template<class T>
	finline bool operator==(const T& v) const {return compare_to(v)==0;}
	template<class T>
	finline bool operator!=(const T& v) const {return compare_to(v)!=0;}
};

template<size_t l, bool s>
std::ostream& operator<<(std::ostream& os, const vint<l, s>& v) {
	return os << v.value;
}

#endif /* VINT_HPP_ */

