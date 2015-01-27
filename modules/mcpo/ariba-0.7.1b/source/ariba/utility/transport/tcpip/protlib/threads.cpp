/// ----------------------------------------*- mode: C++; -*--
/// @file threads.cpp
/// A Thread class for POSIX threads
/// ----------------------------------------------------------
/// $Id: threads.cpp 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/threads.cpp $
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

#include <sys/time.h>

#include "threads.h"


namespace protlib {
   using namespace log;

/** @addtogroup protlib
 * @{
 */


/** This is the default sleep time and can be used as default value in
 * constructors.
 */
uint32 ThreadParam::default_sleep_time = 5;


/**
 * Initializes a ThreadParam object with a default wait time and a 
 * a thread (group) name string.
 *
 * @param wait wait time between stopping and aborting the thread inside
 *    ThreadStarter::abort_processing
 * @param n name of the threads.
 * @param minc minimal number of threads
 * @param maxc maximal number of threads
 */
ThreadParam::ThreadParam(uint32 wait, const char *n, uint32 minc, uint32 maxc) 
		: sleep_time(wait), name(n ? n : "UNKNOWN"),
		  min_count(minc), max_count(maxc) {

	  assert( minc > 0 );
	  assert( maxc >= minc );
}


/**
 * Constructor.
 *
 * @param p thread parameters
 * @param create_queue if true, create one internal queue
 * @param exp_allow if true, allow reception of expedited messages on the queue
 */
Thread::Thread(const ThreadParam &p, bool create_queue, bool exp_allow)
	: running_threads(0), started_threads(0), state(STATE_INIT), tparam(p),
	  fq(create_queue ? new FastQueue(p.name.c_str(), exp_allow) : 0) {

	pthread_mutexattr_t mutex_attr;

	pthread_mutexattr_init(&mutex_attr);

#ifdef _DEBUG
	pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_ERRORCHECK);
#else
	pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_NORMAL);
#endif

	pthread_mutex_init(&mutex, &mutex_attr);
	pthread_cond_init(&cond,NULL);

	pthread_mutexattr_destroy(&mutex_attr);
}


/**
 * Destructor.
 *
 * Currently throws an exception if there are still running threads.
 */
Thread::~Thread() {
	if ( get_running_threads() )
		throw ThreadError(ThreadError::ERROR_STILL_RUNNING);

	delete fq; // delete queue, no-op if fq is NULL

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}


/**
 * Called for each thread when processing is started.
 *
 * The thread must not be locked because this is done inside this method.
 * Cancellation is enabled and set to synchronous mode. So you only need to
 * install cleanup handlers when there is a cancellation point between
 * calls to lock() and unlock().
 */
void *Thread::start_processing() {

	lock();

	switch (state) {
		case STATE_INIT: 
			state=STATE_RUN;
			break;
		case STATE_RUN:
			break;
		case STATE_STOP:
		case STATE_ABORT:
			unlock();
			return NULL;
	}

	inc_running_threads();
	inc_started_threads();

	int thread_num = get_started_threads();

	unlock();

	/*
	 * Catch exceptions for logging, but don't rethrow them as this would
	 * lead to undefined behaviour (probably crashing the ThreadStarter).
	 *
	 * All exceptions should be handled in main_loop(), it is a programming
	 * error if they are propagated up to this point!
	 */
	try {
		main_loop(thread_num);
	}
	catch ( ProtLibException &e ) {
		ERRLog("Threads", "Unhandled ProtLibException in thread "
			<< tparam.name << ", num " << thread_num << ", error ["
			<< e.getstr() << ']');
	}
	catch ( bad_alloc & ) {
		ERRLog("Threads", tparam.name << ", num " << thread_num
			<< ": [out of memory]");
	}
	catch ( ... ) {
		ERRLog("Threads", "Unhandled non-ProtLibException in thread "
			<< tparam.name << ", num " << thread_num);
	}

	lock();
	dec_running_threads();
	unlock();

	return NULL;
}


/**
 * Called when the thread is asked to stop processing.
 *
 * The thread object may do some cleanup or work on until it has completed
 * a task.
 *
 * @param do_lock if true the thread mutex is used
 */
void Thread::stop_processing(bool do_lock) {
	if ( do_lock )
		lock();

	if (state==STATE_RUN) {
		state = STATE_STOP;
		signal_cond();
	}

	if ( do_lock )
		unlock();
}


/**
 * This is called just before a running thread is killed.
 *
 * @param do_lock if true the thread mutex is used
 */
void Thread::abort_processing(bool do_lock) {
	if ( do_lock )
		lock();

	if ( state == STATE_RUN  ||  state == STATE_STOP ) {
		state = STATE_ABORT;
		signal_cond();
	}

	if ( do_lock )
		unlock();
}


/**
 * Checks whether there is still a running thread. 
 *
 * @param do_lock if true the thread mutex is used
 */
bool Thread::is_running(bool do_lock) {

	if ( do_lock )
		lock();

	bool res = ( get_running_threads() > 0 );

	if ( do_lock )
		unlock();

	return res;
}


/**
 * Wait for the condition.
 *
 * @param sec relative time (seconds)
 * @param nsec relative time (nanoseconds)
 * @return 0, ETIMEDOUT or EINTR.
 */
int Thread::wait_cond(int32 sec, int32 nsec) {
	struct timeval tv;
	struct timespec ts;

	if ( sec < 0 )
		sec = 0;
	if ( nsec < 0 )
		nsec = 0;

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec+sec;
	ts.tv_nsec = tv.tv_usec*1000+nsec;

	// TODO: This is weird.
	while ( ts.tv_nsec > 1000000000) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}

	if ( ts.tv_sec < 0 )
		ts.tv_sec = 0;
	if ( ts.tv_nsec < 0 )
		ts.tv_nsec = 0;

	return pthread_cond_timedwait(&cond, &mutex, &ts);
}


/**
 * Returns the thread's state.
 *
 * @param do_lock if true the thread mutex is used
 * @return the thread's current state
 *
 * @see enum state_t for more information on what a thread state is.
 */
Thread::state_t Thread::get_state(bool do_lock) {
	if ( do_lock )
		lock();

	state_t s = state;

	if ( do_lock )
		unlock();

	return s;
}

/// get time of day as timespec
void Thread::get_time_of_day(struct timespec& ts) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec*1000;
}

const char* ThreadError::getstr() const {
	return errstr[(int)err];
}

const char* const ThreadError::errstr[] = {
	"Cannot create POSIX Threads.",
	"Thread is running.",
	"Thread is going to stop.",
	"Thread is aborting.",
	"Still running threads left.",
	"ThreadStarter is not initialized correctly."
	"Internal ThreadStarter or Thread error.",
	"Thread has not been started yet."
};

//@}

} // end namespace protlib
