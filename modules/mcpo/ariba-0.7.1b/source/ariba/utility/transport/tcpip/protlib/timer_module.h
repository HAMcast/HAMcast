/// ----------------------------------------*- mode: C++; -*--
/// @file timer_module.h
/// Timer module (thread) that provides timer management functions
/// ----------------------------------------------------------
/// $Id: timer_module.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/timer_module.h $
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

#ifndef _PROTLIB__TIMER_MODULE_H
#define _PROTLIB__TIMER_MODULE_H

#include <boost/unordered_map.hpp>

#include "messages.h"
#include "threads.h"
#include "llhashers.h"
#include "timer.h"

using boost::unordered_map;

namespace protlib {

/** @addtogroup protlib
 * @{
 */

  static const char *const action_t_str[]=
    {
      "ignore",
      "start",
      "restart",
      "stop",
      "stop all",
      "elapsed",
      "unkown"
    };

/// timer message
/** This message class is used to control timers managed by the timer
 * module and for replies when a timer goes off.
 * Timers are identified by the message ID.
 */
class TimerMsg : public message 
{
public:
  /// timer module actions
  /** Tell the timer module what to do or indicate that a timer elapsed. */
  enum action_t 
    {
      ac_ignore   = 0,
      ac_start    = 1,
      ac_restart  = 2,
      ac_stop     = 3,
      ac_stop_all = 4,
      ac_elapsed  = 5
    }; // end action_t

  /// timer parameter
  /** This parameter corresponds to the timer parameters in timer.h and
   * helps managing timers.
   */
  typedef timer_callback_param_t param_t;
  /// constructor
  TimerMsg(qaddr_t s = qaddr_unknown, bool s_err = false);
  /// destructor
  virtual ~TimerMsg();
  /// get alarm
  void get_time(int32& sec, int32& msec);
  /// get action
  action_t get_action() const;
  /// get action str
  const char* get_action_str() const;
  /// set result
  bool set_ok(bool r);
  /// get result
  bool is_ok();
  /// start absolute timer
  bool start_absolute(int32 sec, int32 msec = 0, param_t p1 = NULL, param_t p2 = NULL);
  /// start relative timer
  bool start_relative(int32 sec, int32 msec = 0, param_t p1 = NULL, param_t p2 = NULL);
  /// start timer
  bool start(bool rel, int32 sec, int32 msec = 0, param_t p1 = NULL, param_t p2 = NULL);
  /// restart absolute timer
  bool restart_absolute(id_t id, int32 sec, int32 msec = 0);
  /// restart relative timer
  bool restart_relative(id_t id, int32 sec, int32 msec = 0);
  /// restart timer
  bool restart(bool rel, id_t id, int32 sec, int32 msec = 0);
  /// stop timer
  bool stop(id_t id);
  /// stop all timers
  bool stop_all();
  /// set to elapsed
  bool set_elapsed();
  /// get send_error flag
  bool get_send_error() const;
  /// set send_error flag
  bool set_send_error(bool f);
  /// get timer parameter #1
  param_t get_param1() const;
  /// get timer parameter #2
  param_t get_param2() const;
  /// get timer parameters
  void get_params(param_t& p1, param_t& p2) const;
  /// Is it an absolute or relative timer?
  bool is_absolute() const;
  /// Is it an absolute or relative timer?
  bool is_relative() const;
 private:
  int32 time_sec;
  int32 time_msec;
  action_t action;
  param_t param1;
  param_t param2;
  bool ok;
  bool send_error;
  bool relative;
}; // end class TimerMsg
 
/** Get alarm time. Please check message action to see if this is an
 * absolute or relative time.
 * If this is a expiration notification, the time is absolute.
 */
inline
void 
TimerMsg::get_time(int32& sec, int32& msec) 
{
  sec = time_sec;
  msec = time_msec;
} // end get_alarm

/** Get timer message action. There is no way to set the action value, this
 * is done by start_relative or start_absolute... 
 */
inline
TimerMsg::action_t 
TimerMsg::get_action() const 
{
	return action;
} // end get_action

inline
const char*
TimerMsg::get_action_str() const 
{
	return action_t_str[action];
} // end get_action_str

/** Get result flag. */
bool 
inline
TimerMsg::is_ok() 
{
  return ok;
} // end is_ok


/// timer module parameters
/** This holds the message queue and the default sleep time. */
struct TimerModuleParam : public ThreadParam 
{
  TimerModuleParam(uint32 sleep_time = ThreadParam::default_sleep_time,
		   bool sua = false, bool ser = true, bool sre = true);
  /// send messages until abort
  const bool send_until_abort;
  const message::qaddr_t source;
  const bool send_error_expedited;
  const bool send_reply_expedited;
}; // end TimerModuleParam

/// timer module class
/** This is the timer module. */
class TimerModule : public Thread, public TimerCallback 
{
  /***** TimerCallback *****/
 public:
  /// callback member function
  /** @param timer timer ID
   * @param callback_param additional callback parameter.
   */
  virtual void timer_expired(timer_id_t timer, timer_callback_param_t callback_param);
 public:
  /// constructor
  TimerModule(const TimerModuleParam& p);
  /// destructor
  virtual ~TimerModule();
  /// timer module main loop
  virtual void main_loop(uint32 nr);
 private:
  /// wait for incoming messages
  void process_queue();
  /// check timers and send messages
  void process_elapsed_timers();
  /// start a timer
  bool start_timer(TimerMsg* m);
  /// restart a timer
  bool restart_timer(TimerMsg* m);
  /// stop a timer
  bool stop_timer(TimerMsg* m);
  /// stop all timers
  bool stop_all_timers();
  /// send back error
  void send_error_or_dispose(TimerMsg* m, bool ok);
  /// timer module parameters
  TimerManager tm;
  /// module parameters
  const TimerModuleParam timerparam;
  /// timer map
  /** This stores timer IDs and the corresponding message IDs. */
  class TimerMap 
  {
  public:
    bool insert(timer_id_t tid, TimerMsg* m);
    /// lookup message ID
    message::id_t lookup_mid(timer_id_t tid) const;
    /// lookup timer ID
    timer_id_t lookup_tid(message::id_t mid) const;
    /// lookup message
    TimerMsg* lookup_msg(timer_id_t tid) const;
    /// erase record of timer ID and message ID
    void erase(timer_id_t tid, message::id_t mid, bool dispose = true);
    /// clear all
    void clear(bool dispose = true);
  private:
    typedef unordered_map<message::id_t,timer_id_t> mid2tid_t;
    typedef mid2tid_t::const_iterator const_mid2tid_it_t;
    typedef unordered_map<timer_id_t,message::id_t> tid2mid_t;
    typedef tid2mid_t::const_iterator const_tid2mid_it_t;
    typedef unordered_map<timer_id_t,TimerMsg*> tid2msg_t;
    typedef tid2msg_t::const_iterator const_tid2msg_it_t;
    mid2tid_t mid2tid;
    tid2mid_t tid2mid;
    tid2msg_t tid2msg;
  } tmap; // end class TimerMap
}; // end class TimerModule

//@}

} // end namespace protlib

#endif
