/// ----------------------------------------*- mode: C++; -*--
/// @file messages.cpp
/// These messages are sent internally between threads (modules)
/// ----------------------------------------------------------
/// $Id: messages.cpp 3013 2008-05-15 16:12:27Z roehricht $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/messages.cpp $
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
 * These messages are sent internally between threads (modules).
 */

#include "messages.h"
//#include "context.h"
#include "queuemanager.h"
#include "cleanuphandler.h"
#include "threadsafe_db.h"

namespace protlib {

/** @addtogroup protlib
 * @{
 */

/***** class message *****/

/** This array contains printable names of messages sources. */
const char* const message::qaddr_string[] = {
	"UNKNOWN",
	"TransportProtocol",
	"TimerModule",
	"Coordination (ext)",	// external
	"Coordination (int)",	// internal
	"Signaling",
	"Policy",
	"ResourceModule",
	"Routing",
	"Accounting",
	"RouterConfiguration",
	"TPoverSCTP",
	"TPoverTCP",
	"TPoverTLS_TCP",
	"TPoverUDP",
	"TPoverQueryEncapsulation",
	"QNSLP-TimerProc",
	"QNSLP-Coordinator",
	"QNSLP-Signaling",
	"APPL-QNSLP-Signaling",
	"QNSLP-APPL-Signaling",
	"GUI",
	"API-0",
	"API-1",
	"API-2",
	"API-3",
	"API-4",
	"API-5",
	"API-6",
	"API-7",
	"API Wrapper Input (from TP)",
	"TPoverUDS",
	"QoS NSLP Client API over UDS",
	"(INVALID)"
}; // end qaddr_string

/** This array contains printable names of message types. */
const char* const message::type_string[] = {
	"TPMsg",
	"TimerMsg",
	"SignalingMsg",
	"ContextMsg",
	"InfoMsg",
	"RoutingMsg",
	"APIMsg"
}; // end type_string

/** Set message type, source ID and source queue.
 * Set message ID to id if possible, otherwise generate a new ID.
 */
message::message(type_t t, qaddr_t s, id_t id) 
: type(t), source(s) {
	if ((!id) || (!set_id(id))) new_id();
} // end constructor

/** Destructor does nothing for this class. */
message::~message() {}

message::id_t message::get_id() const { return msg_id; }

message::id_t message::set_id(id_t id) {
	if (!id) {
		new_id();
		return 0;
	} else {
		msg_id = id;
		return id;
	} // end if id
} // end set_id

/** Generate an unused ID. */
message::id_t message::new_id() {
	msg_id = 0;
	while (!msg_id) msg_id = tsdb::get_new_id64();;
	return msg_id;
} // end new_id

/** Get the message type. */
message::type_t message::get_type() const { return type; }

FastQueue* message::get_source_queue() const {
	return QueueManager::instance()->get_queue(source);
} // end get_source_queue

message::qaddr_t message::get_source() const {
	return source;
} // end get_source

/** Set source ID and return old value. */
message::qaddr_t message::set_source(qaddr_t s) {
	register qaddr_t os = source;
	source = s;
	return os;
} // end set_source

/** Returns a pointer to a string containing a printable name of the message 
 * source.
 */
const char* message::get_qaddr_name() const {
	return qaddr_string[source];
} // end get_source_name

/** Get the name of the given source. */
const char* message::get_qaddr_name(qaddr_t s) {
	return qaddr_string[s];
} // end get_source_name

/** Send this message to destqueue.
 * @returns false if destqueue is NULL or queue does not accept the message.
 */
bool message::send(qaddr_t src, FastQueue* destqueue, bool exp) {
	if (!destqueue) return false;
	source = src;
	return destqueue->enqueue(this,exp);
} // end send

/** Send this message.
 * @returns false if destination queue cannot be found or queue does not accept the message.
 */
bool message::send(qaddr_t src, qaddr_t dest, bool exp) {
	FastQueue* destqueue = QueueManager::instance()->get_queue(dest);
	if (!destqueue) return false;
	source = src;
	return destqueue->enqueue(this,exp);
} // end send

/** Send this message.
 * @returns false if destination queue cannot be found or queue does not accept the message.
 */
bool message::send_to(qaddr_t dest, bool exp) {
	FastQueue* destqueue = QueueManager::instance()->get_queue(dest);
	if (destqueue) return destqueue->enqueue(this,exp);
	else return false;
} // end send_to

bool message::send_to(FastQueue* destqueue, bool exp) {
	if (destqueue) return destqueue->enqueue(this,exp);
	else return false;
} // end send_to

/** Send back this message.
 * @returns false if destination queue cannot be found or queue does not accept the message.
 */
bool message::send_back(qaddr_t from, bool exp) {
	FastQueue* destqueue = QueueManager::instance()->get_queue(source);
	if (!destqueue) return false;
	source = from;
	return destqueue->enqueue(this,exp);
} // end send_back

/** Set all pointer fields to NULL. */
void message::clear_pointers() {}


//@}

} // end namespace protlib
