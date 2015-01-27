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

#ifndef UTILITIES_HPP_
#define UTILITIES_HPP_

//#include "ariba/config.h"
#include <boost/cstdint.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

/* force inline macro */
#ifndef finline
	#ifdef HAVE_MAEMO
		#define finline inline
	#else
		#define finline inline __attribute__((always_inline))
	#endif
#endif

/* check whether X is a base class of Y */
#define if_is_base_of(X,Y) \
	typename boost::enable_if<boost::is_base_of<X, Y> , int>::type __v = 0

/* check whether X is a unsigned integer */
#define if_uint(X) \
	typename boost::enable_if<boost::is_integral<X>,int>::type __i##X = 0,\
	typename boost::enable_if<boost::is_unsigned<X>,int>::type __u##X = 0

/* check whether X is a signed integer */
#define if_int(X) \
	typename boost::enable_if<boost::is_integral<X>,void*>::type __i##X = NULL,\
	typename boost::enable_if<boost::is_signed<X>,void*>::type __s##X = NULL

/* signature conversion */
#define CONVERT_SIGN(X,Y) \
	finline X& _unsigned( Y& v ) { return *((X*)&v); } \
	finline X& _unsigned( X& v ) { return v; } \
	finline Y& _signed( X& v ) { return *((Y*)&v); } \
	finline Y& _signed( Y& v ) { return v; }

CONVERT_SIGN( uint8_t, int8_t );
CONVERT_SIGN( uint16_t, int16_t );
CONVERT_SIGN( uint32_t, int32_t );
CONVERT_SIGN( uint64_t, int64_t );

#endif /* UTILITIES_HPP_ */
