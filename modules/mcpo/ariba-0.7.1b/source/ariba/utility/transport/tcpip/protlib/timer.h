/// ----------------------------------------*- mode: C++; -*--
/// @file timer.h
/// Basic TimerManager class
/// ----------------------------------------------------------
/// $Id: timer.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/timer.h $
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
// timer.h - Software Timer interface
// ----------------------------------------------------------
// $Id: timer.h 2549 2007-04-02 22:17:37Z bless $
// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/timer.h $
// ==========================================================
//                      
// Institute of Telematics, University of Karlsruhe (TH)
// ==========================================================
/** @ingroup protlib
 * @file
 * Software Timer interface
 *
 * You can create a software timer and attach a callback object to it.
 * Timers are only accessed through their timer manager and thtimer ID.
 * timer managers are thread-safe.
 */

#ifndef _PROTLIB__TIMER_H_
#define _PROTLIB__TIMER_H_

#include <pthread.h>
#include <sys/time.h>
#include <list>
#include <boost/unordered_map.hpp>

#include "protlib_types.h"
#include "llhashers.h"

namespace protlib {

/** @addtogroup protlib
 * @{
 */

/// timer ID
/** Each timer of a timer manager has a unique timer ID. */
typedef uint64 timer_id_t;

/// timer callback parameter
typedef void* timer_callback_param_t;

/// Timer Callback
/** When a timer goes off, a callback is called. Each class that wants to
 * receive timer events must inherit from TimerCallback. 
 */
class TimerCallback {
public:
	virtual ~TimerCallback() { }

	/// callback member function
   /** @param id timer ID
    * @param param additional callback parameter.
	 */
	virtual void timer_expired(timer_id_t id, timer_callback_param_t param) = 0;
};

/// Timer Manager
/** Creates, sets, resets, stops, checks and deletes timers. */
class TimerManager {
public:
	/// constructor
	TimerManager();
	/// destructor
        ~TimerManager();
	/// start relative timer
	timer_id_t start_relative(TimerCallback* tc, int32 sec, int32 msec = 0, timer_callback_param_t tcp = NULL);
	/// start absolute timer
	timer_id_t start_absolute(TimerCallback* tc, int32 sec, int32 msec = 0, timer_callback_param_t tcp = NULL);
	/// restart relative timer
	bool restart_relative(timer_id_t id, int32 sec, int32 msec = 0);
	/// restart absolute timer
	bool restart_absolute(timer_id_t id, int32 sec, int32 msec = 0);
	/// stop timer
	bool stop(timer_id_t id);
	/// check for elapsed timers
	uint32 check_timers();
	/// check for elapsed timers or wait for next timer event.
	uint32 check_timers_wait(int32 msec);
	/// stop all timers
	uint32 stop_all();
private:
	/// timer item
	struct timer {
		timer_id_t id;
		struct timespec time;
		TimerCallback* callback;
		timer_callback_param_t param;
		timer(struct timespec& ts, TimerCallback* tc, timer_callback_param_t tcp, bool get_id = true);
		// compare two timers
		bool operator<=(const timer& t);
		/// execute callback
		void do_callback();
		/// timer ID counter
		static timer_id_t next_id;
	}; // end struct timer
	/// timer manager hashmap
	typedef boost::unordered_map<timer_id_t,timer*> timer_hashmap_t;
	/// hashmap iterator
	typedef timer_hashmap_t::iterator timer_hashmap_it_t;
        /// timer list type
        typedef list<timer*> timerlist_t;

	/// cleanup hashmap and list
	uint32 cleanup();
	/// insert into list
	void insert_into_list(timer* t);
	/// delete timer from list
	void delete_from_list(timer *t);
	/// collect elapsed timers
	timer* collect_elapsed();
	/// process elapsed timers
	uint32 process_elapsed();
	/// timer manager mutex
	pthread_mutex_t mutex;
	/// timer manager mutex attributes
        pthread_mutexattr_t mutex_attr;

	/// timer manager condition
	pthread_cond_t cond;	
        /// sorted timer list (should be replaced by a heap for performance reasons...)
        timerlist_t active_timerlist;
        /// elapsed timer list
        timerlist_t elapsed_timerlist;
	/// return first element of active timer list 
        timer* first() { return active_timerlist.empty() ? 0 : active_timerlist.front(); }
	timer_hashmap_t hashmap;
};

//@}

} // end namespace protlib

#endif // _PROTLIB__TIMER_H_
