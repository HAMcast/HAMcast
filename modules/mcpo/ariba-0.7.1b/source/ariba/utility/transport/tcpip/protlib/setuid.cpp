/// ----------------------------------------*- mode: C++; -*--
/// @file setuid.cpp
/// Change effective user ID in a thread-safe way.
/// ----------------------------------------------------------
/// $Id: setuid.cpp 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/setuid.cpp $
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

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "setuid.h"
#include "threadsafe_db.h"
#include "cleanuphandler.h"
#include "logfile.h"


namespace protlib { 

/** @addtogroup protlib
 * @{
 */

  using namespace log;

void setuid::init() {
	if (is_init) {
	  Log(ERROR_LOG,LOG_CRIT, "setuid", "Tried to initialize setuid although already initialized.");
	} else {
		pthread_mutex_init(&mutex,NULL);
		count = 0;
		file_userid = ::geteuid();
		real_userid = ::getuid();
		is_setuid = (real_userid!=file_userid);
		file_username = tsdb::get_username(file_userid);
		real_username = tsdb::get_username(real_userid);
		is_init = true;
		if (is_setuid) {
		  Log(INFO_LOG,LOG_CRIT, "setuid", "Setuid-bit is set, euid is " << file_userid << " " << file_username << ", realuid is " << real_userid << " " << real_username << ". Turning off setuid as soon as possible");
		  count = 1;
		  off();
		} else {
		  Log(INFO_LOG,LOG_CRIT, "setuid", "Setuid-bit is not set or euid and ruid are equal. setuid::on() and setuid::off() will do nothing");
		} // end if is_setuid
	} // end if is_init
} // end init

void setuid::end() {
	if (is_init) {
		is_init = false;
		pthread_mutex_destroy(&mutex);
		count = 0;
		// turn off setuid
		if (is_setuid) {
		  Log(INFO_LOG,LOG_CRIT, "setuid", "setuid::end() turn off setuid. Switching (maybe permamently) to " << real_userid << " " << real_username << " using setuid()");
		  ::setuid(real_userid);
		} // end if is_setuid
	} else {
	  Log(ERROR_LOG,LOG_CRIT, "setuid", "Tried to end setuid although not initialized.");
	} // end if is_init
} // end end

void setuid::on() {
	if (is_setuid) {
		if (is_init) {
			pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
			if (count==0) {
			  Log(INFO_LOG,LOG_CRIT, "setuid", "setuid::on(): setting euid to " << file_userid << " " << file_username);
			  int status;
                          #ifdef _POSIX_SAVED_IDS
			  status = seteuid(file_userid);
				#else
					status = setreuid(real_userid,file_userid);
				#endif
				if (status<0) {
				  Log(ERROR_LOG,LOG_ALERT, "setuid", "setuid::on(): error " << strerror(errno));
				} else count++;
			} else {
			  Log(INFO_LOG,LOG_CRIT, "setuid", "setuid::on(): setuid already on");
			  count++;
			} // end if count
			pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
		} else {
		  Log(ERROR_LOG,LOG_CRIT, "setuid", "Tried to use setuid although not initialized.");
		} // end if is_init
	} // end if is_setuid
} // end on

void setuid::off() {
	if (is_setuid) {
		if (is_init) {
			pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
			if (count==1) {
				int status;
				#ifdef _POSIX_SAVED_IDS
					status = seteuid(real_userid);
				#else
					status = setreuid(file_userid,real_userid);
				#endif
				if (status<0) {
				  Log(ERROR_LOG,LOG_ALERT, "setuid", "setuid::off(): error " << strerror(errno));
				} else {
				  count = 0;
				  Log(INFO_LOG,LOG_CRIT, "setuid", "setuid::off(): set euid to " << real_userid << " " << real_username);
				} // end if count
			} else if (count==0) {
			  Log(INFO_LOG,LOG_CRIT, "setuid", "setuid::off(): setuid already off");
			} else {
			  Log(INFO_LOG,LOG_CRIT, "setuid", "setuid::off(): setuid still on");
			  count--;
			} // end if count
			pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
		} else {
		  Log(ERROR_LOG,LOG_CRIT, "setuid", "Tried to use setuid although not initialized.");
		} // end if is_init
	} // end if is_setuid
} // end off

bool setuid::is_init = false;

pthread_mutex_t setuid::mutex =
#ifdef _DEBUG
    PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#else
    PTHREAD_MUTEX_INITIALIZER;
#endif

uint32 setuid::count = 0;

uid_t setuid::file_userid = 65534;

string setuid::file_username = "nobody";

uid_t setuid::real_userid = 65534;

string setuid::real_username = "nobody";

bool setuid::is_setuid = true; // important!

//@}

} // end namespace protlib
