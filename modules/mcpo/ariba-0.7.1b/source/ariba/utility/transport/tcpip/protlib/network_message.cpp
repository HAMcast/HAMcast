/// ----------------------------------------*- mode: C++; -*--
/// @file network_message.cpp
/// generic class for network messages represented as byte stream
/// ----------------------------------------------------------
/// $Id: network_message.cpp 3247 2008-07-28 20:54:54Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/network_message.cpp $
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
 * This is a generic class for network messages,
 * represented as a byte stream (buffer containing bytes).
 */

#include <netinet/in.h> 
#include <string.h>
#include <cctype>
#include <fstream>
#include <ostream>
#include <sstream>
#include <iomanip>

#include "network_message.h"

#include "logfile.h" // only required for color definitions

namespace protlib {


  using namespace protlib::log;
/** @addtogroup protlib
 * @{
 */

/** @param e error code */
NetMsgError::NetMsgError(error_t e) : err(e) {}

const char* NetMsgError::getstr() const { return errstr[err]; }

/** NetMsg error strings */
const char* const NetMsgError::errstr[] = 
{
    "Network message too long.",
    "Not enough memory to allocate network message.",
    "Operation not valid because of pointer position in NetMsg buffer.",
    "NULL pointer argument in call to NetMsg constructor.",
    "Invalid start offset.",
    "NetMsg buffer too short.",
    "Invalid buffer size: zero"
}; // end network message error strings

/** Maximum size of a network message. This is set to 128000 to prevent
 * very large messages.
 */
const uint32 NetMsg::max_size = 128000;

/** Creates a network message object of the desired size if possible.
 * @param s buffer size.
 */
NetMsg::NetMsg(uint32 s) {
  if (s>max_size) throw NetMsgError(NetMsgError::ERROR_TOO_LONG);
  if (s==0) throw NetMsgError(NetMsgError::ERROR_INVALID_BUFSIZE);
  buf = new(nothrow) uchar[s];
  if (!buf) throw NetMsgError(NetMsgError::ERROR_NO_MEM);
  memset(buf,0,s);
  buf_len = s;
  pos = buf;
  buf_end = buf+(s-1);
} // end constructor

/** Creates a network message object of the desired size if possible.
 * @param b pointer to a buffer to initialize the NetMsg.
 * @param s buffer size
 * @param copy copy the buffer or use the buffer without copying.
 */
NetMsg::NetMsg(uchar *b, uint32 s, bool copy) {
  if (s>max_size) throw NetMsgError(NetMsgError::ERROR_TOO_LONG);
  if (s==0) throw NetMsgError(NetMsgError::ERROR_INVALID_BUFSIZE);
  if (copy) {
    buf = new(nothrow) uchar[s];
    if (!buf) throw NetMsgError(NetMsgError::ERROR_NO_MEM);
    memcpy(buf,b,s);
  } else {
    buf=b;
    if (!buf) throw NetMsgError(NetMsgError::ERROR_NULL_POINTER);
  } // end if copy
  buf_len = s;
  pos = buf;
  buf_end = buf+(s-1);
} // end constructor

NetMsg::NetMsg(const NetMsg& n) {
  buf_len = n.buf_len;
  if (buf_len == 0)
    throw NetMsgError(NetMsgError::ERROR_INVALID_BUFSIZE);

  buf = new(nothrow) uchar[buf_len];
  if (!buf) throw NetMsgError(NetMsgError::ERROR_NO_MEM);
  memcpy(buf,n.buf,buf_len);
  pos = buf+(n.pos-n.buf);
  buf_end = buf+(buf_len-1);
} // end copy constructor

/** Frees the message buffer. */
NetMsg::~NetMsg() {
  if (buf) { delete[] buf; buf= 0; }
} // end destructor

/** @return the size of the network message buffer. */
uint32 NetMsg::get_size() const {
   return buf_len;
} // end get_size

/** Returns the number of bytes left, that is the number of bytes from the 
 * current position to the end of the buffer.
 */
uint32 NetMsg::get_bytes_left() const {
  if (buf<=pos) 
  {
    if (pos<=buf_end) 
      return (buf_end-pos)+1; 
    else 
      return 0;
  } 
  else /// should never happen
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end get_size

/** Returns the offset of the buffer pointer. 
 * Note that an offset of buf_len means that the pointer is one byte behind
 * the end of the buffer. This means that you reached the buffer end.
 * This leads to a get-pos result of buf_len.
 */
uint32 NetMsg::get_pos() const {
  if ((buf<=pos) && (pos<=(buf_end+1))) 
    return (pos-buf);
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end get_pos

/** Move pointer to the given offset. 
 * @param p pointer offset 
 * @note if p==buf_len then the pointer is one byte behind the buffer end.
 * This is correct and signals that all data has been read from the buffer.
 */
NetMsg& NetMsg::set_pos(uint32 p) {
  if (p<=buf_len) 
  {
    pos = buf+p;
    return *this;
  } 
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end set_pos

/** Move pointer relative to current position.
 * @param rp offset relative to current position. 
 */
NetMsg& NetMsg::set_pos_r(int32 rp) 
{
  if (((pos+rp)>=buf) && ((pos+rp)<=(buf_end+1))) {
    pos += rp;
  } 
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
  return *this;
} // end set_pos_r

/** Set current position to buffer start. */
NetMsg& NetMsg::to_start() {
  pos=buf;
  return *this;
} // end to_start

/** Copy n bytes into NetMsg buffer.
 * @param b source
 * @param n number of bytes to copy
 */
uint32 NetMsg::copy_from(const uchar *b, uint32 n) {
  return copy_from(b,0,n);
} // end copy_from

/** Copy bytes into NetMsg buffer.
 * @param b source
 * @param start offset of first byte to copy
 * @param n number of bytes to copy
 */
uint32 NetMsg::copy_from(const uchar *b, uint32 start, uint32 n) {
  if ((n+start)>buf_len) throw NetMsgError(NetMsgError::ERROR_TOO_SHORT);
  // TH: I BELIEVE THIS IS WRONG !!
  //memmove(buf,b+start,n);
  memmove(buf+start,b,n);
  return n;
} // end copy_from

/** Copy n bytes from NetMsg buffer into b. If the NetMsg buffer is smaller
 * than n, less bytes are copied. 
 * @param b destination buffer
 * @param n number of bytes to be copied
 * @return number of copied bytes
 */
uint32 NetMsg::copy_to(uchar *b, uint32 n) const {
  try {
    return copy_to(b,0,n);
  } catch(NetMsgError& e) {
    // ERROR, should not happen
  } // end try-catch
  return 0;
} // end copy_to

/** Copy n bytes from NetMsg buffer into b. If the NetMsg buffer is smaller
 * than n, less bytes are copied. 
 * @param b destination buffer
 * @param start offset into NetMsg buffer
 * @param n number of bytes to be copied
 * @return number of copied bytes
 */
uint32 NetMsg::copy_to(uchar *b, uint32 start, uint32 n) const {
  if (start>=buf_len) throw NetMsgError(NetMsgError::ERROR_INVALID_START_OFFSET);
  if ((n+start)>buf_len) n=buf_len-start;
  memmove(b,buf+start,n);
  return n;
} // end copy_to

/** returns a pointer to the NetMsg buffer. */
uchar* NetMsg::get_buffer() const {
	return buf;
} // end get_buffer



/** Decode an uint8 integer.
 * @param move determines if current position in buffer is changed or not.
 */
uint8 NetMsg::decode8(bool move) {
  register uint8 i;
  if (pos<=buf_end) 
  {
    i = *pos;
    if (move) pos+=1;
  } 
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
  
  return i;
} // end decode uint8

/** Decode an uint16 integer.
 * @param move determines if current position in buffer is changed or not.
 */
uint16 NetMsg::decode16(bool move) {
  register uint16 i;
  if ((pos+1)<=buf_end) {
    i = ntohs(*((uint16*)pos));
    if (move) pos+=2;
  } 
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
  return i;
} // end decode uint16

/** Decode an uint32 integer.
 * @param move determines if current position in buffer is changed or not.
 */
uint32 NetMsg::decode32(bool move) {
  register uint32 i;
  if ((pos+3)<=buf_end) {
    i = ntohl(*((uint32*)pos));
    if (move) pos+=4;
  } 
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
  return i;
} // end decode uint32

/** Decode an uint64 integer.
 * @param move determines if current position in buffer is changed or not.
 */
uint64 NetMsg::decode64(bool move) {
  uint64 hi = 0;
  uint64 lo = 0;
  uint64 res = 0;
  uint32* p = (uint32*)pos;
  if ((pos+7)<=buf_end) {
    hi = ntohl(*p);
    lo = ntohl(*(p+1));
    res = (hi<<32)+lo;
    if (move) pos+=8;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
  return res;
} // end decode uint64

/** Decode an uint128 integer.
 * @param move determines if current position in buffer is changed or not.
 */
uint128 NetMsg::decode128(bool move) {
  uint32 word1, word2, word3, word4;
  uint128 res;
  word1 = NetMsg::decode32();
  word2 = NetMsg::decode32();
  word3 = NetMsg::decode32();
  word4 = NetMsg::decode32();
  res.w1 = word1;
  res.w2 = word2;
  res.w3 = word3;
  res.w4 = word4;

  return res;
} // end decode uint128

/** Encode an uint8 integer.
 * @param i an uint8
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode8(uint8 i, bool move) {
  if (pos<=buf_end) {
    *pos = i;
    if (move) pos+=1;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode uint8

/** Encode an uint16 integer.
 * @param i an uint16
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode16(uint16 i, bool move) {
  if ((pos+1)<=buf_end) {
    *((uint16*)pos) = htons(i);
    if (move) pos+=2;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode uint16

/** Encode an uint32 integer.
 * @param i an uint32
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode32(uint32 i, bool move) {
  if ((pos+3)<=buf_end) {
    *((uint32*)pos) = htonl(i);
    if (move) pos+=4;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode uint32

/** Encode an uint64 integer.
 * @param i an uint64
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode64(uint64 i, bool move) {
	uint32 hi = 0;
	uint32 lo = 0;
	uint32* p = (uint32*)pos;
	if ((pos+7)<=buf_end) {
		lo = i;
		hi = (i>>32);
		*p = htonl(hi);
		*(p+1) = htonl(lo);
		if (move) pos+=8;
	} else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode uint64

/** Encode an uint128 integer.
 * @param i an uint128
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode128(uint128 i, bool move) {
  encode32(i.w1);
  encode32(i.w2);
  encode32(i.w3);
  encode32(i.w4);
} // end encode uint128

/** Decode uchars.
 * @param c pointer to uchar array
 * @param len uchar array size
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::decode(uchar *c, uint32 len, bool move) {
  if ((pos+(len-1))<=buf_end) {
    memmove(c,pos,len);
    if (move) pos+=len;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end decode uchars

/** Encode uchars.
 * @param c pointer to uchar array
 * @param len uchar array size
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode(const uchar *c, uint32 len, bool move) {
  if ((pos+(len-1))<=buf_end) {
    memmove(pos,c,len);
    if (move) pos+=len;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode uchars

/** Decode string.
 * @param s string reference
 * @param len number of bytes to decode
 * @param move determines if current position in buffer is changed or not.
 * @return string length.
 */
uint32 NetMsg::decode(string& s, uint32 len, bool move) {
  if (len==0) {
    s.clear();
    return 0;
  } else if ((pos+(len-1))<=buf_end) {
    s.clear();
    s.assign((const char*)pos,0,len);
    if (move) pos+=len;
    return s.length();
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end decode string

/** Encode string.
 * @param s string reference
 * @param move determines if current position in buffer is changed or not.
 */
uint32 NetMsg::encode(const string& s, bool move) {
  uint32 len = s.length();
  if ((pos+(len-1))<=buf_end) {
    memmove(pos,s.c_str(),len);
    if (move) pos+=len;
    return len;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode string

/** Decode an IPv4 address. 
 * @param in reference to in_addr
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::decode(struct in_addr& in, bool move) {
  //in.s_addr = decode32(move);
  if ((pos+3)<=buf_end) {
    in.s_addr = *((uint32*)pos);
    if (move) pos+=4;
  } 
  else 
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end decode(in_addr)

/** Encode an IPv4 address. 
 * @param in reference to const in_addr
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode(const struct in_addr& in, bool move) {
  //encode32(in.s_addr,move);
  if ((pos+3)<=buf_end) {
    *((uint32*)pos) = in.s_addr;
    if (move) pos+=4;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode(in_addr)

/** Decode an IPv6 address. 
 * @param in reference to in6_addr
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::decode(struct in6_addr& in, bool move) {
  if ((pos+15)<=buf_end) {
    memmove(in.s6_addr,pos,16);
    if (move) pos+=16;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end decode(in6_addr)

/** Encode an IPv6 address. 
 * @param in reference to const in6_addr
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode(const struct in6_addr& in, bool move) {
  if ((pos+15)<=buf_end) {
    memmove(pos,in.s6_addr,16);
    if (move) pos+=16;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode(in6_addr)






/** Truncates the buffer at the current position and sets the current
 * position to buffer start.
 * @return new buffer size.
 */
uint32 NetMsg::truncate() {
  if ((pos>=buf) && (pos<=(buf_end+1))) {
    buf_len = (pos-buf);
    buf_end = pos-1;
    to_start();
  } else {
    throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
  } // end if buf
  return buf_len;
} // end truncate

/** Truncates the buffer at given offset. */
uint32 NetMsg::truncate(uint32 t) {
  set_pos(t);
  return truncate();
} // end truncate

/** Set padding bytes to 0.
 * @param len padding length
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::padding(uint32 len, bool move) {
  if (len==0) return;
  else if ((pos+(len-1))<=buf_end) {
    memset(pos,0,len);
    if (move) pos+=len;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end padding

/** Two network messages are equal if their buffers and states are equal. */
bool NetMsg::operator==(const NetMsg& n) const 
{
  // buffer size equal
  if (buf_len==n.buf_len) 
  {
    // empty buffers are considered equal
    if ((buf==NULL) && (n.buf==NULL)) 
      return true; 
    else 
    if (buf && n.buf) 
    {
      // compare buffer content 
      if (memcmp(buf,n.buf,buf_len)==0) 
      {
	// last check: position must be equal
	return ((pos-buf)==(n.pos-n.buf));
      } 
      else 
	return false;
    } 
    else 
      return false;
  } 
  else 
    return false;
} // end operator==

/** Decode NetMsg.
 * @param m NetMsg reference
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::decode(NetMsg& m, bool move) {
  uint32 len = m.get_size();
  if ((pos+(len-1))<=buf_end) {
    // copy buffer
    m.to_start();
    memmove(m.buf,pos,len);
    if (move) pos+=len;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end decode NetMsg

/** Encode NetMsg.
 * @param m NetMsg reference
 * @param len length of data to encode
 * @param move determines if current position in buffer is changed or not.
 */
void NetMsg::encode(const NetMsg& m, uint32 len, bool move) {
  if ((pos+(len-1))<=buf_end) {
    memmove(pos,m.buf,len);
    if (move) pos+=len;
  } else throw NetMsgError(NetMsgError::ERROR_INVALID_POS);
} // end encode NetMsg


/** hex of dump buffer contents
 * if startpos is 0, the beginning of the buffer is used
 */
ostream& 
NetMsg::hexdump(ostream& os, uchar *startpos, uint32 length) const
{
  if (length==0) 
    length=buf_len;

  if (startpos == 0)
    startpos= buf;

  ios_base::fmtflags flags = os.flags();	// save stream flags

  os << color[blue] << "[dump: start=" << static_cast<void *>(startpos) << ", length:" << length;
  if (startpos > buf_end)
    return os << "ERROR: start position behind last buffer byte ]" << color[clear] << endl;
  else
  if (startpos < buf)
    return os << "ERROR: start position before first buffer byte ]" << color[clear] << endl;

  os << endl;
 
  while ( length > 0 && startpos <= buf_end )
  {
    os << setw(4) << startpos-buf << ": ";

    // alphanumeric characters are printed directly
    for (uint8 index=0; index <= 3; index++) {
	if ( startpos+index <= buf_end ) {
	    uchar c = *(startpos+index);
	    os << ( isalnum(c) ? static_cast<char>(c) : '.');
	}
	else
	    os << ' ';
    }

    os << " : " << hex << noshowbase;

    // dump hex numbers
    for (uint8 index=0; index <= 3; index++) {
	if ( startpos+index <= buf_end )
	    os << setw(2) << setfill('0')
		<< static_cast<unsigned short>(*(startpos+index)) << ' ';
	else
	    os << "   ";
    }

    os << setfill(' ') << "       ";

    // print in base 2
    for (uint8 index=0; index <= 3 && (startpos+index <= buf_end); index++)
    {
	unsigned short val = static_cast<unsigned short>(*(startpos+index));

	for (int i=7; i >= 0; i--)
	    os << ((val >> i) & 1);

	os << ' ';
    }

    startpos += 4;
    length = ( length >= 4 ) ? length-4 : 0;

    // reset formatting
    os.width(0);
    os << dec << setfill(' ') << endl;
  }

  os.setf(flags);		// reset stream flags

  return os << ']' << color[clear] << endl;
}

ostream& operator<<(ostream& os, NetMsg& msg)
{ 
 
  ostringstream hexdumpstr;
  msg.hexdump(hexdumpstr);
  return os << hexdumpstr.str();
  
}

//@}

} // end namespace protlib
