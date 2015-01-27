// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
// Zirkel 2, 76128 Karlsruhe
// Germany
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the Institute of
// Telematics.
// [License]

#ifndef SYSTEMQUEUE_H_
#define SYSTEMQUEUE_H_

#include <vector>
#include <cassert>
#include "SystemEvent.h"
#include "SystemEventListener.h"
#include "ariba/utility/logging/Logging.h"
#include <boost/date_time.hpp>
#include <boost/cstdint.hpp>

#ifdef UNDERLAY_OMNET
  #include <csimplemodule.h>
  #include <cmessage.h>
  #include <macros.h>
#else
  #include <boost/thread/mutex.hpp>
  #include <boost/thread/thread.hpp>
  #include <boost/thread/condition_variable.hpp>
  #include <boost/utility.hpp>
  #include <boost/bind.hpp>
#endif

using std::vector;
using boost::posix_time::ptime;

namespace ariba {
namespace utility {

/**
 * This class implements a simple system event queue to allow
 * a simulation of cooperative multitasking. It also allows
 * events to be scheduled from other tasks. This allows
 * dispatching asynchronous tasks.
 *
 * @author Christoph Mayer, Sebastian Mies
 */

#ifndef UNDERLAY_OMNET
class SystemQueue : private boost::noncopyable {
#else
class SystemQueue : public cSimpleModule {
#endif
	
	use_logging_h(SystemQueue);
	friend class EnterMethod;
public:
	/**
	 * Get the SystemQueue singleton instance.
	 */
	static SystemQueue& instance() {
		static SystemQueue _inst;
		return _inst;
	}

#ifdef UNDERLAY_OMNET
	/**
	 * Prevent deletion of this module
	 * by implementing the virtual method
	 * and doing nothing in it
	 */
	virtual void deleteModule(){}
#endif

	/**
	 * This methods schedules a given event.
	 *
	 * @param The event to be scheduled
	 * @param The delay in milli-seconds
	 */
	void scheduleEvent( const SystemEvent& event, uint32_t delay = 0 );

	/**
	 * Starts the processing and waiting for events.
	 * Use <code>cancel()</code> to end system queue processing and
	 * <code>isEmpty()</code> to check wheter the queue is empty.
	 */
	void run();

	/**
	 * Cancels the system queue and ends the processing after the
	 * currently processed event is processed.
	 *
	 * This method is thread-safe.
	 */
	void cancel();

	/**
	 * Drop all queued events for that listener
	 */
	void dropAll( const SystemEventListener* mlistener);

	/**
	 * Check wheter this queue has items or not.
	 *
	 * @return True, if this queue is empty.
	 */
	bool isEmpty();

	/**
	 * Is the system queue already started and running?
	 *
	 * @return True, if the system queue is running.
	 */
	bool isRunning();

protected:

	/**
	 * Aqcuire the mutex
	 */
	void enterMethod();

	/**
	 * Leave the mutex
	 */
	void leaveMethod();

	/**
	 * Constructs a system queue.
	 */
	SystemQueue();

	/**
	 * Destroys the system queue. Beware that all events
	 * are canceled
	 */
	~SystemQueue();

#ifdef UNDERLAY_OMNET
	virtual void handleMessage( cMessage* msg );
#endif

private:

#ifndef UNDERLAY_OMNET
	typedef vector<SystemEvent> EventQueue;

	//********************************************************

	class QueueThread {
	public:
		QueueThread(QueueThread* _transferQueue = NULL);
		virtual ~QueueThread();
		void run();
		void cancel();
		bool isEmpty();
		void insert( const SystemEvent& event, uint32_t delay );
		void enter();
		void leave();
		void dropAll( const SystemEventListener* mlistener);

	protected:
		virtual void onItemInserted( const SystemEvent& event ) = 0;
		virtual void onNextQueueItem( const SystemEvent& event ) = 0;
		QueueThread* transferQueue;
		EventQueue eventsQueue;
		boost::mutex queueMutex;
	private:
		boost::thread* queueThread;
		static void threadFunc( QueueThread* obj );
		boost::condition_variable itemsAvailable;
		volatile bool running;
	}; // class QueueThread

	//********************************************************

	class QueueThreadDirect : public QueueThread {
	public:
		QueueThreadDirect();
		~QueueThreadDirect();
	protected:
		virtual void onItemInserted( const SystemEvent& event );
		virtual void onNextQueueItem( const SystemEvent& event );
	}; // class QueueThreadDirect

	//********************************************************

	class QueueThreadDelay : public QueueThread {
	public:
		QueueThreadDelay(QueueThread* _transferQueue = NULL);
		~QueueThreadDelay();
	protected:
		virtual void onItemInserted( const SystemEvent& event );
		virtual void onNextQueueItem( const SystemEvent& event );
	private:
		volatile bool isSleeping;
		ptime sleepStart;
		boost::mutex sleepMutex;
		boost::condition_variable sleepCond;
	}; // class QueueThreadDelay

	//********************************************************

	QueueThreadDirect directScheduler;
	QueueThreadDelay delayScheduler;
	volatile bool systemQueueRunning;
#endif

}; // class SystemQueue

#ifdef UNDERLAY_OMNET

#if 0
	//
	// the system queue must be a singleton in simulations, too.
	// and to include it in the simulation the module is defined
	// as submodule in every SpoVNet host. Therefore we hack the
	// Define_Module (see omnet/includes/macros.h) the way we need
	// it with our singleton ...
	//
	// this is the macro definition from macros.h
	//
	// #define Define_Module(CLASSNAME) /backslash
	//   static cModule *CLASSNAME##__create() {return new CLASSNAME();} /backslash
	//   EXECUTE_ON_STARTUP(CLASSNAME##__mod, modtypes.instance()->add(new cModuleType(#CLASSNAME,#CLASSNAME,(ModuleCreateFunc)CLASSNAME##__create));)
	//
	// and this is how we do it :)
	//
#endif

  	static cModule* SystemQueue__create() {
		return &SystemQueue::instance();
	}

 	EXECUTE_ON_STARTUP(SystemQueue__mod, modtypes.instance()->add(new cModuleType("SystemQueue","SystemQueue",(ModuleCreateFunc)SystemQueue__create));)

#endif

}} // spovnet, common

#endif /* SYSTEMQUEUE_H_ */
