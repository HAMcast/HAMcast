/// ----------------------------------------*- mode: C++; -*--
/// @file queuemanager.h
/// This is the queuemanager which records queues and message source IDs
/// ----------------------------------------------------------
/// $Id: queuemanager.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/queuemanager.h $
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
 * This is the queuemanager which records queues and message source IDs.
 */

#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include "protlib_types.h"
#include "cleanuphandler.h"

#include <vector>

#include "fqueue.h"
#include "messages.h"

namespace protlib { 

/** @addtogroup protlib
 * \ingroup protlib
 * @{
 */

/// QueueManager errors
class QueueManagerError : public ProtLibException {
public:
	/// error codes
	enum error_t {
		ERROR_NO_QUEUE_MANAGER,
		ERROR_REGISTER
	}; // end error_t
	const error_t err;
	/// constructor
	QueueManagerError(error_t e);
	virtual const char* getstr() const;
	virtual const char *what() const throw() { return getstr(); }
private:
	/// QueueManager error strings
	static const char* const errstr[];
}; // end class QueueManagerError


/**
 * The Queue Manager singleton.
 *
 * The QueueManager manages several FastQueue objects. A queue is registered
 * with the manager using a simple (key, value) scheme. The key is of type
 * message:qaddr_t, and the value is a FastQueue object.
 *
 * You can send messages to a registered queue using the message::send_to(dest)
 * method, where dest has to be the message::qaddr_t which has been used for
 * registering the queue.
 *
 * This class is used as a singleton, so there will usually be only one
 * object of this class.
 */
class QueueManager {
public:
	/// return QueueManager singleton instance
	static QueueManager* instance();
	/// clear QueueManager 
	static void clear();
	/// register a queue
	void register_queue(FastQueue* fq, message::qaddr_t s);
	/// deregister a queue
	void unregister_queue(message::qaddr_t s);

	/// get queue
	FastQueue* get_queue(message::qaddr_t s) const;
private:
	/// QueueManager instance
	static QueueManager* inst;
	/// constructor
	QueueManager();
	/// Destruktor
	~QueueManager();
	/// QueueManager array
	typedef vector<FastQueue*> qm_array_t;
	/// QueueManager array iterator
	typedef qm_array_t::iterator qm_array_it_t;
	/// array
	qm_array_t queue_arr;
	/// locking
	mutable pthread_mutex_t mutex;

	static const size_t INITIAL_ARRAY_SIZE = 128;
};

//@}

} // end namespace protlib

#endif
