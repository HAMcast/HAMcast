/// ----------------------------------------*- mode: C++; -*--
/// @file threadsafe_db.cpp
/// Thread-safe access for some resolving functions (netdb)...
/// ----------------------------------------------------------
/// $Id: threadsafe_db.cpp 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/threadsafe_db.cpp $
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
 * As the reentrant functions for netdb access seem not portable, I decided to 
 * write some wrappers for those functions I need. This is not 
 * object-oriented because the NetDB would be another singleton object and
 * I only want some wrapper functions.
 */

#include <netdb.h>
#include <pwd.h>
#include <netinet/in.h>
#include <cerrno>

#include "threadsafe_db.h"
#include "cleanuphandler.h"
#include "logfile.h"

namespace protlib { 

/** @addtogroup protlib
 * @{
 */

  using namespace log;

bool tsdb::is_init = false;
bool tsdb::resolvenames = true;
pthread_mutex_t tsdb::mutex = 
#ifdef _DEBUG
    PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#else
    PTHREAD_MUTEX_INITIALIZER;
#endif


uint32 tsdb::id32 = 1;
uint64 tsdb::id64 = 1;

protocol_t tsdb::udp_id= 17;
protocol_t tsdb::tcp_id= 6;
protocol_t tsdb::sctp_id= 132;


void 
tsdb::init(bool noresolving) 
{
  if (is_init) 
  {
    Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to initialize tstdb although already initialized.");
  } else 
  {
    pthread_mutex_init(&mutex,NULL);
    is_init = true;

    // initialize frequently used protocol constants
    udp_id= tsdb::getprotobyname("udp");
    tcp_id= tsdb::getprotobyname("tcp");
    sctp_id= tsdb::getprotobyname("sctp");

    resolvenames=!noresolving;
    if (!resolvenames) 
      Log(INFO_LOG,LOG_NORMAL,"Threadsafe_DB"," ** Disabled reverse name lookups - addresses will not be resolved to names **");
  } // end if is_init
} // end init
  
void tsdb::end() {
	if (is_init) {
		is_init = false;
		pthread_mutex_destroy(&mutex);
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to end tstdb although not initialized.");
	} // end if is_init
} // end end

uint32 tsdb::get_new_id32() {
	uint32 res = 0;
	if (is_init) {
		pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
		res = id32++;
		pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	} // end if is_init
	return res;
} // end get_new_id32

uint64 tsdb::get_new_id64() {
	uint64 res = 0;
	if (is_init) {
		pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
		res = id64++;
		pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	} // end if is_init
	return res;
} // end get_new_id64

string tsdb::getprotobynumber(protocol_t proto, bool *res) {
	string str;
	if (is_init) 
	{
		pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
		struct protoent* entry = ::getprotobynumber(proto);

		if (res) *res = (entry!=NULL);
		if (entry) 
		  str = entry->p_name;
		else 
		  str = "UNKNOWN";

		pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	} 
	else 
	{
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	  if (res) *res = false;
	  str = "";
	} // end if is_init
	return str;
} // end getprotobynumber

protocol_t tsdb::getprotobyname(const string &pname, bool *res) {
	return getprotobyname(pname.c_str(),res);
} // end getprotobyname

protocol_t tsdb::getprotobyname(const char* pname, bool *res) {

#ifdef ANDROID
	if(strcmp(pname, "tcp") == 0) return tsdb::tcp_id;
	else if(strcmp(pname, "udp") == 0) return tsdb::udp_id;
	else if(strcmp(pname, "sctp") == 0) return tsdb::sctp_id;
	else return 0;
#endif

	register protocol_t pnum;
	struct protoent* entry = NULL;
	if (is_init) {
		pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
		if (pname) entry = ::getprotobyname(pname);
		if (res) *res = (entry!=NULL);
		if (entry) pnum = entry->p_proto;
		else pnum = 0;
		pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	  if (res) *res = false;
	  pnum = 0;
	} // end if is_init
	return pnum;
} // end getprotobyname

string tsdb::get_username(uid_t uid, bool *res) {
	string str;
	if (is_init) {
		pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
		struct passwd* entry = ::getpwuid(uid);
		if (res) *res = (entry!=NULL);
		if (entry) str = entry->pw_name;
		else str = "UNKNOWN";
		pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	  if (res) *res = false;
	  str = "";
	} // end if is_init
	return str;
} // end get_username

uid_t tsdb::get_userid(const char* uname, bool *res) {
	register uid_t uid;
	struct passwd* entry = NULL;
	if (is_init) {
		pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
		if (uname) entry = ::getpwnam(uname);
		if (res) *res = (entry!=NULL);
		if (entry) uid = entry->pw_uid;
		else uid = 0;
		pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	  if (res) *res = false;
	  uid = 0;
	} // end if is_init
	return uid;
} // end get_userid

uid_t tsdb::get_userid(const string& uname, bool *res) {
	return get_userid(uname.c_str(),res);
} // end get_userid

string tsdb::get_portname(port_t port, protocol_t prot, bool *res) {
	string str;
	if (is_init) {
		bool tmpres = true;
		string pname = getprotobynumber(prot,&tmpres);
		if (tmpres) {
			pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
			struct servent* entry = ::getservbyport(htons(port),pname.c_str());
			if (res) *res = (entry!=NULL);
			if (entry) str = entry->s_name;
			else str = "UNKNOWN";
			pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
		} else {
			if (res) *res = false;
			str = "UNKNOWN";
		} // end if tmpres
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	  if (res) *res = false;
	  str = "";
	} // end if is_init
	return str;
} // end get_portname

port_t tsdb::get_portnumber(const char* pname, protocol_t prot, bool *res) {
	register port_t pnum;
	struct servent* entry = NULL;
	if (is_init) {
		bool tmpres = true;
		string protoname = getprotobynumber(prot,&tmpres);
		if (tmpres) {
			pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
			if (pname) entry = ::getservbyname(pname,protoname.c_str());
			if (res) *res = (entry!=NULL);
			if (entry) pnum = ntohs(entry->s_port);
			else pnum = 0;
			pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
		} else {
			if (res) *res = false;
			pnum = 0;
		} // end if tmpres
	} else {
	  Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
	  if (res) *res = false;
	  pnum = 0;
	} // end if is_init
	return pnum;
} // end get_portnumber

port_t tsdb::get_portnumber(const string& pname, protocol_t prot, bool *res) {
	return get_portnumber(pname.c_str(),prot,res);
} // end get_portnumber


string 
tsdb::get_hostname(const struct sockaddr* sa, bool *res) 
{
  string str;
  static char tmpbuf[NI_MAXHOST];

  if (is_init) 
  {
    pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
    if (resolvenames)
    {
      int resultval= getnameinfo(sa,sizeof(struct sockaddr),
				 tmpbuf,sizeof(tmpbuf),
				 0,0, // services
				 0);  // flags
      
      if (res) *res = (resultval==0);
      if (resultval==0)
      { // success
	str= tmpbuf; // this should copy the buffer contents
      }
      else
      {
	str = "UNKNOWN";
	if (resultval==EAI_AGAIN || errno==EAI_AGAIN)
	{
	  Log(INFO_LOG,LOG_NORMAL, "Threadsafe_DB", "Temporary failure in name lookup. Try again later.");
	}
	else
	  Log(INFO_LOG,LOG_NORMAL, "Threadsafe_DB", "Name lookup failed -" << strerror(errno));

	if (res) *res= false;
      }
    }
    else
    {
      str= "disabled";
      if (res) *res= false;
    }
    pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  }
  else 
  {
    Log(ERROR_LOG,LOG_NORMAL, "Threadsafe_DB", "Tried to access tsdb although not initialized.");
    if (res) *res = false;
    str = "";
  } // end if is_init
  return str;
} // ent get_hostname(in_addr)



/** lookup of hostname for an ipv4 address
 *  @param in ipv4 address structure
 *  @param res returns true if name lookup was successful, otherwise false
 *
 *  @return in case that resolving is enabled it returns the host name corresponding to the given address or "UNKNOWN", 
 *           otherwise it returns "disabled"
 */
string 
tsdb::get_hostname(const in_addr& in, bool *res) 
{
  struct sockaddr_in sa={
    AF_INET,
    0,
    in
  };
  return get_hostname(reinterpret_cast<const sockaddr*>(&sa),res);
}

/** lookup of hostname for ipv6 address
 *  @param in ipv6 address structure
 *  @param res returns true if name lookup was successful, otherwise false
 *
 *  @return in case that resolving is enabled it returns the host name corresponding to the given address or "UNKNOWN", 
 *           otherwise it returns "disabled"
 */
string 
tsdb::get_hostname(const in6_addr& in, bool *res) 
{
  struct sockaddr_in6 sa={
    AF_INET6,
    0, // transport layer port #
    0, // IPv6 flow information
    in,
    0  // scope id (new in RFC2553)
  };
  return get_hostname(reinterpret_cast<const sockaddr*>(&sa),res);
} // ent get_hostname(in6_addr)

//@}

} // end namespace protlib
