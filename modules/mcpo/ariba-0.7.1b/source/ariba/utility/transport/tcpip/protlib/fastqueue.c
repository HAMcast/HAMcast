/// ----------------------------------------*- mode: C++; -*--
/// @file fastqueue.c
/// a simple FIFO queue with mutexes for use with pthreads
/// ----------------------------------------------------------
/// $Id: fastqueue.c 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/fastqueue/fastqueue.c $
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

/** @addtogroup protlib
 * @{
 */

/** @file
 * Fast and thread-safe queue.
 */

/******************************************************************************
 * fastqueue.c -- a simple FIFO queue with mutexes for use with pthreads      *
 * -------------------------------------------------------------------------- *
 * written by Roland Bless 1995                                               *
 * all operations enqueue,dequeue are done in O(1) which means constant time  *
 ******************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

                       /**** module interface ****/
#include "fastqueue.h"

                  /*************** defines *****************/
#define qerr(errnr)      fprintf(stderr,"queue.c: %s\n",queue_errmsg[errnr])

#ifndef ANDROID
	#define PTHREAD_MUTEX_NORMAL PTHREAD_MUTEX_TIMED_NP
#endif
#define PRI_OTHER_MIN  PRI_FG_MIN_NP
#define PRI_OTHER_MAX  PRI_FG_MAX_NP
#define PRI_FG_MIN_NP  8
#define PRI_FG_MAX_NP  15

#define CLOCK_REALTIME 0
#define NSEC_PER_SEC   1000000000
extern int eclock_gettime(struct timespec *tp);
#define clock_gettime(clock_id, tspec) eclock_gettime(tspec)

                 /*************** typedefs *****************/

enum {
       QERR_NONE,
       QERR_NOMEM,
       QERR_MUTEXINIT,
       QERR_MUTEXLOCK,
       QERR_MUTEXUNLOCK,
       QERR_MUTEXDESTROY,
       QERR_QEMPTY,
       QERR_QINVALID,
       QERR_QNOTEMPTY,
       QERR_CONDINIT,
       QERR_CONDWAIT,
       QERR_CONDSIGNAL,
       QERR_CONDDESTROY
};

const 
char *const queue_errmsg[]=
{
  "all ok",
  "can't get enough memory",
  "initializing mutex",
  "locking mutex",
  "unlocking mutex",
  "destroying mutex",
  "queue empty",
  "invalid queueobject",
  "destroying queue - queue not empty",
  "initializing queue condition variable",
  "waiting on condition",
  "signalling condition",
  "destroying condition"
};



queue_t *create_queue(const char* name)
/* initialization routine for a queue.
 * returns: NULL if an error occured, or a queue-object which is actually
 *          a queueheader structure
 * arguments: none
 */
{
  queue_t *queuehead;

  /* Allocate memory for queue head */
  if ((queuehead= (queue_t *) malloc(sizeof(queue_t)))!=NULL)
  {
    /* Set mutex kind */
    pthread_mutexattr_init(&queuehead->mutex_attr);
#ifdef _DEBUG
    pthread_mutexattr_settype(&queuehead->mutex_attr,PTHREAD_MUTEX_ERRORCHECK);
#else
    pthread_mutexattr_settype(&queuehead->mutex_attr,PTHREAD_MUTEX_NORMAL);
#endif
    /* use PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_ERRORCHECK_NP for testing */

    /* Initialize mutex */
    if (pthread_mutex_init(&queuehead->mutex, &queuehead->mutex_attr)==0)
    {
      /* Create Condition variable for command queue */
      if (pthread_cond_init(&queuehead->cond, NULL)==0)
      {
        queuehead->nr_of_elements= 0UL;
        queuehead->exp_nr_of_elements= 0UL;
        queuehead->exp_enabled = 0;
        queuehead->first_block= (queue_elblock_t *) malloc(sizeof(queue_elblock_t));
        queuehead->exp_first_block= (queue_elblock_t *) malloc(sizeof(queue_elblock_t));
        if ((queuehead->first_block == NULL) || (queuehead->exp_first_block == NULL))
           qerr(QERR_NOMEM);
        else
	{
          queuehead->first_block->read=  0;
          queuehead->first_block->write= 0;
          queuehead->first_block->next_block= NULL;          
          queuehead->last_block= queuehead->first_block;
          queuehead->exp_first_block->read=  0;
          queuehead->exp_first_block->write= 0;
          queuehead->exp_first_block->next_block= NULL;          
          queuehead->exp_last_block= queuehead->exp_first_block;
          if (name)
          {
            if (strlen(name) <= MAX_QUEUENAME_LENGTH)
              strcpy(queuehead->name, name);
            else
            {
              memcpy(queuehead->name, name, MAX_QUEUENAME_LENGTH);
              queuehead->name[MAX_QUEUENAME_LENGTH + 1] = '\0';
            }
          }
          else
            queuehead->name[0] = '\0';
          /* Now it's simple to enqueue elements, esp. the first one */
        }
//#ifdef QUEUELEN
        queuehead->queue_maxlength= 0;
//#endif
      }
      else
        qerr(QERR_CONDINIT);
    }
    else /* error during initialize */
      qerr(QERR_MUTEXINIT);
  }
  else
    qerr(QERR_NOMEM);

  return queuehead;
}

int enqueue_element_signal(queue_t *queuehead, void *element)
{
  return enqueue_element_expedited_signal(queuehead,element,0);
}

int enqueue_element_expedited_signal(queue_t *queuehead, void *element, int exp)
/* add a new element into the queue. Memory for the element must be
 * allocated anywhere else. This routine signals other waiting threads.
 * returns: -1 if an error occured, or 0 is action could be performed
 * arguments: pointer to queue_t object, pointer to an element
 */
{
  queue_elblock_t *newelement, *lastblockp;

  if (queuehead==NULL)
  {
    qerr(QERR_QINVALID);
    return -1;
  }

  if (pthread_mutex_lock(&queuehead->mutex)!=0)
  {
    qerr(QERR_MUTEXLOCK);
    return -1;
  }
  /* begin critical section */

  if (exp && queuehead->exp_enabled) exp = 1; else exp = 0;
  /* Allocate new element structure when necessary */
  /* Note: queuehead->last_block must always contain a valid value */
  lastblockp = (exp ? (queuehead->exp_last_block) : (queuehead->last_block));
  if (lastblockp->write == ELEMENT_BLOCKSIZE)
  { /* last block is full, so allocate a new block */
    if ((newelement= (queue_elblock_t *) malloc(sizeof(queue_elblock_t)))==NULL)
    {
      qerr(QERR_NOMEM);
      return -1;
    }

    /* initialize new structure */
    newelement->element[0]= element;
    newelement->read      = 0;
    newelement->write     = 1;
    newelement->next_block= NULL;
    
    /* append new element to the end */
    lastblockp->next_block= newelement;
    /* new element becomes last element */
    if (exp) queuehead->exp_last_block = newelement; 
    else queuehead->last_block = newelement; 
  }
  else /* last block was not full */
  { 
    lastblockp->element[lastblockp->write]= element;
    lastblockp->write++;
  }
  
  if (exp) queuehead->exp_nr_of_elements++;
  queuehead->nr_of_elements++;
//#ifdef QUEUELEN
    if (queuehead->nr_of_elements > queuehead->queue_maxlength)
       queuehead->queue_maxlength= queuehead->nr_of_elements;
//#endif
  /* Condition should be set while mutex is locked. 
     Recommended by libc manual.
  */
  if (pthread_cond_signal(&queuehead->cond)!=0)
     qerr(QERR_CONDSIGNAL);
  /* end critical section */
  if (pthread_mutex_unlock(&queuehead->mutex)!=0)
  {
    qerr(QERR_MUTEXUNLOCK);
    return -1;
  }
  // see above
//  if (pthread_cond_signal(&queuehead->cond)!=0)
//     qerr(QERR_CONDSIGNAL);

  return 0;
}


void *dequeue_element_wait(queue_t *queuehead)
/* wait for the queue to contain an element.
 * if it contains an element return and remove it.
 * returns: NULL if an error occured, the pointer to the element otherwise
 * arguments: pointer to queue_t object
 */
{
  void       *element;
  queue_elblock_t *blockp;
  int exp = 0;
  element= NULL;
  int retcode= 0;

  if (queuehead != NULL)
  {
    /* Wait for an element in the queue */
    /* Before waiting on a condition, the associated mutex must be locked */
    if (pthread_mutex_lock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXLOCK); return NULL;
    }

    while(queuehead->nr_of_elements==0) /* while there is no work to do, wait */
    { /* for a safe state the predicate must be checked in a loop! */
      /* cond_wait() unlocks the mutex and might return sometimes without
         getting a signal! */
      if ((retcode= pthread_cond_wait(&queuehead->cond, &queuehead->mutex)) != 0)
      {
         if (retcode!=EINTR && retcode!=ETIMEDOUT)
	 {
	   qerr(QERR_CONDWAIT);
	 }
      }
    }

    /* begin critical section */
    exp = (queuehead->exp_nr_of_elements!=0);
    blockp = (exp ? (queuehead->exp_first_block) : (queuehead->first_block));
    if (blockp != NULL)
    {
      /* get the first element */
      element= blockp->element[blockp->read];
      blockp->read++;

      if (blockp->next_block == NULL) /* this is the last block */
      {
        if (blockp->read == blockp->write) 
        { /* block is completely dequeued, so reset values */
          /* the last block always remains allocated! */
          blockp->read=  0;
          blockp->write= 0;
        }
      }
      else /* this is not the last block */
      {
        /* if block was completely dequeued, remove it */
        if (blockp->read == ELEMENT_BLOCKSIZE)
        {
          if (exp) queuehead->exp_first_block= blockp->next_block;
          else queuehead->first_block= blockp->next_block;
          free(blockp);
        }
      }
      if (exp) queuehead->exp_nr_of_elements--;
      queuehead->nr_of_elements--;
    }
    else
      qerr(QERR_QEMPTY);

    /* end critical section */
    if (pthread_mutex_unlock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXUNLOCK);
      return NULL;
    }
  }
  else
    qerr(QERR_QINVALID);

  return element;
}

void *dequeue_element_timedwait(queue_t *queuehead, const struct timespec *tspec)
/* wait for the queue to contain an element.
 * if it contains an element return and remove it.
 * returns: NULL if an error occured, the pointer to the element otherwise
 * arguments: pointer to queue_t object
 *            tpsec is the time interval to wait (not an absolute time!)
 */
{
  void       *element;
  queue_elblock_t *blockp;
  int result;
  struct timespec abs_tspec;
  int exp = 0;
  element= NULL;

  if (queuehead != NULL)
  {
    /* Wait for an element in the queue */
    /* Before waiting on a condition, the associated mutex must be locked */
    if (pthread_mutex_lock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXLOCK); return NULL;
    }

    while(queuehead->nr_of_elements==0) /* while there is no work to do, wait */
    { /* for a safe state the predicate must be checked in a loop! */
      /* cond_wait() unlocks the mutex and might return sometimes without
         getting a signal! */
      clock_gettime(CLOCK_REALTIME, &abs_tspec);
      abs_tspec.tv_nsec+= tspec->tv_nsec;
      abs_tspec.tv_sec+= tspec->tv_sec;
      if (abs_tspec.tv_nsec >= NSEC_PER_SEC)
      {
        abs_tspec.tv_nsec%= NSEC_PER_SEC;
        abs_tspec.tv_sec++;
      };

      if ((result = pthread_cond_timedwait(&queuehead->cond,
                                           &queuehead->mutex, &abs_tspec))!=0)
      {
	if ( (result != ETIMEDOUT) && (result != EINTR) && (result != EINVAL) ) 
        {
	  qerr(QERR_CONDWAIT);
	} 
        else 
        { /* timeout */
	  if (pthread_mutex_unlock(&queuehead->mutex)!=0) 
          {
	    qerr(QERR_MUTEXUNLOCK);
	    return NULL;
	  } 
	  return NULL;
	}
      }
    }

    /* begin critical section */
    exp = (queuehead->exp_nr_of_elements!=0);
    blockp = (exp ? (queuehead->exp_first_block) : (queuehead->first_block));
    if (blockp != NULL)
    {
      /* get the first element */
      element= blockp->element[blockp->read];
      blockp->read++;

      if (blockp->next_block == NULL) /* this is the last block */
      {
        if (blockp->read == blockp->write) 
        { /* block is completely dequeued, so reset values */
          /* the last block always remains allocated! */
          blockp->read=  0;
          blockp->write= 0;
        }
      }
      else /* this is not the last block */
      {
        /* if block was completely dequeued, remove it */
        if (blockp->read == ELEMENT_BLOCKSIZE)
        {
          if (exp) queuehead->exp_first_block= blockp->next_block;
          else queuehead->first_block= blockp->next_block;
          free(blockp);
        }
      }
      if (exp) queuehead->exp_nr_of_elements--;
      queuehead->nr_of_elements--;
    }
    else
      qerr(QERR_QEMPTY);

    /* end critical section */
    if (pthread_mutex_unlock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXUNLOCK);
      return NULL;
    }
  }
  else
    qerr(QERR_QINVALID);

  return element;
}

int destroy_queue(queue_t *queuehead)
/* destroys the queue and frees all resources, except the elements!
 * the queue must be empty to destroy it.
 * returns: -1 if an error occured, 0 otherwise
 * arguments: pointer to queue_t object
 */
{
  if (queuehead!=NULL)
  {
    /* queue not empty? */
    if (queuehead->nr_of_elements != 0)
      qerr(QERR_QNOTEMPTY);
    else
    {
      /* destroy condition variable */
      if (pthread_cond_destroy(&queuehead->cond)!=0) qerr(QERR_CONDDESTROY);
      /* destroy mutex */
      if (pthread_mutex_destroy(&queuehead->mutex)!=0) qerr(QERR_MUTEXDESTROY);
      
      pthread_mutexattr_destroy(&queuehead->mutex_attr);

      /* free memory for queuehead */
#ifdef QUEUELEN
      fprintf(stderr,"queue.c: length of queue (%s) growed up to %lu elements\n",
              queuehead->name, queuehead->queue_maxlength);
#endif      
      free(queuehead->exp_last_block);
      free(queuehead->last_block);
      free(queuehead);
    }

    return 0;
  }
  else
    qerr(QERR_QINVALID);

  return -1;
}

void *dequeue_element_nonblocking(queue_t *queuehead)
/* if queue contains an element return and remove it.
 * returns: NULL if an error occured or queue was empty, the pointer to
 * the element otherwise.
 * arguments: pointer to queue_t object
 */
{
  void       *element;
  queue_elblock_t *blockp;
  int exp = 0;
  element= NULL;

  if (queuehead != NULL)
  {
    if (pthread_mutex_lock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXLOCK); return NULL;
    }

    /* begin critical section */

    if (queuehead->nr_of_elements==0) 
    { 
      if (pthread_mutex_unlock(&queuehead->mutex)!=0) qerr(QERR_MUTEXUNLOCK);
      return NULL;
    }

    exp = (queuehead->exp_nr_of_elements!=0);
    blockp = (exp ? (queuehead->exp_first_block) : (queuehead->first_block));
    if (blockp != NULL)
    {
      /* get the first element */
      element= blockp->element[blockp->read];
      blockp->read++;

      if (blockp->next_block == NULL) /* this is the last block */
      {
        if (blockp->read == blockp->write) 
        { /* block is completely dequeued, so reset values */
          /* the last block always remains allocated! */
          blockp->read=  0;
          blockp->write= 0;
        }
      }
      else /* this is not the last block */
      {
        /* if block was completely dequeued, remove it */
        if (blockp->read == ELEMENT_BLOCKSIZE)
        {
          if (exp) queuehead->exp_first_block= blockp->next_block;
          else queuehead->first_block= blockp->next_block;
          free(blockp);
        }
      }
      if (exp) queuehead->exp_nr_of_elements--;
      queuehead->nr_of_elements--;
    }
    else
      qerr(QERR_QEMPTY);

    /* end critical section */
    if (pthread_mutex_unlock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXUNLOCK);
      return NULL;
    }
  }
  else
    qerr(QERR_QINVALID);

  return element;
}

unsigned long queue_nr_of_elements(queue_t *queuehead)
/** Get number fo elements in queue. */
{
  unsigned long result = 0;

  if (queuehead != NULL)
  {
    if (pthread_mutex_lock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXLOCK);
      return 0;
    }
    /* begin critical section */

    result = queuehead->nr_of_elements; 

    /* end critical section */
    if (pthread_mutex_unlock(&queuehead->mutex)!=0) qerr(QERR_MUTEXUNLOCK);
  }
  else
    qerr(QERR_QINVALID);

  return result;
}

int queue_is_expedited_enabled(queue_t *queuehead)
/** Get exp_enabled flag. */
{
  int result = 0;

  if (queuehead != NULL)
  {
    if (pthread_mutex_lock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXLOCK);
      return 0;
    }
    /* begin critical section */

    result = queuehead->exp_enabled; 

    /* end critical section */
    if (pthread_mutex_unlock(&queuehead->mutex)!=0) qerr(QERR_MUTEXUNLOCK);
  }
  else
    qerr(QERR_QINVALID);

  return result;
}

int queue_enable_expedited(queue_t *queuehead, int exp)
/** Set exp_enabled flag and return old value. */
{
  int result = 0;

  if (queuehead != NULL)
  {
    if (pthread_mutex_lock(&queuehead->mutex)!=0)
    {
      qerr(QERR_MUTEXLOCK);
      return 0;
    }
    /* begin critical section */

    result = queuehead->exp_enabled; 
    if (exp) queuehead->exp_enabled = 1;
    else queuehead->exp_enabled = 0;

    /* end critical section */
    if (pthread_mutex_unlock(&queuehead->mutex)!=0) qerr(QERR_MUTEXUNLOCK);
  }
  else
    qerr(QERR_QINVALID);

  return result;
}

//@}
