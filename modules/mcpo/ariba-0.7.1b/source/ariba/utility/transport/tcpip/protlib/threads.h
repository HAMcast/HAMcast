/// ----------------------------------------*- mode: C++; -*--
/// @file threads.h
/// Thread support functions (classes thread and threadstarter) based on POSIX threads
/// ----------------------------------------------------------
/// $Id: threads.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/threads.h $
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
 * Classes to support multi-threaded programming.
 *
 * @ingroup protlib
 *
 * A Thread module class must inherit from Thread. Several instances may run
 * simultaneously but they share exactly one module object. So you must take
 * care of this fact when writing the module code and use locks accordingly.
 *
 * Use lock(), unlock(), wait_cond() and signal_cond() the way you would use
 * the corresponding POSIX thread functions.
 *
 * Use the ThreadStarter template class to create threads.
 */

#ifndef PROTLIB__THREADS_H
#define PROTLIB__THREADS_H

#include <vector>
#include <pthread.h>
#include <signal.h>
#include <sys/times.h>
#include <string>

#include "protlib_types.h"
#include "logfile.h"
#include "fqueue.h"

namespace protlib {
  using namespace log;

/** @addtogroup protlib
 * @{
 */


/**
 * Call the method start_processing of a Thread instance.
 *
 * @param thread_object a Thread instance
 */
template <class T> void *thread_starter(void *thread_object) {

#ifdef PTHREAD_CANCEL_ENABLE
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
#endif
#ifdef PTHREAD_CANCEL_DEFERRED
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
#endif

	(static_cast<T*>(thread_object))->start_processing();
	return NULL;
}


/**
 * Base class for thread object parameters.
 *
 * This is used by ThreadStarter to extract and store overall data like the
 * sleep time and is also accessible to the thread object.
 */
class ThreadParam {
  public:
	ThreadParam();
	ThreadParam(uint32 wait, const char* name,
		uint32 minc=1, uint32 maxc=(uint32)-1);

	static uint32 default_sleep_time;
	/// sleep time
	const uint32 sleep_time;
	const std::string name;
	/// minimum thread count
	const uint32 min_count;
	/// maximum thread count
	const uint32 max_count;
};


/**
 * This exception will be thrown if there is some trouble with threading.
 */
class ThreadError : public ProtLibException {
  public:
	enum error_t {
		ERROR_THREAD_CREATION, ERROR_RUNNING, ERROR_STOPPING,
		ERROR_ABORTING, ERROR_STILL_RUNNING, ERROR_UNINITIALIZED,
		ERROR_INTERNAL, ERROR_NOT_STARTED
	};

	ThreadError(error_t e) : err(e) { }
	virtual ~ThreadError() throw () { }

	virtual const char* getstr() const;
	virtual const char *what() const throw() { return getstr(); }
	const error_t err;

  protected:
	static const char* const errstr[];
};


/**
 * Abstract interface for thread modules.
 *
 * Don't confuse this Thread class with POSIX threads. A Thread class only
 * provides a main_loop method which will be executed by one or more POSIX
 * threads simultaneously. The Thread instance provides a central point for
 * all those POSIX threads to store data. Don't forget to lock() the Thread
 * instance to avoid race conditions if you want to access and/or modify
 * the data.
 */
class Thread {
  public:
	Thread(const ThreadParam& p,
		bool create_queue=true, bool exp_allow=true);
	virtual ~Thread();

	void *start_processing();
	void stop_processing(bool do_lock=true);
	void abort_processing(bool do_lock=true);

	bool is_running(bool do_lock=true);

	virtual void main_loop(uint32 thread_num) = 0;

	void lock();
	void unlock();

	void signal_cond();
	void broadcast_cond();
	void wait_cond();
	int wait_cond(const struct timespec& ts);
	int wait_cond(int32 sec, int32 nsec=0);

	/**
	* State of a thread.
	*
	* The state of a thread does not really tell whether there are threads
	* active or not. It only represents a state in the life cycle of a 
	* thread object.
	*/
	enum state_t {
		STATE_INIT, STATE_RUN, STATE_STOP, STATE_ABORT
	};

	state_t get_state(bool do_lock=true);
	FastQueue* get_fqueue() { return fq; }

	static void get_time_of_day(struct timespec& ts);

  private:
	/// This counter records the number of threads running on this object.
	uint32 running_threads;

	/// This counter records the number of started threads.
	uint32 started_threads;

	/** 
	* Thread-global mutex.
	*
	* This mutex is used to lock the thread object when data common to all
	* threads on this object is modified.
	*/
	pthread_mutex_t mutex;

	/// thread object condition
	pthread_cond_t cond;

	/// thread state
	state_t state;

	/// Thread parameters.
	const ThreadParam tparam;

	/// The input queue where threads can get messages from.
	FastQueue* fq;

	void inc_running_threads();
	void dec_running_threads();
	uint32 get_running_threads() const;
	void inc_started_threads();
	uint32 get_started_threads() const;
};
    

inline void Thread::lock() {
	if ( pthread_mutex_lock(&mutex) != 0 ) {
		ERRLog(tparam.name, "Error while locking mutex");
	}
}

inline void Thread::unlock() {
	int ret = pthread_mutex_unlock(&mutex);
	assert( ret == 0 );
}

inline void Thread::signal_cond() {
	pthread_cond_signal(&cond);
}

inline void Thread::broadcast_cond() {
	pthread_cond_broadcast(&cond);
}

inline void Thread::wait_cond() {
	pthread_cond_wait(&cond,&mutex);
}


/**
 * @param ts absolute time
 * @return 0, ETIMEDOUT or EINTR.
 */
inline int Thread::wait_cond(const struct timespec& ts) {
	return pthread_cond_timedwait(&cond, &mutex, &ts);
}


inline void Thread::inc_running_threads() {
	running_threads++;
}

inline void Thread::dec_running_threads() {
	assert( running_threads > 0 );
	running_threads--;
}

inline uint32 Thread::get_running_threads() const {
	return running_threads;
}

inline void Thread::inc_started_threads() {
	started_threads++;
}

inline uint32 Thread::get_started_threads() const {
	return started_threads;
}


/**
 * A template class used to start threads.
 *
 * Note that the ThreadStarter template class is not thread-safe yet, so it
 * may only be accessed by one thread at a time.
 */
template <class T, class TParam> class ThreadStarter {
  public:
	ThreadStarter(uint32 count, const TParam& param);
	~ThreadStarter();

	void start_processing();
	void stop_processing();
	bool sleepuntilstop();		// deprecated!
	void wait_until_stopped();
	void abort_processing(bool kill=false);

	/// get a pointer to the thread object
	inline T *get_thread_object() { return &thread_object; }

	/// Are all threads finished: TODO
	inline bool is_running() const { return thread_object.is_running(); }

  private:
	/// The Thread object on which the threads run.
	T thread_object;

	/// For debugging, the name of the thread as given by TParam.
	const TParam thread_param;

	/// Contains the handles of all pthreads that we created.
	std::vector<pthread_t> pthreads;
};


/**
 * Constructor.
 *
 * @param count the number of threads to start
 * @param param thread parameters
 */
template <class T, class TParam>
ThreadStarter<T, TParam>::ThreadStarter(uint32 count, const TParam& param)
		: thread_object(param), thread_param(param), pthreads(count) {

	// TODO: fix all Thread subclasses that use an invalid count!
	if ( count < param.min_count )
		count = param.min_count;
	else if ( count > param.max_count )
		count = param.max_count;

	assert( count >= param.min_count && count <= param.max_count );

	pthreads.resize(count); // TODO: remove
}


/**
 * Destructor.
 *
 * This cancels all running threads if there are still some.
 */
template <class T, class TParam> ThreadStarter<T, TParam>::~ThreadStarter() {

	if ( thread_object.is_running() ) {
		catch_all(stop_processing());
		sleepuntilstop();
		catch_all(abort_processing(true));
	}
}


/**
 * Start the threads.
 */
template <class T, class TParam>
void ThreadStarter<T, TParam>::start_processing() {

	thread_object.lock();

	/*
	 * Check if this is a fresh Thread. If it is or was already running,
	 * we have detected a programming error.
	 */
	if ( thread_object.is_running(false) ) {
		thread_object.unlock();

		ERRLog("Threads", "start_processing(): " << thread_param.name
			<< " is already running");

		throw ThreadError(ThreadError::ERROR_INTERNAL);
	}


	/*
	 * Create the requested number of threads.
	 */
	int res;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	for (unsigned i = 0; i < pthreads.size(); i++) {
		// Create a posix thread. It will start running immediately.
		res = pthread_create(&pthreads[i], &attr,
					&thread_starter<T>, &thread_object);

		if ( res != 0 ) {
			thread_object.unlock();
			ERRLog("Threads", "pthread_create() failed starting a "
				<< "thread for " << thread_param.name);

			throw ThreadError(ThreadError::ERROR_THREAD_CREATION);
		}
	}

	ILog("Threads", pthreads.size() << " " << thread_param.name
		<< " thread(s) sucessfully created");

	pthread_attr_destroy(&attr); // has no effect on the created threads

	thread_object.unlock();
}


/**
 * Ask all threads to stop (politely).
 */
template <class T, class TParam>
void ThreadStarter<T, TParam>::stop_processing() {

	thread_object.lock();

	typename T::state_t state = thread_object.get_state(false);

	switch (state) {

	  case T::STATE_INIT:
		thread_object.unlock();
		DLog("Threads", "Thread " << thread_param.name
			<< " has not been started yet.");
		throw ThreadError(ThreadError::ERROR_NOT_STARTED);
		break;

	  case T::STATE_RUN:
		thread_object.stop_processing(false);
		thread_object.unlock();

		ILog("Threads", "Thread(s) "
			<< thread_param.name << " asked to stop");
		break;

	  case T::STATE_STOP:
		thread_object.unlock();
		DLog("Threads", "Thread(s) "
			<< thread_param.name << " is already in state stop.");
		throw ThreadError(ThreadError::ERROR_STOPPING);
		break;

	  case T::STATE_ABORT:
		//thread_object.unlock();
		DLog("Threads", "Thread "
			<< thread_param.name << " is in state abort.");
		//throw ThreadError(ThreadError::ERROR_ABORTING);
		break;

	  default:
		assert( false ); // unknown state

	}
}


/**
 * Wait for the thread to stop running (DEPRECATED).
 *
 * Sleeps until all threads have stopped running but not longer than
 * sleep_time seconds.
 *
 * This method is deprecated because it suffers from a race condition:
 * If none of the pthreads created in start_processing() has been run yet,
 * then this method returns immediately. Use wait_until_stopped() instead.
 *
 * @return true if the threads have stopped
 *
 * @see ThreadParam
 */
template <class T, class TParam>
bool ThreadStarter<T, TParam>::sleepuntilstop() {
	
	for (uint32 i = 0; thread_object.is_running()
			&& i < thread_param.sleep_time; i++)
		sleep(1);

	return ( thread_object.is_running() ? false : true );
}


/**
 * Wait until all threads have stopped running.
 *
 * Threads that haven't been running yet (state IDLE) are not considered
 * as stopped!
 */
template <class T, class TParam>
void ThreadStarter<T, TParam>::wait_until_stopped() {

	DLog("Threads",
		"Waiting for Thread " << thread_param.name << " to stop");

	Thread::state_t state = thread_object.get_state(false);

	while ( state == Thread::STATE_INIT || thread_object.is_running() ) {
		sleep(1);
		state = thread_object.get_state(false);
	}

	DLog("Threads", "Thread " << thread_param.name << " has stopped");
}


/**
 * Stop and kill the threads.
 *
 * @param kill kill the threads if they do not stop. 
 */
template <class T, class TParam>
void ThreadStarter<T, TParam>::abort_processing(bool kill) {

	thread_object.lock();

	switch ( thread_object.get_state(false) ) {

	  case T::STATE_INIT:
		thread_object.unlock();
		DLog("Threads", "Thread "
			<< thread_param.name << " has not been started yet.");
		throw ThreadError(ThreadError::ERROR_NOT_STARTED);
		break;

	  case T::STATE_ABORT: 
		if ( ! kill ) {
			//thread_object.unlock();
			DLog("Threads", "Thread " << thread_param.name
				<< " is already in state abort.");

			//throw ThreadError(ThreadError::ERROR_ABORTING);
		}
		break;

	  default:
		break;
	}

	if ( thread_object.is_running(false) ) {
		thread_object.stop_processing(false); 
		// unlock and sleep so the threads have a chance to stop.
		thread_object.unlock();
		sleepuntilstop();
		thread_object.lock();
	}

	thread_object.abort_processing(false);

	// unlock and let the thread abort
	thread_object.unlock();
	sleepuntilstop();
	thread_object.lock();

	if ( thread_object.is_running(false) ) {
		// unlock and maybe kill
		thread_object.unlock();
		if (kill) {

#ifdef PTHREAD_CANCEL_ENABLE
			for (unsigned i = 0; i < pthreads.size(); i++) 
				pthread_cancel( pthreads[i] );

			sleepuntilstop();
#endif

			for (unsigned i = 0; i < pthreads.size(); i++) 
				pthread_kill(pthreads[i], 9);

			ILog("Threads", pthreads.size() << " thread(s) "
				<< thread_param.name << " killed");
		} else {
			ILog("Threads", pthreads.size() << " thread(s) "
				<< thread_param.name << " refused to abort");

			throw ThreadError(ThreadError::ERROR_STILL_RUNNING);
		}

	} else {
		thread_object.unlock();
		ILog("Threads", pthreads.size() << " thread(s) "
			<< thread_param.name << " have terminated");
	}
}


//@}

} // namespace protlib

#endif // PROTLIB__THREADS_H
