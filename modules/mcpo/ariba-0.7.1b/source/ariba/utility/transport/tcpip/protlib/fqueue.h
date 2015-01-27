/// ----------------------------------------*- mode: C++; -*--
/// @file fqueue.h
/// a wrapper class for fastqueue C implementation
/// ----------------------------------------------------------
/// $Id: fqueue.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/fqueue.h $
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
 */

#ifndef __FQUEUE_H__
#define __FQUEUE_H__

#include <string>

#include "protlib_types.h"
#include "messages.h"

extern "C" {
#include"fastqueue.h"
}

namespace protlib {

/** @addtogroup protlib
 * @{
 */

// declared in messages.h
class message;


/** 
 * A fast message queue.
 *
 * This is a fast and thread-safe message queue with expedited data
 * support. It is an object oriented wrapper around fastqueue.c.
 * The queue grows dynamically and has no built-in entry limit.
 */
class FastQueue {
public:
	/// FastQueue error
	class FQError{};
	/// constructor
	FastQueue(const char *qname = 0, bool exp = false);
	/// destructor
	~FastQueue();
	/// enqueue message
	bool enqueue(message *element, bool exp = false);
	/// dequeue message
	message *dequeue(bool blocking = true);
	/// dequeue message, timed wait
	message *dequeue_timedwait(const struct timespec &tspec);
	/// dequeue message, timed wait
	message *dequeue_timedwait(const long int msec);
	/// is queue empty
	bool is_empty() const;
	/// get number of enqueued messages
	unsigned long size() const;
	/// is expedited data support enabled
	bool is_expedited_enabled() const;
	/// enable/disable expedited data
	bool enable_expedited(bool exp);
	/// shutdown queue, do not accept messages
	void shutdown();
	/// delete stored messages
	unsigned long cleanup();
	/// Return the name of the queue.
	const char* get_name() const { return queue_name.c_str(); }
private:
	/// C fastqueue
	queue_t *queue;
	/// name of the queue, also stored in the queue_t
	string queue_name;
	/// accept or reject messages
	bool shutdownflag;
};


/**
 * Remove the first message from the queue.
 *
 * Messages are removed in the same order they were added to the queue (FIFO).
 * If the expedited messages feature is enabled, there's an exception to
 * this rule: Expedited messages are always removed before all other messages.
 * The FIFO condition still holds among expedited messages, however.
 *
 * If blocking is set, wait infinitely for a message. If set to false,
 * return immediately if the queue is empty. In this case, NULL is returned.
 *
 * @param blocking if true, block until a message arrives
 *
 * @return the message, or NULL
 */
inline
message *
FastQueue::dequeue(bool blocking)
{
  return static_cast<message*>(blocking ? 
			       dequeue_element_wait(queue) :
			       dequeue_element_nonblocking(queue));
}


/**
 * Wait for a message for a given time.
 *
 * If no message arrives in the given time period, NULL is returned.
 *
 * @param tspec the time to wait
 *
 * @return the message, or NULL
 */
inline
message *
FastQueue::dequeue_timedwait(const struct timespec& tspec)
{
  return (message*)dequeue_element_timedwait(queue, &tspec);
}

//@}

} // end namespace protlib

#endif 
