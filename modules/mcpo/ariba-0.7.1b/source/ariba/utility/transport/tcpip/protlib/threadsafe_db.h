/// ----------------------------------------*- mode: C++; -*--
/// @file threadsafe_db.h
/// Thread-safe netdb access
/// ----------------------------------------------------------
/// $Id: threadsafe_db.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/threadsafe_db.h $
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
 * Thread-safe netdb access for linux.
 *
 * As the reentrant functions for netdb access seem not portable, I decided to 
 * write some wrappers for those functions I need. This is not 
 * object-oriented because the NetDB would be another singleton object and
 * I only want some wrapper functions.
 */

#ifndef THREADSAFE_DB_H
#define THREADSAFE_DB_H

#include <sys/types.h>
#include <pthread.h>
#include <netinet/in.h>
#include <string>

#include "protlib_types.h"

namespace protlib { 

/** @addtogroup protlib
 * @{
 */

/// Thread-safe DB
/** This class provides class methods for accessing the protocol database
 * and maybe other services from netdb.h in a thread-safe way.
 */
class tsdb {
private:
	/// init state
	static bool is_init;
	/// enable/disable name resolving via DNS
	static bool resolvenames;

	/// netdb mutex
	static pthread_mutex_t mutex;
	// @{
	/// last used IDs
	static uint32 id32;
	static uint64 id64;
	// @}

	// standard protocol ids
	static protocol_t udp_id;
	static protocol_t tcp_id;
	static protocol_t sctp_id;

public:
	/// initialize netdb
	static void init(bool noresolving= false);
	/// cleanup netdb resources
	static void end();
	/// get new 32bit-ID
	static uint32 get_new_id32();
	/// get new 64bit-ID
	static uint64 get_new_id64();
	/// get protocol name by number
	static string getprotobynumber(protocol_t proto, bool *res = NULL);
	/// get protocol number by name
	static protocol_t getprotobyname(const string &pname, bool *res = NULL);
	/// get protocol number by name
	static protocol_t getprotobyname(const char* pname, bool *res = NULL);
	/// get frequently used protocol numbers
	static protocol_t get_udp_id()  { return udp_id; }
	static protocol_t get_tcp_id()  { return tcp_id; } 
	static protocol_t get_sctp_id() { return sctp_id; } 
	
	/// get user name
	static string get_username(uid_t uid, bool *res = NULL);
	/// get user ID
	static uid_t get_userid(const char* uname, bool *res = NULL);
	/// get user ID
	static uid_t get_userid(const string& uname, bool *res = NULL);
	/// get port name
	static string get_portname(port_t port, protocol_t prot, bool *res = NULL);
	/// get port number
	static port_t get_portnumber(const char* pname, protocol_t prot, bool *res = NULL);
	/// get port number
	static port_t get_portnumber(const string& pname, protocol_t prot, bool *res = NULL);
	/// lookup host name
	static string get_hostname(const struct sockaddr* sa, bool *res);
	static string get_hostname(const in_addr& in, bool *res = NULL);
	static string get_hostname(const in6_addr& in, bool *res = NULL);

}; // end class tsdb

//@}

} // end namespace protlib

#endif
