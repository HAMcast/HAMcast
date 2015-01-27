/// ----------------------------------------*- mode: C++; -*--
/// @file fastqueue.h
/// Fast and thread-safe queue to send/receive messages between 
/// POSIX threads
/// ----------------------------------------------------------
/// $Id: fastqueue.h 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/fastqueue/fastqueue.h $
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
/** 
 * @ingroup protlib
 * @{
 */

/** @file
 * Fast and thread-safe queue to send/receive messages between POSIX threads
 * This can be used as a port for a thread to receive messages (from any thread).
 * The implementations allows for arbitrary long queues, but queues grow by
 * element blocks containing ELEMENT_BLOCKSIZE elements. This avoids frequent
 * malloc/free operations.
 */

#ifndef QUEUE_TYPE
#define QUEUE_TYPE

#include <pthread.h>

#define ELEMENT_BLOCKSIZE    64
#define MAX_QUEUENAME_LENGTH 32


/* queue element block type */
typedef struct queue_elblock_struct
  {
    void *element[ELEMENT_BLOCKSIZE];
    int read, write;
    struct queue_elblock_struct *next_block;
  }
queue_elblock_t;


typedef struct queue_struct
  {
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;
    pthread_cond_t cond;

    unsigned long nr_of_elements;
    unsigned long exp_nr_of_elements;
    int exp_enabled;
    queue_elblock_t *first_block;
    queue_elblock_t *last_block;
    queue_elblock_t *exp_first_block;
    queue_elblock_t *exp_last_block;
//#ifdef QUEUELEN
    unsigned long queue_maxlength;
//#endif
    char name[MAX_QUEUENAME_LENGTH + 2];
  }
queue_t;

extern queue_t *create_queue (const char *name);
extern int enqueue_element_signal (queue_t * queuehead, void *element);
extern int enqueue_element_expedited_signal (queue_t * queuehead, void *element, int exp);
extern void *dequeue_element_wait (queue_t * queuehead);
extern void *dequeue_element_timedwait(queue_t *queuehead, const struct timespec *tspec);
extern int destroy_queue (queue_t * queuehead);
extern void *dequeue_element_nonblocking(queue_t * queuehead);
extern unsigned long queue_nr_of_elements(queue_t *queue);
extern int queue_is_expedited_enabled(queue_t *queue);
extern int queue_enable_expedited(queue_t *queue, int exp);

#endif

//@}
