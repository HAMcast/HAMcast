/// ----------------------------------------*- mode: C++; -*--
/// @file cleanuphandler.h
/// preprocessor macros to install cleanup handlers for threads
/// ----------------------------------------------------------
/// $Id: cleanuphandler.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/cleanuphandler.h $
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
 * This header file defines a preprocessor macro to 
 * install cleanup handlers.
 *
 * This cannot be done without macros because the pthread library also uses
 * macros for cleanup.
 * References to a common base class of all lockable classes also are not
 * useful because they are casted to void* and cannot be casted back
 * correctly after that in all cases, especially when pointers are 
 * changed while casting. This happens when casting around in a type 
 * hierarchy.
 */

#ifndef CLEANUP_HANDLER_H
#define CLEANUP_HANDLER_H

#include <pthread.h>

namespace protlib { 

/** @addtogroup protlib
 * @{
 */

/// install cleanup handler
/** This macro installs a cleanup handler
 * and does some type casting.
 * Use uninstall_cleanup(execute) or an apropriate unlock routine to unlock
 * the mutex and uninstall the handler.
 * @param f pointer to a cleanup handler routine. 
 * This is casted to void (*routine)(void *)
 * @param m cleanup handler argument, normally a pointer to the mutex.
 * This is casted to void*.
 */
#define install_cleanup(f,m) pthread_cleanup_push((void (*)(void *)) f, (void *) m)

/// install cleanup handler for mutex
/** Calls install_cleanup and uses pthread_mutex_unlock as cleanup handler.
 * @param m pointer to the mutex.
 */
#define install_cleanup_mutex(m) install_cleanup(pthread_mutex_unlock,m)

/// lock mutex and install cleanup handler
/** @param m mutex
 */
#define install_cleanup_mutex_lock(m) install_cleanup_mutex(m) pthread_mutex_lock(m)

/// Lock thread and install cleanup handler
/** @param ttype class name of thread object
 * @param tp pointer to thread object
 */
#define install_cleanup_thread_lock(ttype,tp) install_cleanup(call_unlock<ttype>,tp) tp->lock()

/// uninstall cleanup handler
/** This uninstalls a cleanup handler and optionally executes it.
 * @param exec 0 or 1
 */
#define uninstall_cleanup(exec) pthread_cleanup_pop(exec)

/// unlock template
/** This function calls the unlock method of an object.
 * @param pobj pointer to the locked object.
 */
template <class T> void call_unlock(void* pobj) { 
	T* t;
	t = static_cast<T*>(pobj);
	t->unlock();
} // end call_unlock<T>

/// call void function
/** This function calls a function of type void f(void). */
inline void call_void_fun(void (*f)()) { 
	f();
} // end call_void_fun

//@}

} // end namespace protlib
#endif
