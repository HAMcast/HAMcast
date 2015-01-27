/// ----------------------------------------*- mode: C++; -*--
/// @file tp.cpp
/// generic interface for sending/receiving network messages via a transport protocol 
/// ----------------------------------------------------------
/// $Id: tp.cpp 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/tp.cpp $
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
 * This is the interface for sending network messages over a transport 
 * protocol. You can receive messages through queues.
 */

#include "tp.h"
#include "threadsafe_db.h"
#include "logfile.h"

namespace protlib {

/** @addtogroup protlib
 * @ingroup protlib
 * @{
 */

  using namespace log;


/***** class TP *****/

/** Set ID of the underlying transport protocol, e.g. TCP, SCTP or UDP.
 * Init is set false here, set to true in constructors of derived classes if
 * initialization is done.
 * If the maximum payload is greater than the maximum size of a network
 * message, it is decreased.
 */
TP::TP(protocol_t p, const string& pname, const string& tpn, 
       const unsigned short common_header_length, 
       bool (*const getmsglength)(NetMsg& netmsg, uint32& msglen),
       uint32 mp) 
	: protocol(p), protoname(pname), tp_name(tpn), 
	  common_header_length(common_header_length), 
	  getmsglength(getmsglength),
	  init(false), 
	  max_payload((mp<NetMsg::max_size)?mp:(NetMsg::max_size))
          {}

/** TP destructor does nothing. */
TP::~TP() { init = false; }

/** Get the ID of the underlying transport protocol. */
protocol_t TP::get_underlying_protocol() const { return protocol; }

/** Get the name of the underlying transport protocol. */
string TP::get_underlying_protocol_name() const { return protoname; }

/** Get the name of this TP implementation. */
string TP::get_tp_name() const { return tp_name; }

uint32 TP::get_max_payload() const { return max_payload; }

/* @param msg NetMsg to send
 * @param addr destination address
 * @return true if args are OK.
 */

void TP::check_send_args(const NetMsg& msg, const address& addr)
		const {
  if (!init) {
    Log(ERROR_LOG,LOG_NORMAL, "TP", "TP::check_send_args: " << tp_name << " not initialized");
    throw TPErrorArgsNotInit();
  } // end if not init
  
  if ((msg.get_size()==0) || (msg.get_size()>max_payload)) {
    Log(ERROR_LOG,LOG_NORMAL, "TP", "TP::check_send_args: NetMsg empty or too big. Size: " << msg.get_size() << ", " << tp_name << ", max_payload " << max_payload);
    throw TPErrorPayload();
  } // end if too big

} // end check_send_args 


//@}

} // end namespace protlib
