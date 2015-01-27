/// ----------------------------------------*- mode: C++; -*--
/// @file address.cpp
/// GIST address objects
/// ----------------------------------------------------------
/// $Id: address.cpp 3046 2008-06-18 14:40:43Z hiwi-laier $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/address.cpp $
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
 * GIST address objects
 */

#include <sys/types.h>
#include <sys/socket.h>

#include "address.h"
#include "threadsafe_db.h"
#include "logfile.h"

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iomanip>
#include <netdb.h>

namespace protlib {

/** @addtogroup protlib
 * @{
 */
  using namespace log;
/***** class address *****/

/** Set subtype. */
address::address(subtype_t st) 
	: subtype(st) 
{ 
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for " << (void *) this); 
}

/** Log and throw a nomem_error */
void address::throw_nomem_error() const {
	try {
	  ERRCLog("address", "Not enough memory for address");
	} catch(...) {}
	throw IEError(IEError::ERROR_NO_MEM);
} // end throw_nomem_error


/***** class hostaddress *****/

/***** inherited from IE *****/

hostaddress* hostaddress::new_instance() const {
	hostaddress* ha = NULL;
	catch_bad_alloc(ha = new hostaddress());
	return ha;
} // end new_instance

hostaddress* hostaddress::copy() const {
	hostaddress* ha = NULL;
	catch_bad_alloc(ha =  new hostaddress(*this));
	return ha;
} // end copy 

bool hostaddress::operator==(const address& ie) const {
	const hostaddress* haddr = dynamic_cast<const hostaddress*>(&ie);
	if (haddr) {
#ifdef DEBUG_HARD
	  Log(DEBUG_LOG,LOG_NORMAL,"hostaddress","::operator==()" << haddr->get_ip_str()<<"=="<<this->get_ip_str());
	  if (!ipv4flag)
	    Log(DEBUG_LOG,LOG_NORMAL,"hostaddress","::operator==(), v6=" << IN6_ARE_ADDR_EQUAL(&ipv6addr, &(haddr->ipv6addr)));
#endif
	  return ipv4flag ? (ipv4addr.s_addr==haddr->ipv4addr.s_addr) :
	                    IN6_ARE_ADDR_EQUAL(&ipv6addr, &(haddr->ipv6addr));
	} else return false;
} // end operator==

/***** new in hostaddress *****/


/** Initialize hostaddress from string if possible. */
hostaddress::hostaddress(const char *str, bool *res) 
	: address(IPv6HostAddress),
	  ipv4flag(false), 
	  outstring(NULL)
{
	register bool tmpres = false;
	clear_ip();
	tmpres = set_ip(str);
	if (res) *res = tmpres;
} // end string constructor hostaddress


/** Set IPv4 from string or leave object unchanged.
 * This changes object type to IPv4.
 * @return true on success.
 */
bool 
hostaddress::set_ipv4(const char *str) {
	struct in_addr in;
	if (str && (inet_pton(AF_INET,str,&in)>0)) {
		set_ip(in);
		return true;
	} else return false;
} // end set_ipv4

/** Set IPv4 address from struct in_addr. 
 * This changes object type to IPv4. 
 */
void hostaddress::set_ip(const struct in_addr &in) {
	clear_ip();
	ipv4addr = in;	
	// set subtype to IPv4
	set_subtype(true);
	return;
} // end set_ip(in_addr)

/** Set IPv6 from string or leave object unchanged.
 * This changes object type to IPv6.
 * @return true on success.
 */
bool hostaddress::set_ipv6(const char *str) {
	struct in6_addr in;
	if (str && (inet_pton(AF_INET6,str,&in)>0)) {
		set_ip(in);
		return true;
	} else return false;
} // end set_ipv6	

/** Set IPv6 address from struct in6_addr. 
 * This changes object type to IPv6. 
 */
void 
hostaddress::set_ip(const struct in6_addr &in) {
	clear_ip();
	ipv6addr = in;	
	// set subtype to IPv6
	set_subtype(false);
	return;
} // end set_ip(in6_addr)



void hostaddress::set_ip(const hostaddress& h) {
	clear_ip();
	if (h.ipv4flag) {
		ipv4addr = h.ipv4addr;
	} else {
		ipv6addr = h.ipv6addr;
	} // end if ipv4flag
	set_subtype(h.ipv4flag);
} // end set_ip(hostaddress)

/** Check if IP address is unspecified (set to 0). */
bool hostaddress::is_ip_unspec() const {
	if (ipv4flag) return (ipv4addr.s_addr==0);
	else return IN6_IS_ADDR_UNSPECIFIED(&ipv6addr);
	// never reached
	return true;
} // end is_unspec

/** Get IP address as a string. The output string is kept inside the
 * hostaddress object and should be copied if used for a longer time.
 */
const char* hostaddress::get_ip_str() const 
{
  // If outstring exists then it is valid.
  if (outstring) 
    return outstring;
  else
    outstring= (ipv4flag ? new(nothrow) char[INET_ADDRSTRLEN] : 
		           new(nothrow) char[INET6_ADDRSTRLEN]);

  if (hostaddress::get_ip_str(outstring)) 
    return outstring;
  else 
  {
    // error
    if (outstring) 
      delete[] outstring;

    return (outstring = NULL);
  } // end if get_ip_str()
} // end get_ip_str

/** Get IP address as a string.
 * @param str string buffer
 */
const char* hostaddress::get_ip_str(char *str) const 
{
  if (!str) return NULL;
  memset(str,0, ipv4flag ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
  return ipv4flag ? inet_ntop(AF_INET,(void*)&ipv4addr,str,INET_ADDRSTRLEN) 
                  : inet_ntop(AF_INET6,(void*)&ipv6addr,str,INET6_ADDRSTRLEN);
} // end get_ip_str(char*)


/** Get IP address as an in_addr if possible.
 * @return true on success.
 */
bool hostaddress::get_ip(struct in_addr& in) const {
	if (ipv4flag) {
		in = ipv4addr;
		return true;
	} else if (IN6_IS_ADDR_V4MAPPED(&ipv6addr)) {
		memcpy(&(in.s_addr),ipv6addr.s6_addr+12,4);
		return true;
	} else return false;
} // get_ip

/** Get IP address as an in6_addr.
 * If this is an IPv4 address, it is mapped to IPv6.
 * @return true since this is always possible.
 */
bool hostaddress::get_ip(struct in6_addr& in) const {
	if (ipv4flag) {
		// map to IPv6
		memset(in.s6_addr,0,10);
		memset(in.s6_addr+10,255,2);
		memcpy(in.s6_addr+12,&(ipv4addr.s_addr),4);
	} else in = ipv6addr;
	return true;
} // get_ip

/// returns false if address is not allowed to be used
/// as source address: loopback, multicast, broadcast, unspecified
bool 
hostaddress::is_bogus_source() const
{
  if (ipv4flag)
  { // IPv4
    if ( IN_MULTICAST(ipv4addr.s_addr) || 
	 ipv4addr.s_addr == INADDR_LOOPBACK  ||
         ipv4addr.s_addr == INADDR_ANY ||
         ipv4addr.s_addr == INADDR_BROADCAST
       )
    {
      return true;
    }
  }
  else
  { // IPv6
    if ( ipv6addr.s6_addr == in6addr_any.s6_addr ||
	 ipv6addr.s6_addr == in6addr_loopback.s6_addr )
      return true;
  }

  return false;
}


/** Convert the IP address to IPv6 by mapping it from IPv4 to IPv6 if
 * necessary
 */
void hostaddress::convert_to_ipv6() {
	if (ipv4flag) {
		if (is_ip_unspec()) {
			// clear IP and set to IPv6
			clear_ip();
			set_subtype(false);
		} else {
			// map to IPv6
			struct in6_addr in;
			get_ip(in);
			// set IP
			set_ip(in);
		} // end if is_ip_unspec()
	} // end if ipv4flag
} // end convert_to_ipv6

/** Two addresses are equivalent if they are equal after converting both to
 * IPv6.
 */
bool hostaddress::equiv(const hostaddress& h) const {
	bool thisipv4 = is_ipv4();
	bool result = false;
	hostaddress* ipv4haddr = NULL;
	const hostaddress* ipv6haddr = NULL;
	// if both IPv4 or both IPv6 then just test for equality
	if (thisipv4==h.is_ipv4()) return operator==(h);
	// OK, one is IPv6 and the other is IPv4 (to be converted!).
	// Allocating memory dynamically because I do not know the concrete
	// type of *this or h.
	try {
		if (thisipv4) {
			ipv6haddr = &h;
			if (h.is_mapped_ip()) ipv4haddr = this->copy();
		} else {
			ipv6haddr = this;
			if (is_mapped_ip()) ipv4haddr = h.copy();
		} // end if thisipv4
	} catch(IEError&) { ipv4haddr = NULL; }
	if (ipv4haddr) {
		ipv4haddr->convert_to_ipv6();
		result = ((*ipv4haddr)==(*ipv6haddr));
		delete ipv4haddr;
		return result;
	} else return false;
} // end equiv


/** Set subtype and IPv4 flag. This does NOT clear the outstring buffer.
 * Use clear_ip(). 
 */
void hostaddress::set_subtype(bool ipv4) {
	ipv4flag = ipv4; 
	subtype = ipv4flag ? IPv4HostAddress : IPv6HostAddress;
} // end set_subtype

/** Clear the IP address buffer and the outstring if it exists. */
void hostaddress::clear_ip() {
	// only need to clear ipv6 because of union
	ipv6addr = in6addr_any;
	if (outstring) {
		delete[] outstring;
		outstring = NULL;
	} // end if outstring
} // end clear_ip


/** Match this IP address against another IP address.
 * Two IP addresses match if they are equal.
 * @return -1 on no match or error, e.g. when IPv4 and IPv6 are matched
 * @return the length of the matched prefix.
 */
int hostaddress::match_against(const hostaddress& ha) const {
	if (ipv4flag==ha.ipv4flag) {
		if (operator==(ha)) {
			if (ipv4flag) return 32;
			else return 128;
		} else return -1;
	} else return -1;
} // end match_against(

/** Match this IP address against the given network prefix.
 * @return -1 on no match or error, e.g. when IPv4 and IPv6 are matched
 * @return the length of the matched prefix.
 */
int hostaddress::match_against(const netaddress& na) const {
	uint32 word1 = 0;
	uint32 word2 = 0;
	const prefix_length_t preflen = na.get_pref_len();
	prefix_length_t lenwords = preflen/32;
	prefix_length_t lenbits = preflen%32;
	// now 0<=lenwords<=4, 0<=lenbits<=31
	prefix_length_t i;
	const hostaddress& ha = dynamic_cast<const hostaddress&>(na);
	if (ipv4flag==na.ipv4flag) {
		if (ipv4flag) {
			if (preflen >= 32)
				lenbits = 32;
			// match IPv4
			word1 = ntohl(ipv4addr.s_addr);
			word2 = ntohl(ha.ipv4addr.s_addr);
			// shift right
			word1 >>= (32-lenbits);
			word2 >>= (32-lenbits);
			if (word1==word2) return preflen;
			else return -1;
		} else {
			if (preflen > 128)
				return -1;
			// match IPv6
			// match words
			for (i=0;i<lenwords;i++) {
			  word1 = ntohl(ipv6addr.s6_addr32[i]);    // RB: as long as both words are in same order we dont need ntohl?!
			  word2 = ntohl(ha.ipv6addr.s6_addr32[i]);
				if (word1!=word2) return -1;
			} // end for i
			// now i points to the next word to be matched
			// match bits
			if (lenbits) {
				word1 = ntohl(ipv6addr.s6_addr32[i]);
				word2 = ntohl(ha.ipv6addr.s6_addr32[i]);
				// shift right
				word1 >>= (32-lenbits);
				word2 >>= (32-lenbits);
				if (word1==word2) return preflen;
				else return -1;
			} else {
				// no extra bits to match and everything OK
				return preflen;
			} // end if lenbits
		} // end if ipv4flag
	} else return -1;
} // end match_against

/***** class appladdress *****/

/***** inherited from IE *****/

appladdress* appladdress::new_instance() const {
	appladdress* aa = NULL;
	catch_bad_alloc(aa = new appladdress());
	return aa;
} // end new_instance

appladdress* appladdress::copy() const {
	appladdress* aa = NULL;
	catch_bad_alloc(aa = new appladdress(*this));
	return aa;
} // end copy 

bool appladdress::operator==(const address& ie) const {
	const appladdress* app = dynamic_cast<const appladdress*>(&ie);
	if (app) {
		// if the IEs are equal when casted to type hostaddress, then
		// only protocols and ports have to be checked.
		// Otherwise they are not equal.
		if (hostaddress::operator==(ie)) {
		    //if (proto!=app->proto) cout << "protocols not matching" << endl;
		    //if (port !=app->port) cout << "ports not matching" << endl;

		    return ((proto==app->proto) && (port==app->port));
		} else return false;
	} else return false;
} // end operator==


/***** class netaddress *****/

/***** inherited from IE *****/

netaddress* netaddress::new_instance() const {
	netaddress* na = NULL;
	catch_bad_alloc(na = new netaddress());
	return na;
} // end new_instance

netaddress* netaddress::copy() const {
	netaddress* na = NULL;
	catch_bad_alloc(na = new netaddress(*this));
	return na;
} // end copy 

bool netaddress::operator==(const address& ie) const {
	const netaddress* na = dynamic_cast<const netaddress*>(&ie);
	if (na) {
		// if the IEs are equal when casted to type hostaddress, then
		// only prefix lengths have to be checked.
		// Otherwise they are not equal.
		if (hostaddress::operator==(ie)) {
		    if (prefix_length!=na->prefix_length) cout << "Prefix length not matching" << endl;
		    return (prefix_length==na->prefix_length);
		} else return false;
	} else return false;
} // end operator==


/***** inherited from hostaddress *****/

/** Convert the IP address to IPv6 by mapping it from IPv4 to IPv6 if
 * necessary. The prefix length is converted, too.
 */
void netaddress::convert_to_ipv6() {
	if (ipv4flag) {
		// map IP
		hostaddress::convert_to_ipv6();
		// adjust prefix length
		set_pref_len(prefix_length+96);
	} // end if ipv4flag
} // end convert_to_ipv6

/** Set subtype and IPv4 flag. This does NOT clear the outstring buffer.
 * Use clear_ip(). 
 * In addition the prefix length is checked and set to reasonable values.
 */
void netaddress::set_subtype(bool ipv4) {
	ipv4flag = ipv4; 
	if (ipv4) {
		subtype = IPv4NetAddress; 
		if (prefix_length>32) prefix_length = 32;
	} else {
		subtype = IPv6NetAddress;
		if (prefix_length>128) prefix_length = 128;
	} // end if ipv4flag
} // end set_subtype

/***** new members in netaddress *****/

/** Constructor sets address type and clears prefix length. */
netaddress::netaddress() : 
  hostaddress(),
  prefix_length(0)
 {
	set_subtype(ipv4flag);
} // end constructor

netaddress::netaddress(const netaddress& na) : hostaddress(na) {
	prefix_length = na.prefix_length;
	set_subtype(ipv4flag);
} // end copy constructor

/** Initialize with the given host address and prefix length. 
 * Prefix length is optional and set to 0 if missing.
 */
netaddress::netaddress(const hostaddress& h, prefix_length_t len) : hostaddress(h) {
  set_subtype(ipv4flag);
  set_pref_len(len); // this will cut the prefix value accordingly
} // end constructor(hostaddress,len)

/** Initialize from string which contains IP and prefix length separated by
 * '/'.
 */
netaddress::netaddress(const char* str, bool *res) : hostaddress() {
	bool tmpres = true; // MUST BE true
	bool tmpres2 = false;
	long int len = 0;
	uint32 iplen;
	char* i = NULL;
	char* errptr = NULL;
	char ipstr[INET6_ADDRSTRLEN] = {0};
	// look for /
	i = strchr((char*)str,'/');
	if (i) {
		iplen = i-str;
		i++;
		// decode prefix length
		len = strtol(i,&errptr,10);
		if ((*i) && errptr && ((*errptr)==0)) {
			// prefix OK
			prefix_length = len;
		} else {
			prefix_length = 0;
			tmpres = false;
		} // end if prefix OK
		if (iplen<=INET6_ADDRSTRLEN) {
			// copy IP string
			strncpy(ipstr,str,iplen);
			ipstr[INET6_ADDRSTRLEN-1] = 0;
			// set str to ipstr
			str = ipstr;
		} // end if iplen valid
	} else {
		// no prefix found, OK
		prefix_length = 0;
	} // end if i
	// set IP, this also sets ipvflag.
	tmpres2 = set_ip(str);
	if (res) *res = (tmpres && tmpres2);
	set_subtype(ipv4flag);
} // end constructor(string)

/** Initialize from string and prefix length.
 * If the string does not contain a valid IP address, it is set to all 0 by
 * the hostaddress constructor.
 */
netaddress::netaddress(const char* str, prefix_length_t len, bool *res) : hostaddress(str,res) {
  set_subtype(ipv4flag);
  set_pref_len(len); // this will cut the prefix value accordingly
} // end constructor(string,port)

/** Assigns the given netaddress address by using hostaddress::operator=(). */
netaddress& netaddress::operator=(const netaddress& na) {
  prefix_length = na.prefix_length;
  hostaddress::operator=(na);
  return *this;
} // end operator=


/** Assigns the given netaddress address by using hostaddress::operator=(). */
netaddress& netaddress::operator=(const hostaddress& na) {
	hostaddress::operator=(na);
	set_pref_len(128);
	return *this;
} // end operator=


/** Set prefix length and return old value.
 * will also cut prefix_length if needed (i.e., if len too large for addr type)
 */
prefix_length_t netaddress::set_pref_len(prefix_length_t len) {
	register prefix_length_t olen = prefix_length;
	prefix_length = ipv4flag ? ( (len>32) ? 32 : len ) :
	                           ( (len>128) ? 128 : len );
	return olen;
} // end set_port



/** Compare two netaddress objects. If neither a<b and b<a then a and b are
 * considered equal.
 */
bool netaddress::operator<(const netaddress& na) const {
	uint32 word1 = 0;
	uint32 word2 = 0;
	prefix_length_t lenwords = prefix_length/32;
	prefix_length_t lenbits = prefix_length%32;
	// now 0<=lenwords<=4, 0<=lenbits<=31
	prefix_length_t i;
	// IPv4 is always < IPv6
	if ((!ipv4flag) && na.ipv4flag) return false;
	else if (ipv4flag && (!na.ipv4flag)) return true;
	// now ipv4flag == na.ipv4flag
	else if (prefix_length<na.prefix_length) return true;
	else if (prefix_length>na.prefix_length) return false;
	// now prefix_length == na.prefix_length
	else if (ipv4flag) {
		// compare IPv4 with same prefix length
		word1 = ntohl(ipv4addr.s_addr);
		word2 = ntohl(na.ipv4addr.s_addr);
		// shift right
		word1 >>= (32-lenbits);
		word2 >>= (32-lenbits);
		if (word1<word2) return true;
		else return false;
	} else {
		// compare IPv6 with same prefix length
		// compare words
		for (i=0;i<lenwords;i++) {
			word1 = ntohl(ipv6addr.s6_addr32[i]);
			word2 = ntohl(na.ipv6addr.s6_addr32[i]);
			if (word1<word2) return true;
			if (word1>word2) return false;
		} // end for i
		// now i points to the next word to be compared and previous words are equal
		// compare bits
		if (lenbits) {
			word1 = ntohl(ipv6addr.s6_addr32[i]);
			word2 = ntohl(na.ipv6addr.s6_addr32[i]);
			// shift right
			word1 >>= (32-lenbits);
			word2 >>= (32-lenbits);
			if (word1<word2) return true;
			else return false;
		} else {
			// no extra bist to compare and everything equal
			return false;
		} // end if lenbits
	} // end if ipv4flag, prefox
} // end match_against

/**
 * Compare function for the radix trie:
 *
 * Compare this and na from *pos up to max(this->prefix, na->prefix)
 *
 * In pos return the position where the compare ended.
 *
 * The return value is 0 if the strings are equal and all of this' string
 * was consumed, otherwise the sign will indicate the bit in this' string
 * at pos (i.e. if the search should continue left or right of na).
 *
 * pos < 0 indicates error
 */
int
netaddress::rdx_cmp(const netaddress *na, int *pos) const
{
	if (na == NULL) {
		*pos = -1;
		return 0;
	}

	if (na->ipv4flag != ipv4flag ||
	    *pos > na->prefix_length ||
	    *pos > prefix_length) {
		*pos = -1;
		return 0;
	}

	if (na->prefix_length == 0) {
		*pos = 1;
		if (ipv4flag)
			return ((ntohl(ipv4addr.s_addr) & 0x80000000) == 0 ?
			    -1 : 1);
		else
			return ((htonl(ipv6addr.s6_addr32[0]) & 0x80000000) == 0 ?
			    -1 : 1);
	}

	if (*pos < 0)
		*pos = 0;

	uint32_t w1, w2, w3;
	int diff, i, p1, p2;

	if (ipv4flag) {
		diff = *pos;
		w1 = ntohl(ipv4addr.s_addr);
		w2 = ntohl(na->ipv4addr.s_addr);
		// in w3 store the difference
		w3 = w1 ^ w2;
		// mask out anything after prefix_length and before *pos
		w3 = (w3 >> (32 - prefix_length)) << (32 - prefix_length + diff);
		if (w3 == 0 && prefix_length <= na->prefix_length) {
			*pos = min(prefix_length, na->prefix_length);
			return 0;
		}
		// pos = 0 means start up front, so we need to fix up to that
		diff++;
		while (diff <= prefix_length && diff <= na->prefix_length) {
			if ((w3 & 0x80000000) != 0) {
				*pos = diff;
				return (((w1 & (1 << (32 - diff))) >>
				    (32 - diff)) == 0 ? -1 : 1);
			}
			w3 = w3 << 1;
			diff++;
		}
		// difference past na->prefix_length
		*pos = diff;
		return (((w1 & (1 << (32 - diff))) >>
		    (32 - diff)) == 0 ? -1 : 1);
	}

	diff = *pos;
	for (i = diff / 32; i < 4; i++) {
		diff = diff % 32;
		w1 = ntohl(ipv6addr.s6_addr32[i]);
		w2 = ntohl(na->ipv6addr.s6_addr32[i]);
		w3 = w1 ^ w2;
		p1 = (prefix_length - (i * 32));
		p1 = p1 > 32 ? 32 : p1;
		p2 = (na->prefix_length - (i * 32));
		p1 = p2 > 32 ? 32 : p2;

		w3 = (w3 >> (32 - p1)) << (32 - p1 + diff);
		if (w3 == 0 && prefix_length <= na->prefix_length) {
			*pos = min(prefix_length, na->prefix_length);
			if (prefix_length <= ((i + 1) * 32))
				return 0;
		}
		// pos = 0 means start up front, so we need to fix up to that
		diff++;
		while (diff <= p1 && diff <= p2) {
			if ((w3 & 0x80000000) != 0) {
				*pos = diff + (i * 32);
				return (((w1 & (1 << (32 - diff))) >>
				    (32 - diff)) == 0 ? -1 : 1);
			}
			w3 = w3 << 1;
			diff++;
		}
		if (diff + (32 * i) <= prefix_length &&
		    diff + (32 * i) <= na->prefix_length) {
			diff--;
			continue;
		}
		// difference past na->prefix_length
		*pos = diff + (i * 32);
		if (diff == 33) {
			diff = 1;
			if (i == 3)
				abort();
			w1 = ntohl(ipv6addr.s6_addr32[i+1]);
		}
		return (((w1 & (1 << (32 - diff))) >>
		    (32 - diff)) == 0 ? -1 : 1);
	}

	// Not reachable, but gcc complains
	return 0;
}

udsaddress* udsaddress::new_instance() const {
	udsaddress* ha = NULL;
	catch_bad_alloc(ha = new udsaddress());
	return ha;
} // end new_instance

udsaddress* udsaddress::copy() const {
        udsaddress* ha = NULL;
	catch_bad_alloc(ha =  new udsaddress(*this));
	return ha;
} // end copy 

bool udsaddress::operator==(const address& ie) const {
	const udsaddress* app = dynamic_cast<const udsaddress*>(&ie);
	if (app) {
	    return (app->socknum == socknum) && (app->uds_socket == uds_socket);
	} else return false;
} // end operator==

AddressList::AddrProperty *AddressList::LocalAddr_P;
AddressList::AddrProperty *AddressList::ConfiguredAddr_P;
AddressList::AddrProperty *AddressList::IgnoreAddr_P;
AddressList::AddrProperty *AddressList::AnyAddr_P;

AddressList::AddressList()
{
	if (LocalAddr_P == 0) {
		LocalAddr_P = new AddrProperty("local");
		ConfiguredAddr_P = new AddrProperty("configured");
		IgnoreAddr_P = new AddrProperty("ignore");
		AnyAddr_P = new AddrProperty("wildcard");
	}
	interfaces = 0;
}

AddressList::~AddressList()
{
	// Refcount AddressLists in order to GC properties?
}

AddressList::iflist_t *
AddressList::get_interfaces()
{
	iflist_t *iflist;

	if (interfaces != 0)
		iflist = new iflist_t(*interfaces);
	else {
		iflist = new iflist_t();
		getifaddrs_iflist(*iflist);
	}

	return iflist;
}

bool
AddressList::by_interface(bool start_empty)
{
	if (interfaces != 0)
		return false;

	interfaces = new iflist_t();
	if (!start_empty)
		getifaddrs_iflist(*interfaces);

	return true;
}

bool
AddressList::add_interface(char *name)
{
	if (interfaces == 0)
		return false;

	return (interfaces->insert(name)).second;
}

bool
AddressList::del_interface(char *name)
{
	if (interfaces == 0)
		return false;

	return (interfaces->erase(name) > 0);
}

bool
AddressList::add_property(netaddress &na, AddrProperty *p, bool propagate)
{
	propmap_t *props, *lpfm_props;
	propmap_t::iterator it;
	addr2prop_t::node *node;

	node = prop_trie.lookup_node(na, false, false);
	if (node != NULL) {
		props = node->data;
		if (props == NULL) {
			props = new propmap_t();
			node->data = props;
		}
		props->insert(pair<AddrProperty *, bool>(p, propagate));
		
	} else {
		props = new propmap_t();
		props->insert(pair<AddrProperty *, bool>(p, propagate));
		node = prop_trie.insert(na, *props);
	}

	if (propagate)
		bequeath(node, p, true);

	// copy lpfm properties
	lpfm_props = prop_trie.lookup(na, true);
	if (lpfm_props == NULL)
		return true;

	for (it = lpfm_props->begin(); it != lpfm_props->end(); it++) {
		if ((*it).second)
			props->insert((*it));
	}

	return true;
}

bool
AddressList::del_property(netaddress &na, AddrProperty *p, bool propagate)
{
	propmap_t *props, *lpfm_props;
	propmap_t::iterator it;
	addr2prop_t::node *node;

	node = prop_trie.lookup_node(na, false, true);
	if (node == NULL) {
		// no exact match
		if (!propagate) {
			node = prop_trie.lookup_node(na, true, true);
			if (node == NULL) {
				// no lpfm either, we're done
				return false;
			}

			props = node->data;
			it = props->find(p);
			if (it == props->end()) {
				// lpfm doesn't have p set -> done
				return false;
			}
		}
		// insert an empty propmap
		props = new propmap_t();
		node = prop_trie.insert(na, *props);

		// copy other lpfm properties
		lpfm_props = prop_trie.lookup(na, true);
		if (p != AnyAddr_P && lpfm_props != NULL) {
			for (it = lpfm_props->begin(); it != lpfm_props->end();
			    it++) {
				if ((*it).first != p && (*it).second)
					props->insert((*it));
			}
		}
	} else {
		props = node->data;
		if (p == AnyAddr_P) {
			props->clear();
		} else {
			it = props->find(p);
			if (it == props->end() && !propagate)
				return false;

			props->erase(it);
		}
	}

	if (propagate)
		bequeath(node, p, false);

	return true;
}

bool
AddressList::add_host_prop(const char *name, AddrProperty *p)
{
	netaddress na;
	sockaddr_in *sin;
	sockaddr_in6 *sin6;
	struct addrinfo hints = {0}, *res, *cur;
	int error;
	char buf[1024];

	if (name == NULL) {
		name = buf;
		if (gethostname(buf, sizeof(buf)) != 0)
			return false;
		buf[sizeof(buf) - 1] = '\0';
	}
	hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	error = getaddrinfo(name, NULL, &hints, &res);
	if (error != 0)
		return false;

	for(cur = res; cur != NULL && error == 0; cur = cur->ai_next) {
		if (cur->ai_family == AF_INET) {
			sin = (struct sockaddr_in *)cur->ai_addr;
			na.set_ip(sin->sin_addr);
			na.set_pref_len(32);
		} else if (cur->ai_family == AF_INET6) {
			sin6 = (struct sockaddr_in6 *)cur->ai_addr;
			na.set_ip(sin6->sin6_addr);
			na.set_pref_len(128);
		} else
			continue;

		// cout << ++i << "XXMOB: " << na << endl;

		error += add_property(na, p) ? 0 : 1;
		// XXXMOB: for some reason we need a 'reset' here
		//         if we want to use /etc/hosts
		na.set_ip("127.0.0.1");
	}
	freeaddrinfo(res);

	return (error == 0);
}

bool
AddressList::del_host_prop(const char *name, AddrProperty *p)
{
	netaddress na;
	sockaddr_in *sin;
	sockaddr_in6 *sin6;
	struct addrinfo hints = {0}, *res, *cur;
	int error;
	char buf[1024];

	if (name == NULL) {
		name = buf;
		if (gethostname(buf, sizeof(buf)) != 0)
			return false;
		buf[sizeof(buf) - 1] = '\0';
	}
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_family = AF_UNSPEC;
	error = getaddrinfo(name, NULL, &hints, &res);
	if (error != 0)
		return false;

	for(cur = res; cur != NULL && error == 0; cur = cur->ai_next) {
		if (cur->ai_family == AF_INET) {
			sin = (struct sockaddr_in *)cur->ai_addr;
			na.set_ip(sin->sin_addr);
			na.set_pref_len(32);
		} else if (cur->ai_family == AF_INET6) {
			sin6 = (struct sockaddr_in6 *)cur->ai_addr;
			na.set_ip(sin6->sin6_addr);
			na.set_pref_len(128);
		} else
			continue;

		error += del_property(na, p) ? 0 : 1;
	}
	freeaddrinfo(res);

	return (error == 0);
}

bool
AddressList::ignore(netaddress &na, bool propagate)
{
	del_property(na, AnyAddr_P, propagate);
	return add_property(na, IgnoreAddr_P);
}

bool
AddressList::unignore(netaddress &na, bool propagate)
{
	return del_property(na, IgnoreAddr_P, propagate);
}

bool
AddressList::ignore_bogons(void)
{
	netaddress na;

	// according to IANA Tue Apr 17 09:14:31 PDT 2007
	na.set_ip("0.0.0.0");na.set_pref_len(7);
	ignore(na);
	na.set_ip("2.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("5.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("7.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("23.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("27.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("31.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("36.0.0.0");na.set_pref_len(7);
	ignore(na);
	na.set_ip("39.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("42.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("49.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("50.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("94.0.0.0");na.set_pref_len(7);
	ignore(na);
	na.set_ip("100.0.0.0");na.set_pref_len(6);
	ignore(na);
	na.set_ip("104.0.0.0");na.set_pref_len(5);
	ignore(na);
	na.set_ip("112.0.0.0");na.set_pref_len(6);
	ignore(na);
	na.set_ip("169.254.0.0");na.set_pref_len(16);
	ignore(na);
	na.set_ip("173.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("174.0.0.0");na.set_pref_len(7);
	ignore(na);
	na.set_ip("176.0.0.0");na.set_pref_len(5);
	ignore(na);
	na.set_ip("184.0.0.0");na.set_pref_len(6);
	ignore(na);
	na.set_ip("191.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("192.0.2.0");na.set_pref_len(24);
	ignore(na);
	na.set_ip("197.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("198.18.0.0");na.set_pref_len(15);
	ignore(na);
	na.set_ip("223.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("240.0.0.0");na.set_pref_len(4);
	ignore(na);
	// according to http://www.cymru.com/Documents/bogonv6-list.html#agg
	na.set_ip("0000::");na.set_pref_len(8);
	ignore(na);
	na.set_ip("0100::");na.set_pref_len(8);
	ignore(na);
	na.set_ip("0200::");na.set_pref_len(7);
	ignore(na);
	na.set_ip("0400::");na.set_pref_len(7);
	ignore(na);
	na.set_ip("0600::");na.set_pref_len(7);
	ignore(na);
	na.set_ip("0800::");na.set_pref_len(5);
	ignore(na);
	na.set_ip("1000::");na.set_pref_len(4);
	ignore(na);
	na.set_ip("2000::");na.set_pref_len(16);
	ignore(na);
	na.set_ip("2001:1000::");na.set_pref_len(23);
	ignore(na);
	na.set_ip("2001:1600::");na.set_pref_len(23);
	ignore(na);
	na.set_ip("2001:2000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:3000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:4000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:5000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:6000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:7000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:8000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:9000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:A000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:B000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:C000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:D000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:E000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("2001:F000::");na.set_pref_len(20);
	ignore(na);
	na.set_ip("3FFF::");na.set_pref_len(16);
	ignore(na);
	na.set_ip("4000::");na.set_pref_len(3);
	ignore(na);
	na.set_ip("6000::");na.set_pref_len(3);
	ignore(na);
	na.set_ip("8000::");na.set_pref_len(3);
	ignore(na);
	na.set_ip("A000::");na.set_pref_len(3);
	ignore(na);
	na.set_ip("C000::");na.set_pref_len(3);
	ignore(na);
	na.set_ip("E000::");na.set_pref_len(4);
	ignore(na);
	na.set_ip("F000::");na.set_pref_len(5);
	ignore(na);
	na.set_ip("F800::");na.set_pref_len(6);
	ignore(na);
	na.set_ip("FC00::");na.set_pref_len(7);
	ignore(na);
	na.set_ip("FE00::");na.set_pref_len(9);
	ignore(na);

	return true;
}

bool
AddressList::ignore_locals(void)
{
	netaddress na;

	na.set_ip("10.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("172.16.0.0");na.set_pref_len(12);
	ignore(na);
	na.set_ip("192.168.0.0");na.set_pref_len(16);
	ignore(na);
	na.set_ip("FE80::");na.set_pref_len(10);
	ignore(na);
	na.set_ip("FEC0::");na.set_pref_len(10);
	ignore(na);

	return true;
}

bool
AddressList::ignore_loopback(void)
{
	netaddress na;

	na.set_ip("127.0.0.0");na.set_pref_len(8);
	ignore(na);
	na.set_ip("::1");na.set_pref_len(128);
	ignore(na);

	return true;
}

bool
AddressList::addr_is(netaddress &na, AddrProperty *prop)
{
	propmap_t *props;
	propmap_t::iterator it;

	if (addr_is_in(na, IgnoreAddr_P))
		return false;

	props = prop_trie.lookup(na, false);
	if (props != NULL) {
		it = props->find(prop);
		if (it != props->end()) {
			return true;
		}
	}

	if (prop != LocalAddr_P)
		return false;

	return getifaddrs_is_local(na);
}

bool
AddressList::addr_is_in(netaddress &na, AddrProperty *prop)
{
	addr2prop_t::node *node;
	propmap_t *props;
	propmap_t::iterator it;

	node = prop_trie.lookup_node(na, true, true);
	if (node == NULL)
		return false;

	props = node->data;
	it = props->find(prop);
	if (it == props->end())
		return false;

	if (!(*it).second && props != prop_trie.lookup(na, false))
			return false;

	return true;
}

AddressList::addrlist_t *
AddressList::get_addrs(AddrProperty *prop)
{
	addr2prop_t::node *node;
	netaddress na;
	addrlist_t *res = new addrlist_t();

	if (res == 0)
		return res;

	if (prop == LocalAddr_P || prop == AnyAddr_P)
		getifaddrs_get_addrs(*res);

	na.set_ip("0.0.0.0");
	na.set_pref_len(0);
	node = prop_trie.lookup_node(na, true, false);
	collect(node, prop, *res);

	na.set_ip("::");
	node = prop_trie.lookup_node(na, true, false);
	collect(node, prop, *res);

	return res;
}

netaddress *
AddressList::get_first(AddrProperty *p, bool IPv4)
{
	addr2prop_t::node *node;
	netaddress na;
	addrlist_t list;
	addrlist_t::iterator it;

	if (IPv4) {
		na.set_ip("0.0.0.0");
		na.set_pref_len(0);
	} else {
		na.set_ip("::");
		na.set_pref_len(0);
	}

	node = prop_trie.lookup_node(na, true, false);
	node = collect_first(node, p);
	if (node != NULL)
		return new netaddress(*node->key);

	if (p == LocalAddr_P) {
		getifaddrs_get_addrs(list);
		for (it = list.begin(); it != list.end(); it++)
			if ((*it).is_ipv4() == IPv4)
				return new netaddress(*it);
	}

	return NULL;
}

netaddress *
AddressList::get_src_addr(const netaddress &dest, uint32_t *prefs)
{
	netaddress *res;
	int sfd;

	sfd = socket(dest.is_ipv4()?AF_INET:AF_INET6, SOCK_DGRAM, 0);
	if (sfd == -1)
		return NULL;

#ifdef IPV6_ADDR_PREFERENCES
	if (prefs != NULL && setsockopt(s, IPV6_ADDR_PREFERENCES,
	    (void *)prefs, sizeof (*prefs)) == -1) {
		close(sfd);
		return NULL;
	}
#endif
	if (dest.is_ipv4()) {
		struct sockaddr_in sin = {0};
		socklen_t slen = sizeof(sin);
		sin.sin_family = AF_INET;
		sin.sin_port = htons(4);
		dest.get_ip(sin.sin_addr);
		if (connect(sfd, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
			close(sfd);
			return NULL;
		}
		if (getsockname(sfd, (struct sockaddr *)&sin, &slen) == -1) {
			close(sfd);
			return NULL;
		}
		close(sfd);
		res = new netaddress();
		res->set_ip(sin.sin_addr);
		res->set_pref_len(32);
		return (res);
	} else {
		struct sockaddr_in6 sin6 = {0};
		socklen_t slen = sizeof(sin6);
		sin6.sin6_family = AF_INET6;
		sin6.sin6_port = htons(4);
		dest.get_ip(sin6.sin6_addr);
		if (connect(sfd, (struct sockaddr *)&sin6,
		    sizeof(sin6)) == -1) {
			close(sfd);
			return NULL;
		}
		if (getsockname(sfd, (struct sockaddr *)&sin6, &slen) == -1) {
			close(sfd);
			return NULL;
		}
		close(sfd);
		res = new netaddress();
		res->set_ip(sin6.sin6_addr);
		res->set_pref_len(128);
		return (res);
	}
}

void
AddressList::getifaddrs_iflist(iflist_t &list)
{
	struct ifaddrs *ifap, *cifa;

	if (::getifaddrs(&ifap) != 0)
		return;

	for (cifa = ifap; cifa != NULL; cifa = cifa->ifa_next) {
		list.insert(cifa->ifa_name);
	}

	freeifaddrs(ifap);
}

bool
AddressList::getifaddrs_is_local(netaddress &na)
{
	struct ifaddrs *ifap, *cifa;

	if (::getifaddrs(&ifap) != 0)
		return false;

	for (cifa = ifap; cifa != NULL; cifa = cifa->ifa_next) {
		hostaddress ha;

		if (cifa->ifa_addr->sa_family == AF_INET) {
			ha.set_ip(
			    ((struct sockaddr_in *)cifa->ifa_addr)->sin_addr);
		} else if (cifa->ifa_addr->sa_family == AF_INET6) {
			ha.set_ip(
			    ((struct sockaddr_in6 *)cifa->ifa_addr)->sin6_addr);
		} else {
			continue;
		}

		if (interfaces &&
		    interfaces->find(cifa->ifa_name) == interfaces->end())
			continue;

		if (ha.match_against(na) >= na.get_pref_len()) {
			freeifaddrs(ifap);
			return true;
		}
	}

	freeifaddrs(ifap);

	return false;
}

void
AddressList::getifaddrs_get_addrs(addrlist_t &list)
{
	struct ifaddrs *ifap, *cifa;

	if (::getifaddrs(&ifap) != 0)
		return;

	for (cifa = ifap; cifa != NULL; cifa = cifa->ifa_next) {
		hostaddress *ha;
		netaddress na;

		if (interfaces &&
		    interfaces->find(cifa->ifa_name) == interfaces->end())
			continue;

		if (cifa->ifa_addr->sa_family == AF_INET) {
			ha = new hostaddress;
			ha->set_ip(
			    ((struct sockaddr_in *)cifa->ifa_addr)->sin_addr);
			na.set_pref_len(32);
		} else if (cifa->ifa_addr->sa_family == AF_INET6) {
			ha = new hostaddress;
			ha->set_ip(
			    ((struct sockaddr_in6 *)cifa->ifa_addr)->sin6_addr);
			na.set_pref_len(128);
		} else {
			continue;
		}

		na.set_ip(*ha);
		if (!addr_is_in(na, IgnoreAddr_P))
			list.insert(*ha);
	}

	freeifaddrs(ifap);
}

void
AddressList::bequeath(addr2prop_t::node *head, AddrProperty *p, bool add)
{
	propmap_t *props;
	propmap_t::iterator it;

	if (p == AnyAddr_P && add)
		return;

	props = head->data;
	if (props != NULL) {
		if (p == AnyAddr_P) {
			props->clear();
		} else {
			if (add) {
				props->insert(pair<AddrProperty *, bool>
				    (p, true));
			} else {
				it = props->find(p);
				if (it != props->end())
					props->erase(it);
			}
		}
	}

	if (head->left->index > head->index)
		bequeath(head->left, p, add);
	if (head->right->index > head->index)
		bequeath(head->right, p, add);
}

void
AddressList::collect(addr2prop_t::node *head, AddrProperty *p,
    addrlist_t &list)
{
	propmap_t *props;
	propmap_t::iterator it;

	props = head->data;
	if (props != NULL) {
		if (p == AnyAddr_P) {
			it = props->begin();
		} else {
			it = props->find(p);
		}
		if (it != props->end()) {
			list.insert(*(new netaddress(*head->key)));
		}
	}

	if (head->left->index > head->index)
		collect(head->left, p, list);
	if (head->right->index > head->index)
		collect(head->right, p, list);
	
}

AddressList::addr2prop_t::node *
AddressList::collect_first(addr2prop_t::node *head, AddrProperty *p)
{
	addr2prop_t::node *res = NULL;
	propmap_t *props;
	propmap_t::iterator it;

	props = head->data;
	if (props != NULL) {
		if (p == AnyAddr_P) {
			it = props->begin();
		} else {
			it = props->find(p);
		}
		if (it != props->end()) {
			return head;
		}
	}

	if (head->left->index > head->index) {
		res = collect_first(head->left, p);
		if (res != NULL)
			return res;
	}
	if (head->right->index > head->index) {
		res = collect_first(head->right, p);
		if (res != NULL)
			return res;
	}

	return NULL;
}


} // end namespace protlib
