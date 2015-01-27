/// ----------------------------------------*- mode: C++; -*--
/// @file protlib_types.h
/// This file contains various typedefs
/// ----------------------------------------------------------
/// $Id: protlib_types.h 3064 2008-07-02 08:05:18Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/protlib_types.h $
// ===========================================================
//                      
// Copyright (C) 2005-2007, all rights reserved by
// - Institute of Telematics, Universitaet Karlsruhe (TH)
//
// More information and contact:
// https://projekte.tm.uka.de/trac/NSIS
//                      
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ===========================================================
// ----------------------------------------*- mode: C++; -*--
// protlib_types.h - various typedefs
// ----------------------------------------------------------
// $Id: protlib_types.h 3064 2008-07-02 08:05:18Z bless $
// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/protlib_types.h $
// ==========================================================
//                      
// (C)opyright, all rights reserved by
// - Institute of Telematics, University of Karlsruhe (TH)
// ==========================================================

/**
 * @ingroup protlib
 *
 */

#ifndef PROTLIB__TYPES_H
#define PROTLIB__TYPES_H

#include <iostream>
#include <fstream>
#include <exception>

#include <cassert>

#include <netinet/in.h> // required for IPPROTO constants

#include <sys/types.h>

// What's this used for? If if it's not needed, please delete it.
#define _THREADS


namespace protlib {
  using namespace std;
  using namespace __gnu_cxx;

/**
 * @addtogroup protlib
 * @{
 */
 
/**
 * The abstract base class for all exceptions thrown by protlib.
 */
class ProtLibException : public std::exception {
  public:
	virtual ~ProtLibException() throw() { }

	/**
	 * Get a printable string representation of the error.
	 *
	 * @warning Note that the data this pointer refers to still belongs
	 * to the exception object. It is only valid as long as the exception
	 * object exists.
	 *
	 * @return the error message
	 */
	virtual const char *what() const throw() { return error_msg.c_str(); }

	/**
	 * Deprecated: Use what() instead.
	 */
	virtual const char *getstr() const { return what(); }


  protected:
	ProtLibException() throw() { }
	ProtLibException(std::string msg) throw() : error_msg(msg) { }

	std::string error_msg;
};

inline ostream& operator<<(ostream& os, const ProtLibException &err) {
	return os << err.what();
}


typedef unsigned char       uchar;

typedef char                int8;
typedef unsigned char       uint8;

typedef short int           int16;
typedef unsigned short int  uint16;

// the following types depend on the platform
// since types.h tries to figure out the correct sizes already
// we will not replicate the stuff here. Note that on 64-bit
// platforms usually int == 32-bit, long == 64-bit

typedef int32_t             int32;
typedef u_int32_t           uint32;

typedef int64_t             int64;
typedef u_int64_t           uint64;


class uint128 {
public:
	uint32 w1;
	uint32 w2;
	uint32 w3;
	uint32 w4;
  uint128() : w1(0),w2(0),w3(0),w4(0) {};
  uint128(uint32 w1, uint32 w2, uint32 w3, uint32 w4) : w1(w1),w2(w2),w3(w3),w4(w4) {}; 
  bool operator==(const uint128& val) const { return w1==val.w1 && w2==val.w2 && w3==val.w3 && w4==val.w4; }
};


/// Network prefix length.
typedef uint8 prefix_length_t;

/// Protocol number, as it is given in an IP header.
typedef uint8 protocol_t;

/** these are pseudo protocol IDs in order to being able to perform
 * multiplexing based on on address object alone
 * currently used for Query encapsulation and TLS/TCP
 * this should be changed in the future, probably by using an additional
 * attribute in the appladdress object 
 **/
const protocol_t prot_tls_tcp    = 254;
const protocol_t prot_query_encap= 255;
const protocol_t prot_tcp        = IPPROTO_TCP;
const protocol_t prot_udp        = IPPROTO_UDP;
const protocol_t prot_sctp       = IPPROTO_SCTP;

/// Port number, as given in TCP or UDP headers.
typedef uint16 port_t;

/// A general purpose ID type.
typedef uint64 gp_id_t;

/// Catch everything, do nothing.
#define catch_all(x) try { x; } catch(...) { }

// @}

} // namespace protlib

#endif // PROTLIB__TYPES_H
