/// ----------------------------------------*- mode: C++; -*--
/// @file queuemanager.cpp
/// queuemanager which records queues and message source IDs
/// ----------------------------------------------------------
/// $Id: queuemanager.cpp 2774 2007-08-08 12:32:08Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/queuemanager.cpp $
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
#include <stdexcept>

#include "queuemanager.h"
#include "logfile.h"

namespace protlib { 

/** @addtogroup protlib
 * \ingroup protlib
 * @{
 */

using namespace log;

/***** class QueueManagerError *****/

QueueManagerError::QueueManagerError(error_t e) : err(e) {}

const char* QueueManagerError::getstr() const { return errstr[err]; }

const char* const QueueManagerError::errstr[] = {
	"Unable to create QueueManager.",
	"Cannot register FastQueue. No memory or registered queue more than once."
}; // end errstr

/***** class QueueManager *****/

/** Return QueueManager singleton. */
QueueManager* QueueManager::instance() {
	if (!inst) {
		// try to create singleton
		inst = new(nothrow) QueueManager();
		if (!inst) {
		  Log(INFO_LOG,LOG_NORMAL, "QueueManager" ,"Cannot created QueueManager singleton.");
		  throw QueueManagerError(QueueManagerError::ERROR_NO_QUEUE_MANAGER);
		} else {
		  Log(DEBUG_LOG,LOG_NORMAL, "QueueManager", "Just created QueueManager singleton.");
		} // end if not inst
	} // end if not inst
	return inst;
} // end QueueManager

/**
 * Delete the QueueManager singleton object.
 *
 * After a call to clear references to that object become invalid and must
 * be updated by a call to instance().
 */
void QueueManager::clear() {

	if (inst) {
		QueueManager *tmp = inst;
		inst = 0;
		DLog("QueueManager", "Destroying QueueManager singleton ...");
		delete tmp;
	}

	DLog("QueueManager", "The QueueManager singleton has been destroyed");
}


/**
 * Register a queue.
 *
 * This registers a FastQueue for the given message source ID with the
 * QueueManager.
 *
 * The registered queue (and all its entries) is deleted as soon as the
 * QueueManager is deleted. Because of this, a queue may only be registered
 * once.
 *
 * @param fq pointer to an already allocated fastqueue 
 * @param s  message source ID
 */
void QueueManager::register_queue(FastQueue* fq, message::qaddr_t s) {
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  // expand array if necessary
  if (((uint32)s)>=queue_arr.capacity()) {
    Log(DEBUG_LOG,LOG_NORMAL, "QueueManager", "expanding queue array from " << s << " to " << s+5);
    // get more memory
    queue_arr.reserve(s+5);
    while (queue_arr.size()<queue_arr.capacity()) queue_arr.push_back(NULL);
  } // end get more memory
  
  if (queue_arr[s]) 
  {
    // queue already exists
    Log(ERROR_LOG,LOG_CRIT, "QueueManager", "A queue for " << s << " is already  registered");
    throw QueueManagerError(QueueManagerError::ERROR_REGISTER);
  } // end if queue exists
  else 
  {
    // register queue
    if (fq)
    {
      queue_arr[s] = fq;
    }
    else
    {
      Log(ERROR_LOG,LOG_CRIT, "QueueManager", "Cannot register queue for " << s);
      throw QueueManagerError(QueueManagerError::ERROR_REGISTER);
    }
  } // end else no queue exists
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
} // end register_queue


void 
QueueManager::unregister_queue(message::qaddr_t s) 
{
  pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
  try {
	  queue_arr.at(s) = 0;
  }
  catch ( std::out_of_range ) {
	/*
	 * Nothing to do, queue has probably already been unregistered,
	 * probably by calling QueueManager::clear().
	 */
  }
  pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
}

FastQueue* QueueManager::get_queue(message::qaddr_t s) const {
	FastQueue* fq = NULL;
	pthread_mutex_lock(&mutex); // install_cleanup_mutex_lock(&mutex);
	if (((uint32)s)<queue_arr.size()) {
		fq = queue_arr[s];
	} else {
		fq = NULL;
	} // end if
	pthread_mutex_unlock(&mutex); // uninstall_cleanup(1);
	return fq;
} // end get

QueueManager* QueueManager::inst = NULL;

/**
 * Constructor.
 */
QueueManager::QueueManager() : queue_arr(QueueManager::INITIAL_ARRAY_SIZE) {
	pthread_mutexattr_t mutex_attr;

	pthread_mutexattr_init(&mutex_attr);

#ifdef _DEBUG
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
#else
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_NORMAL);
#endif

	pthread_mutex_init(&mutex, &mutex_attr);

	pthread_mutexattr_destroy(&mutex_attr); // doesn't affect mutex
}


/**
 * Destructor.
 *
 * Delete this object and all FastQueue objects that are still registered.
 */
QueueManager::~QueueManager() {

	pthread_mutex_lock(&mutex);

	// count queues which are still registered
	for ( qm_array_it_t i = queue_arr.begin(); i != queue_arr.end(); i++) 
		if ( *i != 0 )
			WLog("QueueManager",
				"~QueueManager(): queue " << (*i)->get_name()
				<< " has not been unregistered");

	pthread_mutex_unlock(&mutex);

	pthread_mutex_destroy(&mutex);
}

//@}

} // end namespace protlib
