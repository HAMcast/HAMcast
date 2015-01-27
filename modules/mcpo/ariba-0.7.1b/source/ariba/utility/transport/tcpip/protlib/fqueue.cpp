/// ----------------------------------------*- mode: C++; -*--
/// @file fqueue.cpp
/// wrapper class for fastqueue
/// ----------------------------------------------------------
/// $Id: fqueue.cpp 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/fqueue.cpp $
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

#include "fqueue.h"
#include "stdio.h"
#include "logfile.h"

#include <string>

namespace protlib {

/** @addtogroup protlib
 * @{
 */

  using namespace log;

/**
 * Constructor.
 *
 * Initialize a FastQueue with a queue name and enable/disable expedited
 * data.
 *
 * @param qname the queue's name, or NULL
 * @param exp if true, expedited data support is enabled
 */
FastQueue::FastQueue(const char *qname, bool exp)
    : queue_name((qname == 0) ? "" : (char*)qname), shutdownflag(false)
{
  if ((queue = create_queue(qname)) == NULL)
  {
    Log(ERROR_LOG, LOG_ALERT, "FastQueue", "Could not create queue " << queue_name);
    throw FQError();
  } else queue_enable_expedited(queue,exp);
}


/**
 * Add a message to the queue.
 *
 * If exp is true and the queue allows expedited data, the message will
 * pass all normal messages in the queue and thus will be delivered earlier.
 * If there are other expedited messages in the queue already, it will be
 * added after the already existing expedited messages.
 *
 * This method may fail (and return false) if the queue is in shutdown mode,
 * there is a problem aquiring locks, or some other threading problem.
 *
 * In case the queue is deleted before this message has been removed, this
 * message is deleted using the delete operator. Because of this, the same
 * message may only appear once in a queue.
 * 
 * @param element a pointer to the message to add
 * @param exp true if this is expedited data
 * 
 * @return true if the element was enqueued successfully
 */
bool FastQueue::enqueue(message *element, bool exp)
{
  if (shutdownflag) return false;
  if (enqueue_element_expedited_signal(queue, (void*)element, exp) < 0)
  {
    Log(ERROR_LOG, LOG_ALERT, "FastQueue", "Could not enqueue element in queue " << queue_name);
    return false;
  }
  return true;
}


/**
 * Wait for a message for a given time.
 *
 * If no message arrives in the given time period, NULL is returned.
 *
 * @param msec the time to wait in milliseconds
 *
 * @return the message, or NULL
 */
message *FastQueue::dequeue_timedwait(const long int msec)
{
  struct timespec tspec = {0,0};
  tspec.tv_sec = msec/1000;
  tspec.tv_nsec = (msec%1000)*1000000;
  return (message*)dequeue_element_timedwait(queue, &tspec);
}


/**
 * Destructor.
 * 
 * Destroys the queue. All messages which are still in the queue are deleted
 * using the delete operator.
 */
FastQueue::~FastQueue()
{
  if (queue) 
  {
    cleanup();
    if ((destroy_queue(queue)) < 0)
    {
      Log(ERROR_LOG, LOG_ALERT, "FastQueue", "Could not destroy queue " << queue_name);
    }
  }
  DLog("FastQueue", "~FastQueue() - done for queue " << queue_name);
}

/**
 * Test if the queue is empty.
 *
 * @return true if the queue is empty
 */
bool FastQueue::is_empty() const
{
  if (queue_nr_of_elements(queue)==0)
    return true;
  else
    return false;
}


/**
 * Return the number of messages in the queue.
 *
 * @return the number of enqueued messages
 */
unsigned long FastQueue::size() const
{
  return queue_nr_of_elements(queue);
}


/**
 * Test if expedited message support is enabled.
 * 
 * @return true if expedited message support is enabled
 */
bool FastQueue::is_expedited_enabled() const
{
  if (queue_is_expedited_enabled(queue))
    return true;
  else
    return false;
}

/**
 * Enable or disable expedited messages.
 *
 * This also returns the previous value of this flag.
 *
 * @return true, if expedited messages were previously enabled, false otherwise
 */
bool FastQueue::enable_expedited(bool exp)
{
  if (queue_enable_expedited(queue,exp))
    return true;
  else
    return false;
}


/**
 * Disable enqueueing of new messages.
 *
 * A queue in shutdown mode does not accept messages any more.
 */
void FastQueue::shutdown() { shutdownflag = true; }


/**
 * Put queue into shutdown mode and delete all stored messages..
 *
 * @return the number of messages that were in the queue
 */
unsigned long FastQueue::cleanup()
{
  unsigned long count = 0;
  message* m = NULL;
  shutdownflag = true;
  while (!is_empty())
    if ((m = dequeue(false))) {
      delete m;
      m = NULL;
      count++;
    }
  return count;
}

//@}

} // end namespace protlib
