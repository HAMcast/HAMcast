/// ----------------------------------------*- mode: C++; -*--
/// @file tperror.h
/// Errors from TP module
/// ----------------------------------------------------------
/// $Id: tperror.h 2794 2007-09-05 12:01:22Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/tperror.h $
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

#ifndef _PROTLIB__TP_ERROR_H_
#define _PROTLIB__TP_ERROR_H_

#include <string.h>

#include "protlib_types.h"

namespace protlib {

/** @addtogroup protlib
 * @ingroup protlib
 * @{
 */

/// Transport Protocol Error
/** Reports a TP error while connection setup, sending a network message or initialization. */
class TPError : public ProtLibException {
public:
	/// TP error codes
	enum tp_error_t {
	  TPERR_OK,           // everything ok
	  TPERR_BAD_ADDRESS,  // Bad destination address
	  TPERR_BAD_NETMSG,   // not used
	  TPERR_ARGS_NOT_INIT,// arguments not initialized
	  TPERR_UNREACHABLE,  // destination unreachable
	  TPERR_INTERNAL,     // any other internal error
	  TPERR_PAYLOAD,      // maximum payload
	  TPERR_INITFAILED,   // Initialization failed, e.g. during socket setup 
	  TPERR_SENDFAILED,   // send failure
	  TPERR_CONNSETUPFAIL, // connection setup failed
	  TPERR_CLOSEIND,       // close indication (other side closed connection)
	  TPERR_ABORTIND      // abort indication (transport protocol)
	}; // end tp_error_t

	/// constructor
        TPError(tp_error_t e) : errtype(e) {};

	/// get error string
	virtual const char* getstr() const= 0;
	virtual const char *what() const throw() { return getstr(); }
	/// error code
	const tp_error_t errtype;
}; // end class TPError


/***** class TPError *****/

class TPErrorBadDestAddress : public TPError
{
public:
  TPErrorBadDestAddress() : TPError(TPError::TPERR_BAD_ADDRESS) {}
  virtual const char* getstr() const {  return "Bad Destination Address"; }
};


class TPErrorArgsNotInit : public TPError 
{
public:
  TPErrorArgsNotInit() : TPError(TPError::TPERR_ARGS_NOT_INIT) {}
  virtual const char* getstr() const {  return "arguments of TPMsg not initialized"; }
};

class TPErrorUnreachable : public TPError 
{
public:
  TPErrorUnreachable() : TPError(TPError::TPERR_UNREACHABLE) {}
  virtual const char* getstr() const {  return "Destination unreachable"; }
};

class TPErrorInternal : public TPError 
{
public:
  TPErrorInternal() : TPError(TPError::TPERR_INTERNAL) {}
  virtual const char* getstr() const {  return "Internal Transport Protocol Error"; }
};


class TPErrorPayload : public TPError 
{
public:
  TPErrorPayload() : TPError(TPError::TPERR_PAYLOAD) {}
  virtual const char* getstr() const {  return "Payload exceeds maximum transmission unit or empty payload given"; }
};

class TPErrorInitFailed : public TPError 
{
public:
  TPErrorInitFailed() : TPError(TPError::TPERR_INITFAILED) {}
  virtual const char* getstr() const {  return "Initialization of protocol failed"; }
};

class TPErrorSendFailed : public TPError 
{
  int saved_errno; ///< value of errno from send call
public:
  TPErrorSendFailed(int current_errno= 0) : TPError(TPError::TPERR_SENDFAILED), saved_errno(current_errno) {}
  virtual const char* getstr() const {  return "Failure while trying to send a protocol data unit"; }
  int get_reason() const { return saved_errno; } ///< returns saved value of errno from send call
};

class TPErrorConnectSetupFail : public TPError 
{
public:
  TPErrorConnectSetupFail() : TPError(TPError::TPERR_CONNSETUPFAIL) {}
  virtual const char* getstr() const {  return "Connection Setup Failure"; }
};

class TPErrorCloseInd : public TPError 
{
public:
  TPErrorCloseInd() : TPError(TPError::TPERR_CLOSEIND) {}
  virtual const char* getstr() const {  return "Other side closed connection"; }
};

class TPErrorAbortInd : public TPError 
{
public:
  TPErrorAbortInd() : TPError(TPError::TPERR_ABORTIND) {}
  virtual const char* getstr() const {  return "Abort indication, transport protocol indicated failure/abort"; }
};

} // end namespace protlib

#endif // _PROTLIB__TP_ERROR_H_
