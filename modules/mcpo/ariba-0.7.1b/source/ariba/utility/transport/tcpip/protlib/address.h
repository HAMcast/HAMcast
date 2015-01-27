/// ----------------------------------------*- mode: C++; -*--
/// @file address.h
/// GIST address objects
/// ----------------------------------------------------------
/// $Id: address.h 3063 2008-07-02 08:02:45Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/address.h $
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
 * GIST address objects
 */

#ifndef PROTLIB__ADDRESS_H
#define PROTLIB__ADDRESS_H

#include "protlib_types.h"
#include "ie.h"

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

#include <netinet/in.h>
#include <set>

#include "logfile.h"
#include "threadsafe_db.h"

namespace protlib {
  using namespace log;

/// Address base class
/** This is the base class for IP host and application addresses, router,
 * interface and user identification and AS numbers.
 */
class address {
  public:
	virtual address* new_instance() const = 0;
	virtual address* copy() const = 0;
	virtual bool operator==(const address& address) const = 0;

	inline bool operator!=(const address& address) const {
		return (!(*this==address));
	}


	enum subtype_t {
		/** 255 is not a valid subtype and is used to register for all
		 * subtypes.
		 * @note This is no valid subtype because it is not in 0..64.
		 */
		all_subtypes          = 255,
		// @{
		/// address subtypes
		/** These are the address-specific subtypes. */
		IPv4HostAddress       =   1,
		IPv6HostAddress       =   2,
		IPv4NetAddress        =   3,
		IPv6NetAddress        =   4,
		IPv4ApplAddress       =   5,
		IPv6ApplAddress       =   6,
		IPv6Unspecified	      =   7,
		UDSAddress            =   8,
		FlowAddressSpec       =  12,
		AS16                  =  14,
		AS32                  =  15,
		IEEE48                =  16,
		EUI48                 =  17,
		EUI64                 =  18,
		NAI                   =  32,
		X509                  =  33
		// @}
	}; // end subtype_t

	virtual ~address() {};

  subtype_t get_type() const { return subtype; };

  protected:
	/// constructor with subtype
	address(subtype_t st);
        /// virtual destructor

	void throw_nomem_error() const;

	subtype_t subtype;
};

// forward declaration
class netaddress;

/// IP Host Address
/** This class can hold IPv4 and IPv6 Host Addresses. */
class hostaddress : public address {

  public:
	virtual hostaddress* new_instance() const;
	virtual hostaddress* copy() const;
	virtual bool operator==(const address& ie) const;

	/// constructor
	hostaddress();
	/// copy constructor
	hostaddress(const hostaddress& h);
	/// assignment
	hostaddress& operator=(const hostaddress& h);
	/// constructor from string
	hostaddress(const char *str, bool *res = NULL);
	/// constructor from in6_addr
	hostaddress(const struct in6_addr& ipv6addr);
	/// destructor
	virtual ~hostaddress();
	/// set IPv4 from string
	bool set_ipv4(const char *str);
	/// set IPv4 from in_addr
	void set_ip(const struct in_addr &in);
	/// set IPv6 from string
	bool set_ipv6(const char *str);
	/// set IPv6 from in6_addr
	void set_ip(const struct in6_addr &in);
	/// set IPv4 or IPv6 from string
	bool set_ip(const char *str);
	bool set_ip(const string& str);
	/// set IP from hostaddress
	void set_ip(const hostaddress& h);
	/// is IP unspecified
	bool is_ip_unspec() const;
	/// get ip address as string
	const char *get_ip_str() const;
	/// get ip address as string
	const char *get_ip_str(char *str) const;
	/// is it IPv4
	bool is_ipv4() const;
	/// is it IPv6
	bool is_ipv6() const;
	/// is bogus source (e.g. localhost, multicast)
	bool is_bogus_source() const;
	/// is it a 4to6-mapped address?
	bool is_mapped_ip() const;
	/// get as in_addr?
	bool get_ip(struct in_addr& in) const;
	/// get as in6_addr?
	bool get_ip(struct in6_addr& in) const;
	/// get as in6_addr?
	const struct in6_addr *get_ip() const { return ipv4flag ? 0 : &ipv6addr; };
	/// convert to iPv6
	virtual void convert_to_ipv6();
	/// are they equivalent
	virtual bool equiv(const hostaddress& h) const;
	/// lookup host name
	string get_host_name(bool *res = NULL) const;
	/// hash function
	virtual size_t get_hash() const;
	/// match against IP address
	virtual int match_against(const hostaddress& ha) const;
        /// match against network prefix
	virtual int match_against(const netaddress& na) const;
protected:
	/// IPv4 flag
	bool ipv4flag;
	/// set subtype and IPv4 flag
	virtual void set_subtype(bool ipv4);
	/// IP buffer
	/** I in protected and NOT private scope because subclasses have to
	 * (de)serialize it.
	 */
	union {
		/// IPv4 address
		struct in_addr ipv4addr;
		/// IPv6 address
		struct in6_addr ipv6addr;
	}; // end union
public:
	/// clear IP buffer (sets IP address to undefined/any and deletes any outstring)
	void clear_ip();
private:
	/// pointer to IP string representation
	mutable char *outstring;
}; // end hostaddress

inline ostream &operator<<(ostream &out, const hostaddress &addr) {
     return out << addr.get_ip_str();
}

inline
hostaddress::hostaddress(const struct in6_addr& ipv6addr)
  : address(IPv6HostAddress),
    ipv4flag(false),
    ipv6addr(ipv6addr),
    outstring(NULL)
{ set_subtype(false); }


/// ========================================================
/// IP Application Address
/// ========================================================
/** Consists of a IP Host Address and a port number. */
class appladdress : public hostaddress {
    public:
	virtual appladdress* new_instance() const;
	virtual appladdress* copy() const;
	virtual bool operator==(const address& ie) const;

	/// hash function
	virtual size_t get_hash() const;

    protected:
	/// set subtype and IPv4 flag
	virtual void set_subtype(bool ipv4);

    public:
	/// constructor
	appladdress();
	/// copy constructor
	appladdress(const appladdress& app);
	///constructor for use as Unix Domain Address
	appladdress(string socket);
	///constructor for use to specify a explicit socket number (used when no addressing for peer can be derived)
	appladdress(int socket);
        /// constructor from hostaddress, protocol ID and port
	appladdress(const hostaddress& h, protocol_t prot, port_t p);
	/// constructor from sockaddr_in6 sockaddr
	appladdress(const sockaddr_in6& sockaddr,  protocol_t prot);
	/// constructor from hostaddress, protocol name and port
	appladdress(const hostaddress& h, const char* pname, port_t p, bool *res = NULL);
	/// constructor from string, protocol ID and port
	appladdress(const char* str, protocol_t prot, port_t p, bool *res = NULL);
	/// constructor from string, protocol name and port
	appladdress(const char* str, const char* pname, port_t p, bool *res = NULL);
	/// constructor from string, protocol name and port name
	appladdress(const char* str, const char* pname, const char* portname, bool *res = NULL);
	/// assignment
	appladdress& operator=(const appladdress& app);
        /// virtual destructor
        virtual ~appladdress() {};


	/// are they equivalent
        ///virtual bool equiv(const appladdress& h) const { return hostaddress::equiv(h); }

	/// set port
	port_t set_port(port_t p);
	/// set port
	port_t set_port(const char* pname, bool *res = NULL);
	/// set port
	port_t set_port(const string& pname, bool *res = NULL);
	/// get port
	port_t get_port() const;

	/// get sockaddr_in6
	void get_sockaddr(struct sockaddr_in6& sockaddr) const;
	/// get port name
	string get_port_name(bool *res = NULL) const;
	/// set protocol by ID
	protocol_t set_protocol(protocol_t p);
	/// set protocol by name
	protocol_t set_protocol(const char* pname, bool *res = NULL);
	/// set protocol by name
	protocol_t set_protocol(const string& pname, bool *res = NULL);
	/// get protocol ID
	protocol_t get_protocol() const;
	/// get protocol name
	string get_protocol_name(bool *res = NULL) const;
	/// get prefix
	inline
	uint8 get_prefix() const {
	    return prefix;
	}

	/// set prefix
	inline
	void set_prefix(uint8 prfx) {
	    prefix=prfx;
	}

	/// set IP TTL
	inline
	void set_ip_ttl(uint16 ttl) {
	    ip_ttl = ttl;
	}


	/// unset IP TTL
	inline
	void unset_ip_ttl() {
	    ip_ttl = 0;
	}


	/// get IP TTL, if == 0, no IP TTL should be set
	inline
	uint16 get_ip_ttl() const {
	return ip_ttl;
	}


	/// set RAO value
	inline
	void set_rao(uint16 value) {
	    rao_presence = true;
	    rao = value;
	}

	/// unset RAO value
	inline
	void unset_rao() {
	    rao_presence = false;
	    rao = 0;
	}

	/// get RAO value
	inline
	uint16 get_rao() const {
	    return rao;
	}


	/// test if RAO present
	inline
	bool rao_present() const {
	return rao_presence;
	}

	/// set outgoing Interface index
	inline
	void set_if_index(uint16 value) {
	if_index = value;
	}

	/// get outgoing Interface index
	inline
	uint16 get_if_index() const {
	    return if_index;
	}

	/// unset outgoing Interface index
	inline
	void unset_if_index() {
	    if_index = 0;
	}





    private:
	protocol_t proto;
	port_t port;
	uint8 prefix;

	uint16 rao;
	uint16 ip_ttl;
	bool rao_presence;
	uint16 if_index;

    }; // end appladdress


inline
appladdress::appladdress(const sockaddr_in6& sockaddr, protocol_t prot)
    : hostaddress(sockaddr.sin6_addr), proto(prot), port(ntohs(sockaddr.sin6_port)), rao(0), ip_ttl(0), rao_presence(false), if_index(0)
{
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for sockaddr_in6");
}

/** Constructor sets address type and clears port sets prefix to 32 (ipv4). */
inline
appladdress::appladdress() : hostaddress(),
			     proto(0),
			     port(0),
			     prefix(32),
     			     rao(0),
			     ip_ttl(0),
			     rao_presence(false),
			     if_index(0)

{
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for bool ipv4=" << ipv4);
  set_subtype(ipv4flag);
} // end constructor

inline
appladdress::appladdress(const appladdress& app) : hostaddress(app),
						   proto(app.proto),
						   port(app.port),
						   prefix(app.prefix),
						   rao(app.rao),
						   ip_ttl(app.ip_ttl),
						   rao_presence(app.rao_presence),
						   if_index(app.if_index)

{
    //Log(DEBUG_LOG,LOG_NORMAL,"appladdress", "Copy address constructor called for appladdress& app:" << app);
    //DLog("appladdress", "UDSsocket copied: " << uds_socket);
  //DLog("appladdress", "ip_ttl: " << ip_ttl << " if_index: " << if_index);


    set_subtype(ipv4flag);
} // end copy constructor

/** Initialize with the given host address, protocol ID and port number. */
inline
appladdress::appladdress(const hostaddress& h, protocol_t prot, port_t p)
    : hostaddress(h),
      proto(prot),
      port(p),
      prefix(0),
      rao(0),
      ip_ttl(0),
      rao_presence(false),
      if_index(0)
{
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for (const hostaddress& h, protocol_t prot, port_t p)");

  set_subtype(ipv4flag);
} // end constructor(hostaddress,prot,port)

/** Initialize with the given host address, protocol name and port number.
 * If no protocol ID can be found in the protocol database, it is set to 0.
 */
inline
appladdress::appladdress(const hostaddress& h, const char* pname, port_t p, bool *res)
  : hostaddress(h),
    proto(tsdb::getprotobyname(pname,res)),
    port(p),
    prefix(0),
    rao(0),
    ip_ttl(0),
    rao_presence(false),
    if_index(0)

{
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for (const hostaddress& h, const char* pname, port_t p, bool *res)");

    set_subtype(ipv4flag);
} // end constructor(hostaddress,pname,port)

/** Initialize from string, protocol ID and port.
 * If the string does not contain a vaild IP address, it is set to all 0 by
 * the hostaddress constructor.
 */
inline
appladdress::appladdress(const char* str, protocol_t prot, port_t p, bool *res)
  : hostaddress(str,res),
    proto(prot),
    port(p),
    prefix(0),
    rao(0),
    ip_ttl(0),
    rao_presence(false),
    if_index(0)
{
    set_subtype(ipv4flag);
} // end constructor(string,prot,port)

/** Initialize from string, protocol name and port.
 * If the string does not contain a vaild IP address, it is set to all 0 by
 * the hostaddress constructor.
 * If no protocol ID can be found in the protocol database, it is set to 0.
 */
inline
appladdress::appladdress(const char* str, const char* pname, port_t p, bool *res)
  : hostaddress(str,res),
    port(p),
    prefix(0),
    rao(0),
    ip_ttl(0),
    rao_presence(false),
    if_index(0)
{
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for (const char* str, const char* pname, port_t p, bool *res)");

    register bool tmpres = false;
    proto = tsdb::getprotobyname(pname,&tmpres);
    if (res) *res = ((*res) && tmpres);
    set_subtype(ipv4flag);
} // end constructor(string,pname,port)

/** Initialize from string, protocol name and port name.
 * If the string does not contain a vaild IP address, it is set to all 0 by
 * the hostaddress constructor.
 * If no protocol ID can be found in the protocol database, it is set to 0.
 * If no port number can be found in the service database, it is set to 0.
 */
inline
appladdress::appladdress(const char* str, const char* pname, const char* portname, bool *res)
    : hostaddress(str,res),
      prefix(0),
      rao(0),
      ip_ttl(0),
      rao_presence(false),
      if_index(0)
{
  //Log(DEBUG_LOG,LOG_NORMAL,"address","address constructor called for (const char* str, const char* pname, const char* portname, bool *res)");

    bool res1 = false;
    bool res2 = false;
    proto = tsdb::getprotobyname(pname,&res1);
    port = tsdb::get_portnumber(portname,proto,&res2);
    if (res) *res = ((*res) && res1 && res2);
    set_subtype(ipv4flag);
    prefix = 0;
} // end constructor(string,pname,portname)

/** Assigns the given application address by using hostaddress::operator=(). */
inline
appladdress&
appladdress::operator=(const appladdress& app)
{
	hostaddress::operator=(app);
	proto = app.proto;
	port = app.port;
	prefix = app.prefix;
	ip_ttl = app.ip_ttl;
	rao_presence = app.rao_presence;
	rao = app.rao;
	if_index = app.if_index;
	return *this;
} // end operator=


/** Set port and return old value. */
inline
port_t appladdress::set_port(port_t p) {
	register port_t op = port;
	port = p;
	return op;
} // end set_port



/** Set port and return old value.
 * If the port name is not found in the service database, port is set to 0.
 */
inline
port_t appladdress::set_port(const char* pname, bool *res) {
	register port_t op = port;
	port = tsdb::get_portnumber(pname,proto,res);
	return op;
} // end set_port

/** Set port and return old value.
 * If the port name is not found in the service database, port is set to 0.
 */
inline
port_t appladdress::set_port(const string& pname, bool *res) {
	register port_t op = port;
	port = tsdb::get_portnumber(pname,proto,res);
	return op;
} // end set_port

inline
port_t appladdress::get_port() const { return port; }

inline
string appladdress::get_port_name(bool *res) const {
	return tsdb::get_portname(port,proto,res);
} // end get_port_name

/** Set protocol ID and return old value. */
inline
protocol_t appladdress::set_protocol(protocol_t p) {
	register protocol_t o = proto;
	proto = p;
	return o;
} // end set_protocol

/** Set protocol ID and return old value.
 * If no protocol ID can be found in the protocol database, it is set to 0.
 */
inline
protocol_t appladdress::set_protocol(const char* pname, bool *res) {
	register protocol_t o = proto;
	proto = tsdb::getprotobyname(pname,res);
	return o;
} // end set_protocol

/** Set protocol ID and return old value.
 * If no protocol ID can be found in the protocol database, it is set to 0.
 */
inline
protocol_t appladdress::set_protocol(const string& pname, bool *res) {
	register protocol_t o = proto;
	proto = tsdb::getprotobyname(pname,res);
	return o;
} // end set_protocol

inline
protocol_t appladdress::get_protocol() const { return proto; }

inline
string appladdress::get_protocol_name(bool *res) const {
	return tsdb::getprotobynumber(proto,res);
} // end get_protocol_name

inline
size_t appladdress::get_hash() const {
	uint32 tmp = (proto<<16)+port;
	return (hostaddress::get_hash() ^ tmp);
} // end get_hash

inline
void
appladdress::get_sockaddr(struct sockaddr_in6& sa) const
{
  if (!ipv4flag)
  {
    sa.sin6_family= PF_INET6;
    sa.sin6_port  = htons(port);
    sa.sin6_addr  = ipv6addr;
  }
}

/// Network Prefix (or net address)
/** Holds an IP address and a prefix length in bits. */
class netaddress : public hostaddress {
/***** inherited from IE ****/
public:
	virtual netaddress* new_instance() const;
	virtual netaddress* copy() const;
	virtual bool operator==(const address& ie) const;

	/// convert to iPv6
	virtual void convert_to_ipv6();
	/// hash function
	virtual size_t get_hash() const;
	virtual int match_against(const netaddress& na) const;
protected:
	/// set subtype and IPv4 flag
	virtual void set_subtype(bool ipv4);
/***** new members *****/
public:
	/// constructor
	netaddress();
	/// copy constructor
	netaddress(const netaddress& na);
	/// constructor from hostaddress and prefix length
	netaddress(const hostaddress& h, prefix_length_t len = 128);
	/// constructor from string
	netaddress(const char* str, bool *res = NULL);
	/// constructor from string and prefix length
	netaddress(const char* str, prefix_length_t len, bool *res = NULL);
	/// assignment
	netaddress& operator=(const netaddress& na);
	/// assignment
	netaddress& operator=(const hostaddress& ha);

	// set prefix length
	prefix_length_t set_pref_len(prefix_length_t len);
	// get prefix length
	prefix_length_t get_pref_len() const;
	/// comparison for prefixmap
	bool operator<(const netaddress& na) const;

	int rdx_cmp(const netaddress *na, int *pos) const;
private:
	prefix_length_t prefix_length;
}; // end netaddress

inline ostream &operator<<(ostream &out, const netaddress &addr) {
     return out << addr.get_ip_str() << "/" << (int)addr.get_pref_len();
}

/// Unix Domain Socket Address
/** This class can hold a Unix Domain Socket Address OR a Socket Number. */
class udsaddress : public address {

  public:
    virtual udsaddress* new_instance() const;
    virtual udsaddress* copy() const;
    virtual bool operator==(const address& ie) const;

    /// constructor
    udsaddress() : address(UDSAddress) { uds_socket = ""; socknum=0;};
    /// copy constructor
    udsaddress(const udsaddress& h) : address(UDSAddress) { uds_socket = string(h.uds_socket.c_str()); socknum = h.socknum; };
    /// assignment
    udsaddress& operator=(const udsaddress& uds) {
	uds_socket = string(uds.uds_socket);
	socknum = uds.socknum;
	return *this;
    };
    /// constructor from string
    udsaddress(string sockstring): address(UDSAddress) { uds_socket = string(sockstring.c_str()); socknum=0; };
    /// constructor from int
    udsaddress(int num): address(UDSAddress) { socknum = num; uds_socket=""; };
    /// constructor from both
    udsaddress(string sockstring, int num): address(UDSAddress) { socknum = num; uds_socket=string(sockstring.c_str()); };
    /// destructor
    virtual ~udsaddress() {};

    /// hash function
    virtual size_t get_hash() const;

private:
    /// uds socket string
    string uds_socket;
    /// socket number
    int socknum;

public:

/** Set UDS socket path. */
inline
void set_udssocket(string socket) {
    uds_socket = socket;
} // end set_uds socket path


/** Get UDS socket path. */
inline
const string get_udssocket() const {
    return uds_socket;
} // end get_udspath


/** Set Socket Number */
inline
void set_socknum(int socket) {
    socknum = socket;
} // end set_socknum

/** Get Socket Number */
inline
const int get_socknum() const {
    return socknum;
} // end get_socknum



}; // end udsaddress

template <typename _dT>
class RadixTrie {
public:
	typedef _dT	data_type;

	struct node {
		node(netaddress *k, data_type *d) : key(k), data(d) {
			left = right = this;
			index = 0;
		}
		~node() {
			if (data)
				delete data;
			if (key)
				delete key;
			if (left != 0 && left->index > index)
				delete left;
			if (right != 0 && right->index > index)
				delete right;
		}
		node			*left;
		node			*right;
		netaddress		*key;
		data_type		*data;
		int			 index;
	};

	RadixTrie() {
		netaddress *def;
		def = new netaddress("0.0.0.0", (prefix_length_t)0);
		v4head = new node(def, 0);
		def = new netaddress("::", (prefix_length_t)0);
		v6head = new node(def, 0);
	}

	~RadixTrie() {
		delete v4head;
		delete v6head;
	}

	node *insert(netaddress &key, data_type &dat) {
		node *a, *b = NULL, *c, *n, *m;
		int cmp = 0, pos = 0;

		c = a = key.is_ipv4() ? v4head : v6head;

		// search the tree as long as there are bits left in key
		while (key.get_pref_len() > a->index) {

			// compare key to key in node a from position a->index
			pos = a->index - 1;
			cmp = key.rdx_cmp(a->key, &pos);
			if (pos < 0)
				abort();

			// in case of a perfect match
			if ((cmp == 0) &&
			    (a->key->get_pref_len() == key.get_pref_len())) {
			    // replace data in node
				if (a->data)
					delete a->data;
				a->data = &dat;
				return a;
			}

			if (cmp == 0)
				break;

			// select node to continue the search based on the
			// first different bit between a and key
			b = cmp < 0 ? a->left : a->right;

			// we reached a dead end
			if (b->index <= a->index)
				break;

			// the first difference was before a's bitmask ended
			// we must not make any more progress
			if (pos <= a->key->get_pref_len())
				break;

			c = a;
			a = b;
		}

		// first check if a and key share a common prefix
		if ((key.get_pref_len() == a->key->get_pref_len()) ||
		   (pos > a->index && pos <= a->key->get_pref_len())) {
		   	int opos = pos;

		   	// make sure we didn't just miss the perfect match
			pos = a->index;
			cmp = key.rdx_cmp(a->key, &pos);
			if (cmp == 0 &&
			    (a->key->get_pref_len() == key.get_pref_len())) {
			    // replace data in node
				if (a->data)
					delete a->data;
				a->data = &dat;
				return a;
			}

		   	// create a node with that prefix
		   	pos = opos;
			n = new node(new netaddress(key), 0);
			n->key->set_pref_len(pos - 1);

			// hook it to the previous node(c)
			pos = c->index;
			cmp = n->key->rdx_cmp(c->key, &pos);
			n->index = pos;
			if (n->index <= c->index) {
				cout << "DEAD NODE INSERTION!!!" << endl;
				abort();
			}
			if (cmp < 0) {
				if (c->left != a) {
					cout << "TREE CORRUPTION!!!" << endl;
					abort();
				}
				c->left = n;
			} else {
				if (c->right != a) {
					cout << "TREE CORRUPTION!!!" << endl;
					abort();
				}
				c->right = n;
			}

			// hook the current node(a) to the common prefix
			// node(n)
			pos = n->index;
			cmp = a->key->rdx_cmp(n->key, &pos);
			a->index = pos;
			if (a->index <= n->index) {
				cout << "DEAD NODE INSERTION!!!" << endl;
				abort();
			}
			if (cmp < 0)
				n->left = a;
			else
				n->right = a;

			// create a new node(m) for the insert
			m = new node(new netaddress(key), &dat);
			// hook node(m) to the common prefix node(n)
			pos = n->index;
			cmp = m->key->rdx_cmp(n->key, &pos);
			m->index = pos;
			if (cmp < 0) {
				if (n->left == a) {
					cout << "OVERWRITE!!!" << endl;
					abort();
				}
				n->left = m;
			} else {
				if (n->right == a) {
					cout << "OVERWRITE!!!" << endl;
					abort();
				}
				n->right = m;
			}

			return m;
		}

		// c is a prefix of key, key is a prefix of a
		if (a->index >= pos) {
			// create a new node for the key
			n = new node(new netaddress(key), &dat);
			// hook it to the previous node(c)
			n->index = pos;
			if (n->index <= c->index) {
				cout << "DEAD NODE INSERTION!!!" << endl;
				abort();
			}
			if (cmp < 0) {
				if (c->left != a) {
					cout << "TREE CORRUPTION!!!" << endl;
					abort();
				}
				c->left = n;
			} else {
				if (c->right != a) {
					cout << "TREE CORRUPTION!!!" << endl;
					abort();
				}
				c->right = n;
			}

			// hook the current node(a) to the newly created
			// node(n)
			pos = n->index;
			cmp = a->key->rdx_cmp(n->key, &pos);
			a->index = pos;
			if (a->index <= c->index) {
				cout << "DEAD NODE INSERTION!!!" << endl;
				abort();
			}
			if (cmp < 0)
				n->left = a;
			else
				n->right = a;

			return n;
		}

		// reached a deadend, simply add a new node
		n = new node(new netaddress(key), &dat);
		n->index = pos;
		if (n->index <= a->index) {
			cout << "DEAD NODE INSERTION!!!" << endl;
			abort();
		}
		if (b->index <= a->index) {
			if (cmp < 0)
				a->left = n;
			else
				a->right = n;
		} else {
			cout << "TREE CORRUPTION!!!" << endl;
			abort();
		}

		return n;
	}

	node *lookup_node(netaddress &key, bool lpfm = true,
	    bool with_data = true) {
		node *a, *b, *c, *lpfn;
		int cmp, pos = 0;

		lpfn = 0;
		c = b = a = key.is_ipv4() ? v4head : v6head;
		if (lpfm) {
			if (!with_data)
				lpfn = a;
			else if (a->data)
				lpfn = a;
		}

		// search the tree as long as there are bits left in key
		while (key.get_pref_len() > a->index) {

			// compare key to key in node a from pos
			pos--;
			cmp = key.rdx_cmp(a->key, &pos);

			// all of key consumed
			if (cmp == 0) {
				// key is less specific than a
				if (key.get_pref_len() <
				    a->key->get_pref_len())
					return lpfm ? lpfn : NULL;

				// key is an exact match for a
				if (key.get_pref_len() >=
				    a->key->get_pref_len()) {
					if (!with_data)
						return a;
					if (a->data)
						return a;
					return lpfm ? lpfn : NULL;
				}
			}

			// all of a consumed -> a is a prefix of key
			if (pos > a->key->get_pref_len()) {
				if (!with_data)
					lpfn = a;
				else if (a->data)
					lpfn = a;
			}

			// select node to continue the search based on the
			// first different bit between a and key
			b = cmp < 0 ? a->left : a->right;

			// we reached a dead end
			if (b->index <= a->index)
				break;

			c = a;
			a = b;
		}

		return lpfm ? lpfn : NULL;
	}

	data_type *lookup(netaddress &key, bool lpfm = true) {
		node *n = lookup_node(key, lpfm);

		return n ? n->data : NULL;
	}

	bool remove(netaddress &key) {
		node *n = lookup_node(key);

		if (n && n->data) {
			delete n->data;
			n->data = NULL;
		}

		return (n != 0);
	}

	bool remove_all(netaddress &key) {
		node *n = lookup_node(key, false, false);

		if (n == 0)
			return false;

		if (n->data) {
			delete n->data;
			n->data = NULL;
		}

		if (n->left->index > n->index) {
			delete n->left;
			n->left = n;
		}
		if (n->right->index > n->index) {
			delete n->right;
			n->right = n;
		}

		return true;
	}

	void print() {
		cout << "v4_TREE: " << endl;
		print_node(v4head);
		cout << "v6_TREE: " << endl;
		print_node(v6head);
	}

	void print_node(node *x, bool decent = true) {
		if (x && x->key) {
			cout << "node: " << x << " key: " <<  *x->key;
			if (x->data != 0)
				cout << " data: " << x->data;
			else
				cout << " data: NULL";
			cout << " index: " << x->index << endl;
			cout << "\tleft: " << x->left << " right: " << x->right << endl;
			if (decent) {
				if (x->left->index > x->index)
					print_node(x->left);
				if (x->right->index > x->index)
					print_node(x->right);
			}
		}
	}

private:
	struct node *v4head;
	struct node *v6head;
};


/*
 * AddressList
 */

class AddressList {
public:
	class AddrProperty {
		public:
			AddrProperty(const char *name) {
				pname = new string(name);
			}
			~AddrProperty() {
				delete pname;
			};

			string *pname;
	};

	// Default properties, N.B. Ignore will remove all other properties
	static AddrProperty *LocalAddr_P;
	static AddrProperty *ConfiguredAddr_P;

private:
	// This is special
	static AddrProperty *IgnoreAddr_P;
	static AddrProperty *AnyAddr_P;

public:
	struct ltstr {
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};
	struct ltna {
		bool operator()(const netaddress &s1, const netaddress &s2)
		    const {
		    	if (s1.is_ipv4() != s2.is_ipv4())
		    		return (s1.is_ipv4());
			int cmp, pos = 0;
			cmp = s1.rdx_cmp(&s2, &pos);
			return (cmp < 0);
		}

	};
	typedef set<char *, ltstr>	iflist_t;
	typedef set<netaddress, ltna>	addrlist_t;

	AddressList();
	~AddressList();

	// Configure by interfaces
	// Return a list of all local interfaces, which are monitored
	iflist_t *get_interfaces();
	// Begin interface list (otherwise the following have no effect)
	// If start_empty is true use an empty list, otherwise add all
	// currently configured interfaces
	bool by_interface(bool start_empty = true);
	// Add interface to monitoring
	bool add_interface(char *name);
	// Remove interface from monitoring
	bool del_interface(char *name);

	// Manage properties
	// If 'all' is true operate on all matching addresses as well
	bool add_property(netaddress &na, AddrProperty *p = ConfiguredAddr_P,
	    bool propagate = true);
	bool del_property(netaddress &na, AddrProperty *p = ConfiguredAddr_P,
	    bool propagate = true);
	inline bool purge_properties(netaddress &na, bool prop = true) {
		return del_property(na, AnyAddr_P, prop);
	};
	bool add_host_prop(const char *name, AddrProperty *p = ConfiguredAddr_P);
	bool del_host_prop(const char *name, AddrProperty *p = ConfiguredAddr_P);
	inline bool purge_host_prop(const char *name) {
		return del_host_prop(name, AnyAddr_P);
	}

	// Special property
	bool ignore(netaddress &na, bool propagate = true);
	bool unignore(netaddress &na, bool propagate = true);
	bool ignore_bogons(void);
	bool ignore_locals(void);
	bool ignore_loopback(void);

	// Check if the given address has the given property
	bool addr_is(netaddress &na, AddrProperty *prop);
	bool addr_is(const hostaddress &ha, AddrProperty *prop) {
		netaddress na(ha);
		return addr_is(na, prop);
	}
	// Check if the given address is in a network with the given property
	bool addr_is_in(netaddress &na, AddrProperty *prop);
	bool addr_is_in(const hostaddress &ha, AddrProperty *prop) {
		netaddress na(ha);
		return addr_is_in(na, prop);
	}

	addrlist_t *get_addrs(AddrProperty *prop = LocalAddr_P);
	netaddress *get_first(AddrProperty *p = LocalAddr_P, bool IPv4 = true);

	netaddress *get_src_addr(const netaddress &dest, uint32_t *prefs);
private:
	typedef map<AddrProperty *, bool>	propmap_t;
	typedef RadixTrie<propmap_t>		addr2prop_t;

	iflist_t *interfaces;
	addr2prop_t prop_trie;

	// Backends for public functions
	void getifaddrs_iflist(iflist_t &list);
	bool getifaddrs_is_local(netaddress &na);
	void getifaddrs_get_addrs(addrlist_t &list);

	void bequeath(addr2prop_t::node *head, AddrProperty *p,
	    bool add = true);
	void collect(addr2prop_t::node *head, AddrProperty *p,
	    addrlist_t &list);
	addr2prop_t::node *collect_first(addr2prop_t::node *head,
	    AddrProperty *p);
};

inline ostream &operator<<(ostream &out, const AddressList::AddrProperty &prop) {
	return out << *prop.pname;
}


/************************************* inline methods ***********************************/

inline
size_t
hostaddress::get_hash() const
{
	return (ipv6addr.s6_addr32[0] ^ ipv6addr.s6_addr32[1] ^ ipv6addr.s6_addr32[2] ^ ipv6addr.s6_addr32[3]);
} // end get_hash

/***** new in hostaddress *****/


/** Initialize a hostaddress object.
 * This calls virtual member set_subtype and therefore sets subtype in all
 * derived class which overwrite this member function correctly.
 */
inline
hostaddress::hostaddress()
	: address(IPv6HostAddress),
	  ipv4flag(false),
	  outstring(NULL)
{
  clear_ip();
  set_subtype(false);
} // end constructor hostaddress


/** Assign h to this object. */
inline
hostaddress&
hostaddress::operator=(const hostaddress& h) {
	address::operator=(h);
	this->set_ip(h);
	if (outstring)
	  delete outstring;
        outstring= 0;
	return *this;
} // end operator=

/** Copy constructor for hostaddress objects */
inline
hostaddress::hostaddress(const hostaddress& h) :
  address(h),
  outstring(NULL)
{
  this->set_ip(h);

  //Log(DEBUG_LOG,LOG_NORMAL,"hostaddress","hostaddress constructor called for const hostaddress& h:"); // << h << " outstring:" << static_cast<void*>(outstring) << " h.outstring:" << static_cast<void*>(h.outstring));

} // end copy constructor hostaddress

/** Check if this is an IPv4 address. */
inline
bool
hostaddress::is_ipv4() const {
	return ipv4flag;
} // end is_ipv4

/** Check if this is an IPv6 address. */
inline
bool
hostaddress::is_ipv6() const {
	return (!ipv4flag);
} // end is_ipv6

/** Check if this is a 6to4 mapped address. */
inline
bool
hostaddress::is_mapped_ip() const
{
  return (ipv4flag) ? false : IN6_IS_ADDR_V4MAPPED(&ipv6addr);
} // end is_mapped_ip

inline
bool
hostaddress::set_ip(const string& str) { return set_ip(str.c_str()); }

inline
/** Delete outstring if it exists. */
hostaddress::~hostaddress() {
	if (outstring)
	{
	  delete[] outstring;
	  outstring= 0;
	}
} // end destructor hostaddress

/** Set IPv4 or IPv6 from string or leave object unchanged.
 * This changes object type.
 * @return true on success.
 */
inline
bool
hostaddress::set_ip(const char *str)
{
  return (!str) ? false :
                 ( strchr(str,':') ? set_ipv6(str) : set_ipv4(str)); 	// which IP version?

} // end set_ipv


/** Lookup the host name associated with the current IP address. */
inline
string hostaddress::get_host_name(bool *res) const
{
  return  ipv4flag ? tsdb::get_hostname(ipv4addr,res) : tsdb::get_hostname(ipv6addr,res);
} // end get_host_name

/***** inherited from hostaddress *****/

/** Set subtype and IPv4 flag. This does NOT clear the outstring buffer.
 * Use clear_ip().
 */
inline
void
appladdress::set_subtype(bool ipv4)
{
	ipv4flag = ipv4;
	subtype = (ipv4) ?  IPv4ApplAddress : IPv6ApplAddress;
} // end set_subtype

inline
prefix_length_t netaddress::get_pref_len() const { return prefix_length; }

inline
size_t netaddress::get_hash() const {
	return (hostaddress::get_hash() ^ prefix_length);
} // end get_hash

inline
int
netaddress::match_against(const netaddress& na) const
{
	// compare prefix lengths
  return (prefix_length<na.prefix_length) ? -1 : hostaddress::match_against(na);
} // end match_against


inline
ostream &operator<<(ostream &out, const appladdress &addr) {
     if (addr.is_mapped_ip()) return out << "[IPv4-mapped address]: "  << addr.get_ip_str() << ":" << (int)addr.get_port() << ", " << addr.get_protocol_name();
    return out << "[IP address]: " << addr.get_ip_str() << ":" << (int)addr.get_port() << ", " << addr.get_protocol_name();
}

inline
ostream &operator<<(ostream &out, const udsaddress &addr) {
    if (addr.get_socknum()) return out << "[Socketnumber]: " << addr.get_socknum();
    return out << "[Unix Domain Socket]: " << addr.get_udssocket();
}






inline
size_t udsaddress::get_hash() const {
    size_t tmp2 = 1;
	for (unsigned int i = 0; i<uds_socket.size(); i++) {
	    tmp2 = tmp2 * (int) uds_socket[i];
	}
	return (tmp2 ^ socknum);
} // end get_hash



} // end namespace protlib

/*********************************** hash functions ***********************************/

namespace boost {
/// hostaddress hasher
template <> struct hash<protlib::hostaddress> {
	inline size_t operator()(const protlib::hostaddress& addr) const { return addr.get_hash(); }
}; // end hostaddress hasher

/// appladdress hasher
template <> struct hash<protlib::appladdress> {
	inline size_t operator()(const protlib::appladdress& addr) const { return addr.get_hash(); }
}; // end appladdress hasher

/// udsaddress hasher
template <> struct hash<protlib::udsaddress> {
	inline size_t operator()(const protlib::udsaddress& addr) const { return addr.get_hash(); }
}; // end udsaddress hasher

/// netaddress hasher
template <> struct hash<protlib::netaddress> {
	inline size_t operator() (const protlib::netaddress& addr) const { return addr.get_hash(); }
}; // end netaddress hasher

} // end namespace __gnu_cxx


namespace std {

/// hostaddress equal_to
template <> struct equal_to<protlib::hostaddress> {
	inline bool operator()(const protlib::hostaddress& addr1, const protlib::hostaddress& addr2) const { return addr1.equiv(addr2); }
}; // end hostaddress equal_to

/// appladdress equal_to

template <> struct equal_to<protlib::appladdress> {
	inline bool operator()(const protlib::appladdress& addr1, const protlib::appladdress& addr2) const { return addr1.equiv(addr2); }
}; // end appladdress equal_to

/// netaddress equal_to
template <> struct equal_to<protlib::netaddress> {
	inline bool operator()(const protlib::netaddress& addr1, const protlib::netaddress& addr2) const { return addr1.equiv(addr2); }

}; // end netaddress equal_to

} // end namespace std
#endif // PROTLIB__ADDRESS_H
