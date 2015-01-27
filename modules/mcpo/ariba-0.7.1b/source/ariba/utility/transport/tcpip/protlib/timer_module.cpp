/// ----------------------------------------*- mode: C++; -*--
/// @file timer_module.cpp
/// timer module that maintains timers and triggers messages
/// ----------------------------------------------------------
/// $Id: timer_module.cpp 2756 2007-08-06 12:51:39Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/timer_module.cpp $
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
 * The timer module provides a way for other modules to set timers via a
 * message and receive a message back on their queue when the timer goes
 * off.
 */

#include <sstream>

#include "timer_module.h"
#include "queuemanager.h"
#include "logfile.h"
#include "cleanuphandler.h"

namespace protlib {

/** @addtogroup protlib
 * @{
 */

  using namespace log;

/***** class TimerMsg *****/

TimerMsg::TimerMsg(qaddr_t s, bool s_err)
  : message(type_timer,s),
    time_sec(0),
    time_msec(0),
    action(ac_ignore),
    param1(NULL),
    param2(NULL),
    ok(true),
    send_error(s_err),
    relative(true)
{
  // Log(DEBUG_LOG,LOG_NORMAL,"TimerMsg","TimerMsg constructor. msgid:" << get_id());
} // end TimerMsg

/** Destructor, does nothing. */
TimerMsg::~TimerMsg() {}


/** Set result flag and get old value.
 * You should not set this, this is done by the timer module.
 */
bool TimerMsg::set_ok(bool r) {
	register bool old = ok;
	ok = r;
	return old;
} // end set_ok

/** Prepare message to start an absolute timer. */
bool TimerMsg::start_absolute(int32 sec, int32 msec, param_t p1, param_t p2) {
	return start(false,sec,msec,p1,p2);
} // end start_absolute

/** Prepare message to start a relative timer. */
bool TimerMsg::start_relative(int32 sec, int32 msec, param_t p1, param_t p2) {
	return start(true,sec,msec,p1,p2);
} // end start_relative

bool TimerMsg::start(bool rel, int32 sec, int32 msec, param_t p1, param_t p2) {
	time_sec = (sec<=0)?0:sec;
	time_msec = (msec<=0)?0:msec;
	param1 = p1;
	param2 = p2;
	action = ac_start;
	relative = rel;
	ok = true;
	return true;
} // end start

/** Restart an absolute timer. */
bool TimerMsg::restart_absolute(id_t id, int32 sec, int32 msec) {
	return restart(false,id,sec,msec);
} // end restart_absolute

/** Restart a relative timer. */
bool TimerMsg::restart_relative(id_t id, int32 sec, int32 msec) {
	return restart(true,id,sec,msec);
} // end restart_relative

/** restart timer 
 * please note that this method actually overwrites the id of the message(!)
 */
bool TimerMsg::restart(bool rel, id_t id, int32 sec, int32 msec) {
	relative = rel;
	if (id && set_id(id)) {
		time_sec = (sec<=0)?0:sec;
		time_msec = (msec<=0)?0:msec;
		action = ac_restart;
		ok = true;
	} else {
		time_sec = 0;
		time_msec = 0;
		action = ac_ignore;
		ok = false;
	} // end if id
	return ok;
} // end restart

/** Stop a timer. */
bool TimerMsg::stop(id_t id) {
	relative = false;
	time_sec = 0;
	time_msec = 0;
	param1 = param2 = NULL;
	if (id && set_id(id)) {
		action = ac_stop;
		ok = true;
	} else {
		action = ac_ignore;
		ok = false;
	} // end if id
	return ok;
} // end stop

/** Stop all running timers. */
bool TimerMsg::stop_all() {
	relative = false,
	time_sec = 0;
	time_msec = 0;
	param1 = param2 = NULL;
	action = ac_stop_all;
	ok = true;
	return true;
} // end stop_all

/** Prepare message for sending it back as a reply message from the
 * TimerModule when its timer expires.
 */
bool TimerMsg::set_elapsed() {
	send_error = false;
	action = ac_elapsed;
	ok = true;
	return ok;
} // end set_elapsed

bool TimerMsg::get_send_error() const {
	return send_error;
} // end get_send_error

/** Set send_error flag and return old value. */
bool TimerMsg::set_send_error(bool f) {
	register bool o = send_error;
	send_error = f;
	return o;
} // end set_send_error

TimerMsg::param_t TimerMsg::get_param1() const { return param1; }

TimerMsg::param_t TimerMsg::get_param2() const { return param2; }

void TimerMsg::get_params(param_t& p1, param_t& p2) const {
	p1 = param1;
	p2 = param2;
} // end get_params

bool TimerMsg::is_absolute() const { return (!relative); }

bool TimerMsg::is_relative() const { return relative; }

/***** struct TimerModuleParam *****/

/** @param sleep_time default sleep time
 * @param sua send messages until aborted or just until stopped
 * @param see send error messages as expedited data
 * @param sre send reply messages as expedited data
 */
TimerModuleParam::TimerModuleParam(uint32 sleep_time, bool sua, bool see, bool sre)
	: ThreadParam(sleep_time, "TimerModule", 2), send_until_abort(sua),
	source(message::qaddr_timer), 
	send_error_expedited(see), send_reply_expedited(sre) {
	// nothing more to do
} // end constructor TimerModuleParam

/***** class TimerModule *****/

/** Set parameters. */
TimerModule::TimerModule(const TimerModuleParam& p) 
	: Thread(p), timerparam(p) {
	tmap.clear();
	// register queue
	QueueManager::instance()->register_queue(get_fqueue(),p.source);
	DLog(timerparam.name, "Creating TimerModule object");
} // end constructor TimerModule

/** Stop all running timers. */
TimerModule::~TimerModule() {
	stop_all_timers();
	DLog(timerparam.name, "Destroying TimerModule object");
	QueueManager::instance()->unregister_queue(timerparam.source);

} // end destructor TimerModule

/** Devide Threads in thos which process the queue and those which process
 * expired timers.
 */
void TimerModule::main_loop(uint32 nr) {
  Log(INFO_LOG,LOG_NORMAL, timerparam.name, "Starting " << timerparam.name << " thread #" << nr << ", " << ((nr%2) ? "processing input queue" : "processing timer callbacks"));

  if (nr%2) process_queue();
  else process_elapsed_timers();

  Log(INFO_LOG,LOG_NORMAL, timerparam.name,"Thread #" << nr << " stopped");
} // end main_loop

/** Wait for incomming mesages and evaluate message action.
 * Messages are accepted until the module is asked to stop.
 *
 * The module mutex inherited from the Thread base class is locked here, so all
 * called member functions are called inside a critical section if necessary.
 */
void TimerModule::process_queue() {
  uint32 wait = timerparam.sleep_time*1000;
  message* msg = NULL;
  TimerMsg* tmsg = NULL;
  FastQueue* fq = QueueManager::instance()->get_queue(message::qaddr_timer);
  bool opresult = false;
  if (!fq) {
    Log(ERROR_LOG,LOG_ALERT, timerparam.name," cannot find input queue");
    return;
  } // end if not fq
  // wait for messages
  while (get_state()==STATE_RUN) {
    msg = fq->dequeue_timedwait(wait);
    if (msg) {
      if (msg->get_type()==message::type_timer) {
	tmsg = dynamic_cast<TimerMsg*>(msg);
	if (tmsg) {
	  // begin critical section
	  lock(); // install_cleanup_thread_lock(TimerModule,this);
	  if (tmsg->is_ok()) {
	    // evaluate action
	    switch (tmsg->get_action()) {
	      case TimerMsg::ac_ignore:
		      // do nothing
		Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"received message with action set to ignore");
		opresult = true;
		break;
	      case TimerMsg::ac_start:
		opresult = start_timer(tmsg);
		break;
	      case TimerMsg::ac_restart:
		opresult = restart_timer(tmsg);
		break;
	      case TimerMsg::ac_stop:
		opresult = stop_timer(tmsg);
		break;
	      case TimerMsg::ac_stop_all:
		opresult = stop_all_timers();
		break;
	      default:
		ERRLog(timerparam.name, "Wrong action " << tmsg->get_action() << " in message from " << tmsg->get_qaddr_name() << " to " << message::get_qaddr_name(timerparam.source) );
		
		opresult = false;
	    } // end switch get_action
	  } else {
	      Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"received message in invalid state, mid " << tmsg->get_id() );
	    opresult = false;
	  } // if tmsg->is_ok()
	  // error handling, message disposing
	  send_error_or_dispose(tmsg,opresult);
	  // end critical section
	  unlock(); // uninstall_cleanup(1);
	} else {
	  Log(ERROR_LOG,LOG_ALERT, timerparam.name, "Cannot cast message from " << msg->get_qaddr_name() << " of type " << msg->get_type_name() << " to TimerMsg");
	  delete msg;
	} // end if tmsg
      } else {
	ERRLog(timerparam.name,"received message that is not of type_timer from " << msg->get_qaddr_name() << ", type was " << msg->get_type_name());
	delete msg;
      } // end if type
    } // end if msg
  } // end while running
} // end process_queue

/** Check if timers expired and send reply messages.
 * Reply messages are sent until the module is asked to abort if
 * the flag send_until_abort is set true. Otherwise no messages are sent
 * after a stop request.
 */
void TimerModule::process_elapsed_timers() {
	state_t end_state;
	uint32 num = 0;
	uint32 sleeptime = timerparam.sleep_time*1000;
	if (timerparam.send_until_abort) end_state = STATE_ABORT;
	else end_state = STATE_STOP;
	while(get_state()!=end_state) {
		num = tm.check_timers_wait(sleeptime);
		if (num) {
		  Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"got " << num << " elapsed timer(s)");
		} // end if num
	} // end while state
} // end process_elapsed_timers

/** Starts a timer and stores its ID and the reply message in the hash maps. */
bool TimerModule::start_timer(TimerMsg* m) {
	timer_id_t tid = 0;
	bool res = true;
	message::id_t mid = m->get_id();
	bool relative = m->is_relative();
	int32 sec = 0;
	int32 msec = 0;
	if (mid) {
	  // lookup timer ID in map
	  if ((tid=tmap.lookup_tid(mid))) {
	    ERRLog(timerparam.name, m->get_qaddr_name() << " tried to start a timer with mid " << mid << ", but there is already a timer " << tid);
	    res = false;
	  } else {
	    // start timer
	    m->get_time(sec,msec);
	    if (relative) tid = tm.start_relative(this,sec,msec,NULL);
	    else tid = tm.start_absolute(this,sec,msec,NULL);
	    if (tid) {
	      // insert in map
	      tmap.insert(tid,m);
	      // timer successfully started
	      Log(EVENT_LOG,LOG_UNIMP, timerparam.name, "Timer " << tid << " (" << sec << "s " << msec << "ms) started for " 
		                                         << m->get_qaddr_name() << " with mid " << mid);
	      res = true;
	    } else {
	      // timer not started
	      ERRLog(timerparam.name, "TimerManager in " << timerparam.name << " is unable to start a timer for " << m->get_qaddr_name());
	      res = false;
	    } // end if tid
	  } // end if lookup repmsg
	} else {
	  ERRLog(timerparam.name, m->get_qaddr_name() << " tried to start a timer with message ID 0");
	  res = false;
	} // end if repmsg
	return res;
} // end start_timer

/** Restarts a timer and stores its ID and the reply message in the hash maps. */
bool TimerModule::restart_timer(TimerMsg* m) {
	timer_id_t tid = 0;
	bool res = true;
	message::id_t mid = m->get_id();
	bool relative = m->is_relative();
	TimerMsg* repmsg = NULL;
	int32 sec = 0;
	int32 msec = 0;
	if (mid) {
	  // lookup timer_id and reply message for mid in map
	  tid = tmap.lookup_tid(mid);
	  repmsg = tmap.lookup_msg(tid);
	  if (tid && repmsg) {
	    // restart timer
	    m->get_time(sec,msec);
	    if (relative) res = tm.restart_relative(tid,sec,msec);
	    else res = tm.restart_absolute(tid,sec,msec);
	    if (res) {
	      // modify reply message
	      repmsg->restart(relative,mid,sec,msec);
	      // timer successfully restarted
	      DLog(timerparam.name, "Timer " << tid << ", mid " << mid << " restarted for " << m->get_qaddr_name());
	    } else {
	      // timer not restarted
	      ERRLog(timerparam.name, "TimerManager in " << timerparam.name << " is unable to restart a timer for " << m->get_qaddr_name() << ", mid " << mid);
	    } // end if res
	  } else {
	    if (tid) ERRLog(timerparam.name, m->get_qaddr_name() << " tried to restart a timer with mid " << mid << ": or no reply message for timer found");
	    if (repmsg)  ERRLog(timerparam.name, m->get_qaddr_name() << " tried to restart timer with mid " << mid << ": timer not found");
	    if ((!repmsg) & (!tid)) ERRLog(timerparam.name, m->get_qaddr_name() << " tried to restart timer with mid " << mid << ": neither timer nor reply message found");
	    res = false;
	  } // end if tid
	} else {
	  ERRLog(timerparam.name, m->get_qaddr_name() << " tried to restart a timer with an invalid message ID");
	  res = false;
	} // end if repmsg
	return res;
} // end restart_timer

/** Stop a timer and remove its ID and reply message from the hash maps. */
bool TimerModule::stop_timer(TimerMsg* m) {
	timer_id_t tid = 0;
	bool res = true;
	message::id_t mid = m->get_id();
	if (mid) {
		// lookup timer_id for mid in map
		tid = tmap.lookup_tid(mid);
		if (tid) {
			// stop timer
			res = tm.stop(tid);
			if (res) {
				// delete from map
				tmap.erase(tid,mid,true);
				// timer stopped
				DLog(timerparam.name, "Stopped timer " << tid << ", mid " << mid << " for " << m->get_qaddr_name());
			} else {
				// timer not stopped
			  ERRLog(timerparam.name, "TimerManager in " << timerparam.name << " is unable to stop timer " << tid << ", mid " << mid << " for " << m->get_qaddr_name());
			} // end if tid
		} else {
		  ERRLog(timerparam.name, m->get_qaddr_name() << " tried to stop a non-existing timer with mid " << mid);
		  res = false;
		} // end if tid
	} else {
	  ERRLog(timerparam.name, m->get_qaddr_name() << " tried to stop a timer with an invalid message ID");
	  res = false;
	} // end if repmsg
	return res;
} // end stop_timer

/** Stop all timers and clear the hash maps. */
bool TimerModule::stop_all_timers() {
	uint32 num = 0;
	// clear map
	tmap.clear(true);
	// clear TimerManager
	num = tm.stop_all();
	Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"stopped all timers, num " << num);
	return true;
} // end stop_all_timers

/** Send back error message.
 * If no error is necessary, the message is disposed.
 * @param m message that is changed to an error message if 
 *		its flags are set appropriately.
 * @param ok success or error?
 */
void TimerModule::send_error_or_dispose(TimerMsg* m, bool ok) {
	message::qaddr_t dest;
	if (!m) return;

#ifndef _NO_LOGGING
	message::id_t mid = m->get_id();	// only used for logging
#endif

	// Do we send errors?
	if ((!ok) && m->get_send_error()) ok = false;
	else ok = true;
	if (ok) {
		// dispose if not a start message
		if (m->get_action()!=TimerMsg::ac_start) {
			dest = m->get_source();
			delete m;
			//Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"disposed message " << mid << " from " << message::get_qaddr_name(dest));
		} // end if dispose message
	} else {
		// send error
		dest = m->get_source();
		m->set_ok(false);
		if (m->send(timerparam.source,dest,timerparam.send_error_expedited)) {
		  Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"sent error message w/ mid " << mid << " to " << message::get_qaddr_name(dest));
		} else {
		  ERRLog(timerparam.name,"cannot send error message w/ mid " << mid << " to " << message::get_qaddr_name(dest) << ", disposing it now");
		  delete m;
		} // end if sent error
	} // end if ok
} // end send_error_or_dispose

/** This is the callback for the TimerManager used by the TimerModule object.
 * @param timer timer ID
 * @param callback_param additional callback parameter.
 *
 * The module mutex inherited from the Thread base class is locked here, so all
 * called member functions are called inside a critical section if necessary.
 */
void TimerModule::timer_expired(timer_id_t timer, timer_callback_param_t callback_param) {
	TimerMsg* msg = NULL;
	message::qaddr_t dest;
	message::id_t mid = 0;
	// begin critical section
	lock(); // install_cleanup_thread_lock(TimerModule,this);
	// get reply message
	msg = tmap.lookup_msg(timer);
	if (msg) {
		// store message ID for erasing this record from the map
		mid = msg->get_id();
		// send message
		dest = msg->get_source();
		if (msg->set_elapsed() && msg->send_back(timerparam.source,timerparam.send_reply_expedited)) {
		  Log(DEBUG_LOG,LOG_UNIMP, timerparam.name,"sent reply mid " << mid << " for elapsed timer " << timer << " to " << message::get_qaddr_name(dest));
		} else {
		  ERRLog(timerparam.name,"cannot send reply mid " << mid << " for elapsed timer " << timer <<" to " << message::get_qaddr_name(dest));
		  // dispose message
		  delete msg;
		} // end if send_back
	} else {
		// may be timer has been stopped
	  DLog(timerparam.name, "TimerModule::timer_expired cannot find reply message for a timer " << timer << ". Maybe the timer has been stopped");
	} // end if msg
	// erase (timer,mid) without disposing the reply message
	tmap.erase(timer,mid,false);
	// end critical section
	unlock(); // uninstall_cleanup(1);
} // end timer_expired

/***** class TimerModule::TimerMap *****/

bool TimerModule::TimerMap::insert(timer_id_t tid, TimerMsg* m) {
	if (tid && m) {
		message::id_t mid = m->get_id();
		tid2mid[tid] = mid;
		mid2tid[mid] = tid;
		tid2msg[tid] = m;
		return true;
	} else return false;
} // end insert

/** Returns the message ID of the timer bound to timer ID or 0 if not
 * found, since every timer must have an ID <> 0.
 */
message::id_t TimerModule::TimerMap::lookup_mid(timer_id_t tid) const {
	const_tid2mid_it_t hit;
	hit = tid2mid.find(tid);
	if (hit!=tid2mid.end()) return hit->second;
	else return 0;
} // end lookup

/** Returns the timer-ID of the timer bound to the message ID or 0 if not
 * found, since 0 is never used as a timer ID.
 */
timer_id_t TimerModule::TimerMap::lookup_tid(message::id_t mid) const {
	const_mid2tid_it_t hit;
	if ((hit=mid2tid.find(mid))!=mid2tid.end()) return hit->second;
	else return 0;
} // end lookup

/** Returns the timer message of the timer bound to timer ID or NULL if not
 * found.
 */
TimerMsg* TimerModule::TimerMap::lookup_msg(timer_id_t tid) const {
	const_tid2msg_it_t hit;
	hit = tid2msg.find(tid);
	if (hit!=tid2msg.end()) return hit->second;
	else return NULL;
} // end lookup_msg

/** Please be sure that timer ID and message ID are bound to the same timer. 
 * No checking is done!
 */
void TimerModule::TimerMap::erase(timer_id_t tid, message::id_t mid, bool dispose) {
	TimerMsg* m = NULL;
	if (tid) {
		if (dispose) {
			m = lookup_msg(tid);
			if (m) delete m;
		} // end if dispose
		tid2mid.erase(tid);
		tid2msg.erase(tid);
	} // end if tid
	if (mid) mid2tid.erase(mid);
} // end erase

void TimerModule::TimerMap::clear(bool dispose) {
	const_tid2msg_it_t hit;
	if (dispose) {
		for (hit=tid2msg.begin();hit!=tid2msg.end();hit++) {
			if (hit->second) delete hit->second;
		} // end for hit
	} // end if dispose
	tid2mid.clear();
	tid2msg.clear();
	mid2tid.clear();
} // end clear

//@}

} // end namespace protlib
