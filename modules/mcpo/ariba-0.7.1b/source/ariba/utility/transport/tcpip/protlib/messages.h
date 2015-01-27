/// ----------------------------------------*- mode: C++; -*--
/// @file messages.h
/// internal messages sent between modules and other components
/// ----------------------------------------------------------
/// $Id: messages.h 3013 2008-05-15 16:12:27Z roehricht $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/messages.h $
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
 * @file
 * These messages are sent between modules and other DRM components.
 */

#ifndef _PROTLIB__MESSAGE_H_
#define _PROTLIB__MESSAGE_H_

#include <pthread.h>
#include <boost/unordered_set.hpp>

#include "protlib_types.h"
#include "fqueue.h"

namespace protlib {

/** @addtogroup protlib
 * @{
 */

// class FastQueue is declared in fqueue.h
class FastQueue;

// class Context is declared in context.h
class Context;

/// internal messages
/** Base class of all internal drm messages passed between modules. */
class message {
public:
	/// message type
	/** Each subclass of class message has its own unique type. */
	enum type_t {
		type_transport,
		type_timer,
		type_signaling,
		type_context,
		type_info,
		type_routing,
		type_API
	}; // end type_t

	/// source ID
	/** Identifies the module which sent the message.
	 *  If you add an id here, please add also the corresponding string in qaddr_string
	 */
	enum qaddr_t {
		qaddr_unknown, // used as return value in NSLPtable.get_address() if there is no address found
		qaddr_transport,
		qaddr_timer,
		qaddr_coordination,
		qaddr_coordination_internal,
		qaddr_signaling,
		qaddr_policy,
		qaddr_resource,
		qaddr_routing,
		qaddr_accounting,
		qaddr_router_config,
		qaddr_tp_over_sctp,
		qaddr_tp_over_tcp,
		qaddr_tp_over_tls_tcp,
		qaddr_tp_over_udp,
		qaddr_tp_queryencap,
		qaddr_qos_nslp_timerprocessing,
		qaddr_qos_nslp_coordination,
		qaddr_qos_nslp_signaling,
		qaddr_appl_qos_signaling,
		qaddr_qos_appl_signaling,
		qaddr_gui,
		qaddr_api_0,
		qaddr_api_1,
		qaddr_api_2,
		qaddr_api_3,
		qaddr_api_4,
		qaddr_api_5,
		qaddr_api_6,
		qaddr_api_7,
		qaddr_api_wrapper_input,
		qaddr_tp_over_uds,
		qaddr_uds_appl_qos	// receives messages from an external client via UDS
	}; // end qaddr_t

	/// message ID
	/** Each message has an ID.
	 * Message IDs are not unique, you can use the same ID e.g. for request
	 * and response. You cannot use an unused message ID, you can just reuse
	 * an already used ID. ID 0 sets an unused ID.
	 * Since id_t is 64-bit long, you can send 10^10 messages per seconds
	 * for over 30 years without reusing an ID.
	 */
	typedef gp_id_t id_t;
	/// constructor
	message(type_t t, qaddr_t s = qaddr_unknown, id_t id = 0);
	/// destructor
	virtual ~message();
	/// get ID
	id_t get_id() const;
	/// set ID or generate a new one
	id_t set_id(id_t id);
	/// get new ID
	id_t new_id();
	/// get message type
	type_t get_type() const;
	/// get source module queue
	FastQueue *get_source_queue() const;
	/// get source ID
	qaddr_t get_source() const;
	/// set source ID
	qaddr_t set_source(qaddr_t s);
	/// get name of message source
	const char* get_qaddr_name() const;
	/// get source name
	static const char* get_qaddr_name(qaddr_t s);
	/// get name of message type
	const char* get_type_name() const { return type_string[type]; }
	/// get type name
	static const char* get_type_name(type_t t) { return type_string[t]; }
	/// send the message to a queue
	bool send(qaddr_t src, FastQueue* destqueue, bool exp = false);
	/// send the message
	bool send(qaddr_t src, qaddr_t dest, bool exp = false);
	// @{
	/// send the message to dest
	bool send_to(qaddr_t dest, bool exp = false);
	bool send_to(FastQueue* destqueue, bool exp = false);
	// @}
	/// send the message back
	bool send_back(qaddr_t from, bool exp = false);
	//@{
	/// send or delete the message
	void send_or_delete(qaddr_t src, qaddr_t dest, bool exp = false) {	if (!send(src,dest,exp)) delete this; }
	void send_to_or_delete(qaddr_t dest, bool exp = false) { if (!send_to(dest,exp)) delete this; }
	void send_back_or_delete(qaddr_t src, bool exp = false) { if (!send_back(src,exp)) delete this; }
	//@}
	/// clear all pointers
	virtual void clear_pointers();
private:
	const type_t type;
	qaddr_t source;
	id_t msg_id;
 	/// printable message source names
	static const char* const qaddr_string[];
 	/// printable message typee names
	static const char* const type_string[];
}; // end class message



//@}

} // end namespace protlib

namespace boost {

/// message pointer hasher
/** Hash value is the address of the object pointed to. */
template <> struct hash<protlib::message*> {
	inline size_t operator()(protlib::message* m) const { return (size_t)m; }
}; // end msgp_hash

} // end namespace __gnu_cxx

#endif // _PROTLIB__MESSAGE_H_
