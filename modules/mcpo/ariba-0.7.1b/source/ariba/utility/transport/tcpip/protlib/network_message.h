/// ----------------------------------------*- mode: C++; -*--
/// @file network_message.h
/// generic class for network messages (PDUs coming from/going to network)
/// ----------------------------------------------------------
/// $Id: network_message.h 3247 2008-07-28 20:54:54Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/network_message.h $
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

/** @ingroup protlib
 *
 * This is a generic class for network messages. 
 * A network message consists of a number of bytes.
 * There are no access member functions. So the user has full control over
 * the buffer.
 */
 
#ifndef _PROTLIB__NETWORK_MESSAGE_H_
#define _PROTLIB__NETWORK_MESSAGE_H_

#include <netinet/in.h>
#include <string>
#include <ostream>

#include "protlib_types.h"

namespace protlib {

/** @addtogroup protlib
 * @ingroup protlib
 * @{
 */

class NetMsgError : public ProtLibException {
public:
	enum error_t {
		ERROR_TOO_LONG,
		ERROR_NO_MEM,
		ERROR_INVALID_POS,
		ERROR_NULL_POINTER,
		ERROR_INVALID_START_OFFSET,
		ERROR_TOO_SHORT,
		ERROR_INVALID_BUFSIZE
	};
	NetMsgError(error_t e);
	const char * getstr() const;
	virtual const char *what() const throw() { return getstr(); }
	const error_t err;
private:
	static const char* const errstr[];
}; // end NetMsgError

/// network message
/** This class is used to exchange data between signalling and transport
* protocol.
*/
class NetMsg {
public:
	static const uint32 max_size;
	/// constructor
	NetMsg(uint32 s);
	/// constructor
	NetMsg(uchar *b, uint32 s, bool copy = true);
	/// copy constructor
	NetMsg(const NetMsg& n);
	/// destructor
	~NetMsg();
	/// get buffer size
	uint32 get_size() const;
	/// get bytes left until buffer ends
	uint32 get_bytes_left() const;
	/// get pointer offset
	uint32 get_pos() const;
	/// move pointer to offset
	NetMsg& set_pos(uint32 p);
	/// move pointer relative
	NetMsg& set_pos_r(int32 rp);
	/// set pointer to beginning
	NetMsg& to_start();
	/// copy into NetMsg buffer
	uint32 copy_from(const uchar *b, uint32 n);
	/// copy into NetMsg buffer
	uint32 copy_from(const uchar *b, uint32 start, uint32 end);
	/// copy from NetMsg buffer
	uint32 copy_to(uchar *b, uint32 n) const;
	/// copy from NetMsg buffer
	uint32 copy_to(uchar *b, uint32 start, uint32 n) const;
	/// get pointer to buffer
	uchar* get_buffer() const;
	/// decode uint8
	uint8 decode8(bool move = true);
	/// decode uint16
	uint16 decode16(bool move = true);
	/// decode uint32
	uint32 decode32(bool move = true);
	/// decode uint64
	uint64 decode64(bool move = true);
	/// decode uint128
	uint128 decode128(bool move = true);
	/// encode uint8
	void encode8(uint8 i, bool move = true);
	/// encode uint16
	void encode16(uint16 i, bool move = true);
	/// encode uint32
	void encode32(uint32 i, bool move = true);
	/// encode uint64
	void encode64(uint64 i, bool move = true);
	/// encode uint128
	void encode128(uint128 i, bool move = true);
	/// decode uchars
	void decode(uchar *c, uint32 len, bool move = true);
	/// encode uchars
	void encode(const uchar *c, uint32 len, bool move = true);
	/// decode string
	uint32 decode(string& s, uint32 len, bool move = true);
	/// encode string
	uint32 encode(const string& s, bool move = true);
	/// decode IPv4
	void decode(struct in_addr& in, bool move = true);
	/// encode IPv4
	void encode(const struct in_addr& in, bool move = true);
	/// decode IPv6
	void decode(struct in6_addr& in, bool move = true);
	/// encode IPv6
	void encode(const struct in6_addr& in, bool move = true);
        /// truncate buffer
	uint32 truncate();
	/// truncate buffer
	uint32 truncate(uint32 t);
	/// apply padding
	void padding(uint32 len, bool move = true);
	/// test for equality
	bool operator==(const NetMsg& n) const;
	/// encode a NetMsg into this NetMsg
	void encode(const NetMsg& m, uint32 len, bool move = true);
	/// decode a NetMsg from this NetMsg
	void decode(NetMsg& m, bool move = true);
	/// print a raw hexdump of the buffer
	ostream& hexdump(ostream& os, uchar *startpos=0, uint32 length=0) const;

private:
	/// buffer for data
	uchar *buf;
	/// buffer size
	uint32 buf_len;     
	/// current reading/writing position
	uchar *pos;
	/// buffer end
	/** Ponter to the last byte of the buffer. */	
	uchar *buf_end;
};

inline std::ostream &operator<<(std::ostream &out, const NetMsg &msg) {
	msg.hexdump(out);
	return out;
}

//@}

} // namespace protlib

#endif // _PROTLIB__NETWORK_MESSAGE_H_
