/// ----------------------------------------*- mode: C++; -*--
/// @file tp.h
/// generic interface for sending/receiving network messages via a transport protocol 
/// ----------------------------------------------------------
/// $Id: tp.h 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/tp.h $
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
// tp.h - generic transport protocol interface module TP
// ----------------------------------------------------------
// $Id: tp.h 2872 2008-02-18 10:58:03Z bless $
// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/tp.h $
// ==========================================================
//                      
// (C)opyright, all rights reserved by
// - Institute of Telematics, University of Karlsruhe (TH)
// ==========================================================
/** @ingroup protlib
 * This is the interface for sending network messages over a transport 
 * protocol. You can receive messages through queues.
 */

#ifndef _PROTLIB__TP_H_
#define _PROTLIB__TP_H_

#include <string.h>

#include "protlib_types.h"
#include "messages.h"
#include "network_message.h"
#include "address.h"
#include "tperror.h"

namespace protlib {

/** @ingroup protlib
 * @ingroup protlib
 * @{
 */


/// transport protocol base class
/** This is the interface to all transport protocols.
 * It is used by the signaling module to send network messages. 
   // what kinds of messages?
 * Received messages are delivered through FastQueues.
 */
class TP {
public:
	/// constructor
	TP(protocol_t p, const string& pname, const string& tpn, 
	   const unsigned short common_header_length, 
	   bool (*const getmsglength)(NetMsg& netmsg, uint32& msglen),
	   uint32 mp = (uint32)-1);

	/// virtual destructor
	virtual ~TP() = 0;

	/// send a message, message is deleted after it has been sent
        /// use_existing_connection indicates whether a new connection will be established
        /// if required (true means that no connection will be set up if none exists yet)
	virtual void send(NetMsg* msg, const address& addr, bool use_existing_connection= false) = 0;

	/// terminates an existing signaling association/connection
	virtual void terminate(const address& addr) = 0;

	/// check send arguments
	void check_send_args(const NetMsg& msg,
		const address& addr) const;

	/// get protocol ID
	protocol_t get_underlying_protocol() const;

	/// get protocol name
	string get_underlying_protocol_name() const;

	/// get TP name
	string get_tp_name() const;

	/// is it initialized?
	bool is_init() const;

	/// get maximum payload
	uint32 get_max_payload() const;
protected:

	/// transport protocol ID
	const protocol_t protocol;

	/// transport protocol name
	const string protoname;

	/// TP subclass name
	const string tp_name;

        /// what is the length of the common header
        const unsigned short common_header_length;

        /// function pointer to a function that figures out the msg length in number of 4 byte words
        /// it returns false if error occured (e.g., malformed header), result is returned in variable clen_words
        bool (*const getmsglength) (NetMsg& m, uint32& clen_words);

	/// init state
	bool init;
	const uint32 max_payload;
}; // end class TP

/// transport protcol message
/** This message class is used to carry received network messages. */
class TPMsg : public message {
/***** inherited from message *****/
public:
	virtual void clear_pointers();
/***** new *****/
public:
    /// constructor
    TPMsg(NetMsg* m = NULL, address* peer = NULL, address* ownaddr = NULL, TPError* e = NULL, uint16 oif = 0);
    /// destructor
    virtual ~TPMsg();
    /// get peer address
    const address* get_peeraddress() const;
    /// set own address
    const address* get_ownaddress() const;
    /// set peer address
    address* set_peeraddress(address* a);
    /// set own address
    address* set_ownaddress(address* a);
    /// get network message
    NetMsg* get_message() const;
    /// set network message
    NetMsg* set_message(NetMsg* m);
    /// get TP error
    TPError* get_error() const;
    /// set TP error
    TPError* set_error(TPError* e);
    /// set Outgoing Interface
    void set_oif(uint16 iface) { oif=iface; }
    /// get Outgoing Interface
    uint16 get_oif() { return oif; }

private:
    /// peer address
    address* peeraddr;
    /// own address
    address* ownaddr;
    /// network message
    NetMsg* msg;
    TPError* err;
    /// outgoing interface index
    uint16 oif;
}; // end class TPMsg

inline
bool TP::is_init() const { return init; }

/***** class TPMsg *****/

/***** inherited from message *****/

/** Clear all pointer fields. */
inline
void TPMsg::clear_pointers() {
	peeraddr = NULL;
	ownaddr = NULL;
	msg = NULL;
	err = NULL;
} // end clear

/***** new in NetMsg *****/

/** Initialize and set message type and source to 'transport'. */
inline
TPMsg::TPMsg(NetMsg* m, address* p, address* o, TPError* e, uint16 oif) 
  : message(type_transport,qaddr_transport),
    peeraddr(p),
    ownaddr(o),
    msg(m),
    err(e),
    oif(oif)
{} // end constructor TPMsg

/** Dispose NetMsg, address, err and then delete TPMsg. */
inline
TPMsg::~TPMsg() {
	if (msg) delete msg;
	if (peeraddr) delete peeraddr;
	if (ownaddr) delete ownaddr;
	if (err) delete err;
} // end destructor TPMsg

inline
const address* TPMsg::get_peeraddress() const { return peeraddr; }

inline
const address* TPMsg::get_ownaddress() const { return ownaddr; }

inline
address* TPMsg::set_peeraddress(address* peer) {
  register address* oa = peeraddr;
  peeraddr = peer;
  return oa;
} // end set_address

inline
address* TPMsg::set_ownaddress(address* o) {
  register address* oa = ownaddr;
  ownaddr = o;
  return oa;
} // end set_address

inline
NetMsg* TPMsg::get_message() const { return msg; }

inline
NetMsg* TPMsg::set_message(NetMsg* m) {
	register NetMsg* omsg = msg;
	msg = m;
	return omsg;
} // end set_message

inline
TPError* TPMsg::get_error() const { return err; }

inline
TPError* TPMsg::set_error(TPError* e) {
	register TPError* oe = err;
	err = e;
	return oe;
} // end set_error


//@}

} // end namespace protlib

#endif
