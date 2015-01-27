/// ----------------------------------------*- mode: C++; -*--
/// @file setuid.h
/// Change effective user ID in a thread-safe way
/// ----------------------------------------------------------
/// $Id: setuid.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/setuid.h $
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
 * Thread-safe setuid support for linux. 
 * Change effective user ID in a thread-safe way.
 *
 * tsdb::init() must be called before calling setuid::init().
 */

#ifndef THREADSAFE_SETUID_H
#define THREADSAFE_SETUID_H

#include <sys/types.h>
#include <pthread.h>
#include <string>

#include "protlib_types.h"
#include "cleanuphandler.h"

namespace protlib { 

/** @addtogroup protlib
 * @{
 */

/// Thread-safe setuid
/** This class provieds class methods for changing the effective user ID of
 * the current process.
 */
class setuid {
public:
	/// initialize setuid 
	static void init();
	/// cleanup setuid resources
	static void end();
	/// turn on setuid mode
	static void on();
	/// turn off setuid mode
	static void off();
private:
	/// init state
	static bool is_init;
	/// setuid mutex
	static pthread_mutex_t mutex;
	/// setuid counter
	static uint32 count;
	/// file user ID
	static uid_t file_userid;
	/// file user name
	static string file_username;
	/// real user ID
	static uid_t real_userid;
	/// real user name
	static string real_username;
	/// are we using setuid?
	static bool is_setuid;
}; // end class setuid

/// Turn on setuid mode and install cleanup handler.
#define BEGIN_SETUID_MODE protlib::setuid::on(); install_cleanup(call_void_fun,protlib::setuid::off)
#define END_SETUID_MODE uninstall_cleanup(1)

//@}

} // end namespace protlib

#endif
