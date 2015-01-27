// helper.hpp, created on 04.11.2008 by Sebastian Mies
//
// [The FreeBSD Licence]
// Copyright (c) 2008
// Sebastian Mies, Institute of Telematics, Universität Karlsruhe (TH)
// All rights reserved.
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
#ifndef HELPER_HPP_
#define HELPER_HPP_

#include <boost/cstdint.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/if.hpp>

/* force inline macro */
#ifndef finline
	#define finline inline __attribute__((always_inline))
#endif

/* check whether X is a base class of Y */
#define if_is_base_of(X,Y) \
	typename boost::enable_if<boost::is_base_of<X, Y> , int>::type __v = 0

/* check whether X is a integer */
#define if_integral(X) \
	typename boost::enable_if<boost::is_integral<X>,int>::type __i##X = 0

/* check whether X is a unsigned integer */
#define if_uint(X) \
	typename boost::enable_if<boost::is_integral<X>,int>::type __i##X = 0,\
	typename boost::enable_if<boost::is_unsigned<X>,int>::type __u##X = 0

/* check whether X is a signed integer */
#define if_int(X) \
	typename boost::enable_if<boost::is_integral<X>,int>::type __i##X = 0,\
	typename boost::enable_if<boost::is_signed<X>,int>::type __s##X = 0

/* signature conversion */
#define CONVERT_SIGN(X,Y) \
	finline X& _unsigned( Y& v ) { return *((X*)&v); } \
	finline X& _unsigned( X& v ) { return v; } \
	finline Y& _signed( X& v ) { return *((Y*)&v); } \
	finline Y& _signed( Y& v ) { return v; }

CONVERT_SIGN( uint8_t, int8_t );
CONVERT_SIGN( uint16_t, int16_t );
CONVERT_SIGN( uint32_t, int32_t );

/* bijective integer conversion */
#define CONVERT_TO_TYPE( __method, __type ) \
	template<class T> __type& __method(T& x, typename \
	boost::enable_if< boost::mpl::bool_<sizeof(T) == sizeof(__type)>, int> \
	::type __ix = 0) { return (__type&) *(&x);}

CONVERT_TO_TYPE( _uint, uint8_t  )
CONVERT_TO_TYPE( _uint, uint16_t )
CONVERT_TO_TYPE( _uint, uint32_t )

CONVERT_TO_TYPE( _int , int8_t   )
CONVERT_TO_TYPE( _int , int16_t  )
CONVERT_TO_TYPE( _int , int32_t  )

#endif /* HELPER_HPP_ */
