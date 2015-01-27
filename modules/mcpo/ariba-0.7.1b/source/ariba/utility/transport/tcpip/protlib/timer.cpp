/// ----------------------------------------*- mode: C++; -*--
/// @file timer.cpp
/// Software timer interface
/// ----------------------------------------------------------
/// $Id: timer.cpp 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/timer.cpp $
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
 * You can create a software timer and attach a callback object to it.
 * Timers are only accessed through their TimerManager and timer ID.
 * timer managers are thread-safe.
 *
 * Timers are stored in an ordered list to ease checking for elapsed timers.
 * Additionally, their IDs are kept in a hash_map, so a pointer to a
 * timer object can be obtained very fast.
 */

//#define DEBUG_TIMER

#include <errno.h>

#include "cleanuphandler.h"
#include "timer.h"
#include "logfile.h"

namespace protlib {

/** @addtogroup protlib
 * @{
 */

  using namespace log;

static inline 
void 
normalize_timespec(struct timespec& ts) 
{
  while (ts.tv_nsec>=1000000000) {
    ts.tv_sec++;
    ts.tv_nsec -= 1000000000;
  } // end while
  if (ts.tv_sec<0) ts.tv_sec = 0;
  if (ts.tv_nsec<0) ts.tv_nsec = 0;
} // end normalize

/**
 *  fills in timespec with sec+msec, msec may be larger than 999!
 */
static inline 
void 
fill_timespec(struct timespec& ts, int32 sec, int32 msec) 
{
  if (sec<0) sec = 0;
  if (msec<0) msec = 0;
  ts.tv_sec = sec + (msec/1000);
  ts.tv_nsec = (msec%1000)*1000000;
  normalize_timespec(ts);
} // end fill_timespec

static inline 
void 
add_timespecs(struct timespec& ts1, struct timespec& ts2, struct timespec& res) {
  res.tv_sec = ts1.tv_sec + ts2.tv_sec;
  res.tv_nsec = ts1.tv_nsec + ts2.tv_nsec;
  normalize_timespec(res);
} // end add_timespecs

static inline void gettimeofday_timespec(struct timespec& ts) {
	struct timeval now = {0,0};
	gettimeofday(&now,NULL);
	ts.tv_sec = now.tv_sec;
	ts.tv_nsec = now.tv_usec*1000;
} // end gettimeofday_timespec

/** Initialize a TimerManager object. */
TimerManager::TimerManager() 
{
  pthread_mutexattr_init(&mutex_attr);
#ifdef _DEBUG
  pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);
#else
  pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_NORMAL);
#endif

  pthread_mutex_init(&mutex,&mutex_attr);
  pthread_cond_init(&cond,NULL);

  Log(DEBUG_LOG,LOG_NORMAL, "Timer", "Creating TimerManager");
} // end constructor TimerManager

/** Destroy TimerManager. */
TimerManager::~TimerManager() {
#ifndef _NO_LOGGING
	int num = cleanup();
#else
	cleanup();
#endif
	DLog("Timer", "Destroying TimerManager with " << num << " timers");
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	pthread_mutexattr_destroy(&mutex_attr);
} // end destructor TimerManager

/** Start a timer relative to current time.
 * @return ID of the new timer.
 */
timer_id_t 
TimerManager::start_relative(TimerCallback* tc, int32 sec, int32 msec, timer_callback_param_t tcp) 
{
  struct timespec timeofday = {0,0};
  struct timespec reltime = {0,0};
  struct timespec alarm = {0,0};
  timer *t = NULL;
  timer_id_t result = 0;
  fill_timespec(reltime,sec,msec);
  // calculate absolute alarm time
  gettimeofday_timespec(timeofday);
  add_timespecs(timeofday,reltime,alarm);	
  // begin critical section
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  // start timer
  t = new(nothrow) timer(alarm,tc,tcp);
  if (t) {
    result = t->id;
    // insert into list
    insert_into_list(t);
    // insert into hash
    hashmap[result] = t;
    // wake up threads
    pthread_cond_signal(&cond);
  } else {
    Log(ERROR_LOG,LOG_CRIT, "Timer", "TimerManager::start_relative() is unable to create timer object");
  } // end if t
  // end critical section
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  return result;
} // end start_relative

/** Start an absolute timer.
 * @return ID of the new timer.
 */
timer_id_t 
TimerManager::start_absolute(TimerCallback* tc, int32 sec, int32 msec, timer_callback_param_t tcp) {
  struct timespec alarm = {0,0};
  timer *t = NULL;
  timer_id_t result = 0;
  fill_timespec(alarm,sec,msec);
  // begin critical section
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  // start timer
  t = new(nothrow) timer(alarm,tc,tcp);
  if (t) {
    result = t->id;
    // insert into list
    insert_into_list(t);
    // insert into hash
    hashmap[result] = t;
    // wake up threads
    pthread_cond_signal(&cond);
  } else {
    Log(ERROR_LOG,LOG_CRIT, "Timer", "TimerManager::start_absolute() is unable to create timer object");
  } // end if t
  // end critical section
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  return result;
} // end start_absolute

/** Restart a timer and set the alarm relative to the current time.
 * The timer must exist and go off in the future.
 * @return true on success, false otherwise.
 */
bool 
TimerManager::restart_relative(timer_id_t id, int32 sec, int32 msec) 
{
  struct timespec timeofday = {0,0};
  struct timespec reltime = {0,0};
  struct timespec alarm = {0,0};
  timer *t = NULL;
  bool result = false;
  timer_hashmap_it_t hit;
  fill_timespec(reltime,sec,msec);
  // calculate absolute alarm time
  gettimeofday_timespec(timeofday);
  add_timespecs(timeofday,reltime,alarm);	
  // begin critical section
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  // try to get timer with given ID
  if ((hit=hashmap.find(id))!=hashmap.end()) t = hit->second; else t = NULL;
  if (t) {
    // delete from list, set new alarm and insert
    delete_from_list(t);
    t->time = alarm;
    insert_into_list(t);
    // wake up threads
    pthread_cond_signal(&cond);
    result = true;
  } else {
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::restart_relative() is unable to restart timer " << id);
  } // end if t
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  return result;
} // end restart_relative


/** Restart a timer and set the alarm to an absolute time.
 * The timer must exist and go off in the future.
 * @return true on success, false otherwise.
 */
bool 
TimerManager::restart_absolute(timer_id_t id, int32 sec, int32 msec) 
{
  struct timespec alarm = {0,0};
  timer *t = NULL;
  bool result = false;
  timer_hashmap_it_t hit;
  fill_timespec(alarm,sec,msec);
  // begin critical section
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  // try to get timer with given ID
  if ((hit=hashmap.find(id))!=hashmap.end()) t = hit->second; else t = NULL;
  if (t) {
    // delete from list, set new alarm and insert
    delete_from_list(t);
    t->time = alarm;
    insert_into_list(t);
    // wake up threads
    pthread_cond_signal(&cond);
    result = true;
  } else {
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::restart_relative() is unable to restart timer " << id);
  } // end if t
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  return result;
} // end restart_absolute

/** Stop the given timer. */
bool TimerManager::stop(timer_id_t id) 
{
  timer *t = NULL;
  bool result = false;
  timer_hashmap_it_t hit;
  // begin critical section
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  if ((hit=hashmap.find(id))!=hashmap.end()) {
    t = hit->second;
    // erase from hashmap
    hashmap.erase(hit);
  } else t = NULL;
  // delete from list if t exists
  if (t) {
    delete_from_list(t);
    // wake up threads
    pthread_cond_signal(&cond);
    result = true;
  } else {
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::stop() is unable to stop timer " << id);
  } // end if t
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  return result;
} // end stop

/** Check if timers have elapsed and call their callbacks.
 * @return the number of elapsed timers. 
 */
uint32 
TimerManager::check_timers() 
{
  timer *elapsed;
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  elapsed = collect_elapsed();
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	return process_elapsed();
} // end check_timers

/** Like check_timers() but waits msec milliseconds for a timer to elapse. */
uint32 
TimerManager::check_timers_wait(int32 msec) 
{
  struct timespec now = {0,0};
  struct timespec reltime = {0,0};
  struct timespec abstime = {0,0};
  timer* elapsed;
  int wait_res = 0;
  fill_timespec(reltime,0,msec);
  gettimeofday_timespec(now);
  // calculate timespec for pthread_cond_timedwait()
  add_timespecs(now,reltime,abstime);
  timer maxwait(abstime,NULL,NULL,false);
  // begin critical section
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  // look for elapsed timers until timeout
  elapsed = collect_elapsed();
  while ((!elapsed) && (wait_res!=ETIMEDOUT)) {
    // neither timeout nor elapsed timers
    // if there is a timer in the list, wait until it elapses.
    // otherwise wait abstime.
    if (first() && ((*first())<=maxwait)) abstime = first()->time;
    else abstime = maxwait.time;
    // wait for condition or timeout
    wait_res = pthread_cond_timedwait(&cond,&mutex,&abstime);
#ifdef DEBUG_TIMER
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::check_timers_wait() - timedwait returned " << wait_res << ":" << strerror(wait_res));
#endif
    elapsed = collect_elapsed();
#ifdef DEBUG_TIMER
    if (elapsed)
      Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::check_timers_wait() - collect_elapsed() returned " <<  elapsed->id);
#endif
  } // end while !elapsed and !wait_res
  // end critical section
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  // now either timeout, cancellation or elapsed timers
  return process_elapsed();
} // end check_timers_wait

/** Stop all timers. */
uint32 
TimerManager::stop_all() 
{
  uint32 result = 0;
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  result = cleanup();
  if (result) {
    // wake up threads
    pthread_cond_signal(&cond);
  } // end if result
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
  return result;
} // end stop_all_timers

/** Stop all timers without locking the mutex.
 * So this can be called safely inside the TimerManager constructor.
 */
uint32 
TimerManager::cleanup() 
{
  uint32 num = 0;
  // clear hashmap
  hashmap.clear();
  
  // delete all timers
  timer *curr= 0;
  while (!active_timerlist.empty())
  {
    if ( (curr= active_timerlist.front()) != 0 )
    {
      delete curr;
      active_timerlist.pop_front();
      num++;
    }
  } // end while
  return num;
} // end cleanup

/** Insert a timer object into the timer list.
 * Timers are stored in an ordered list, so when checking for elapsed timers
 * it's enough to check timers beginning at the front end until one is
 * still running.
 */
inline 
void 
TimerManager::insert_into_list(timer *t) 
{
  timerlist_t::iterator listiter= active_timerlist.begin();
  while(listiter != active_timerlist.end())
  {
    if ( *listiter ) 
    {
      // if current element is greater than *t, leave the loop
      if (!( *(*listiter) <= *t ))
	break;
      
    }
    listiter++;
  } // end for
  active_timerlist.insert(listiter,t);


#ifdef DEBUG_TIMER
	Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::insert_into_list() - inserting timer " << t->id << " list now:");
#endif

#ifdef DEBUG_TIMER
  for(timerlist_t::iterator listpiter= active_timerlist.begin();
      listpiter != active_timerlist.end();
      listpiter++)
  {
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::insert_into_list() - timer list, timer#: " << (*listpiter ? (*listpiter)->id : 0));
  }
#endif

} // end insert_into_list

/** Delete a timer from the ordered timer list. 
 * The timer object is NOT freed because it may be needed for executing its
 * callback or for restarting.
 * The timer objects must be linked correctly, no checking is done.
 */
void 
TimerManager::delete_from_list(timer *t) {

  if (!t) return;

#ifdef DEBUG_TIMER
  Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::delete_from_list() - deleting timer " << t->id);
#endif
  timerlist_t::iterator listiter= find(active_timerlist.begin(),active_timerlist.end(),t);
  if ( listiter != active_timerlist.end() )
      active_timerlist.erase(listiter);
#ifdef DEBUG_TIMER
  if (first()) 
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::delete_from_list() - first timer now " << (first() ? first()->id : 0) );
#endif
} // end delete_from_list

/** Collect all elapsed timers in the elapsed_timerlist and delete them already
 * from the hashmap. The timers are deleted in process_elapsed().
 * You must lock the TimerManager mutex before collecting timers.
 * When timers have elapsed, the condition is signaled.
 */
TimerManager::timer* 
TimerManager::collect_elapsed() 
{	
  struct timespec tod;
  gettimeofday_timespec(tod);
  timer now(tod,NULL,NULL,false);

  timerlist_t::iterator currentit = active_timerlist.begin();
  timer* curr= first();
#ifdef DEBUG_TIMER
  if (curr)
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::collect_elapsed() - first timer is " << curr->id);
#endif

  // search the last timer in list <= now
  while (curr && ((*curr)<=now)) 
  { // current timer is already elapsed
    hashmap.erase(curr->id);
#ifdef DEBUG_TIMER
    Log(DEBUG_LOG,LOG_NORMAL, "Timer", "TimerManager::collect_elapsed() - deleted elapsed timer " << curr->id);
#endif
    // remember it in elapsed list
    elapsed_timerlist.push_back(curr);
    // delete it from the active timer list
    currentit= active_timerlist.erase(currentit);

    curr= (currentit != active_timerlist.end()) ? *currentit : 0;
  } // end while
  // List is ordered, so all timers before currentit are elapsed.

  timer* elapsed = elapsed_timerlist.empty() ? 0 : elapsed_timerlist.front();

  // wake up threads
  if (elapsed) pthread_cond_signal(&cond);
  
  return elapsed;
} // end collect_elapsed

/** Process a list of timers by executing the callback routines and deleting
 * the timer objects. They are NOT deleted from the TimerManager hashmap.
 * The mutex should not be locked when callbacks are executed, because 
 * it is unpossible to start a new timer inside a callback routine when
 * the mutex is still locked.
 * @see collect_elapsed()
 */
uint32 TimerManager::process_elapsed() 
{
  uint32 num = 0;
  timer *tmp = NULL;

  // for every elapsed timer (list should be prepared by collect_elapsed) do_callback()
  for (timerlist_t::iterator elapsed_it= elapsed_timerlist.begin();
       elapsed_it != elapsed_timerlist.end();
       elapsed_it= elapsed_timerlist.erase(elapsed_it))
  {
    // invoke callback function on elapsed timer
    if ( (tmp = *elapsed_it) != 0)
    {
      tmp->do_callback();
      // when callback is finished, delete the stuff
      delete tmp;
      num++;
    }
    // get next elapsed timer
  } // end for
  return num;
} // end process_elapsed

/** Initialize a timer.
 * If get_id is set (default) the timer gets a unique ID, otherwise the ID
 * of the timer is 0.
 */
TimerManager::timer::timer(struct timespec& ts, TimerCallback* tc, timer_callback_param_t tcp, bool get_id) :
  id (0),
  time(ts),
  callback(tc),
  param(tcp)
{
  if (get_id) while (id==0) id = next_id++;
} // end constructor timer

/** This holds the timer ID of the next timer. */
timer_id_t TimerManager::timer::next_id = 1;

/** Check which timer goes off eralier. 
 * The timespec structores of the timers are expected to have correct
 * format.
 */
inline 
bool 
TimerManager::timer::operator<=(const timer& t) {
  if (time.tv_sec == t.time.tv_sec) {
    // Seconds are equal, it depends on nanoseconds.
    return (time.tv_nsec <= t.time.tv_nsec);
  } else {
    // Seconds are not equal, nanoseconds do not matter.
    return (time.tv_sec < t.time.tv_sec);
  }
} // end operator<=

inline 
void 
TimerManager::timer::do_callback() {
  callback->timer_expired(id,param);
} // end do_callback

//@}

} // end namespace protlib
