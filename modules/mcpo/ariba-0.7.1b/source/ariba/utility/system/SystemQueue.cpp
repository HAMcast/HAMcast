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

#include "SystemQueue.h"

namespace ariba {
namespace utility {

use_logging_cpp(SystemQueue);

SystemQueue::SystemQueue()
#ifndef UNDERLAY_OMNET
 : delayScheduler( &directScheduler ), systemQueueRunning( false )
#endif
{
}

SystemQueue::~SystemQueue() {
}

void SystemQueue::scheduleEvent( const SystemEvent& event, uint32_t delay ) {
#ifndef UNDERLAY_OMNET
	if( delay == 0 ) directScheduler.insert( event, delay );
	else delayScheduler.insert ( event, delay );
#else
	Enter_Method_Silent();
	cMessage* msg = new cMessage();
	msg->setContextPointer( new SystemEvent(event) );

	if( delay == 0 )
		cSimpleModule::scheduleAt( cSimpleModule::simTime(), msg );
	else
		cSimpleModule::scheduleAt( cSimpleModule::simTime()+((double)delay/1000.0), msg );
#endif
}

#ifdef UNDERLAY_OMNET
void SystemQueue::handleMessage(cMessage* msg){
	SystemEvent* event = (SystemEvent*)msg->contextPointer();

	event->listener->handleSystemEvent( *event );

	delete event; delete msg;
}
#endif

void SystemQueue::run() {
#ifndef UNDERLAY_OMNET
	systemQueueRunning = true;
	directScheduler.run();
	delayScheduler.run();
#endif
}

void SystemQueue::cancel() {
#ifndef UNDERLAY_OMNET
	systemQueueRunning = false;
	directScheduler.cancel();
	delayScheduler.cancel();
#endif
}

void SystemQueue::dropAll( const SystemEventListener* mlistener){
#ifndef UNDERLAY_OMNET
	directScheduler.dropAll(mlistener);
	delayScheduler.dropAll(mlistener);
#endif
}

bool SystemQueue::isEmpty() {
#ifndef UNDERLAY_OMNET
	return directScheduler.isEmpty() || delayScheduler.isEmpty();
#else
	return false;
#endif
}

bool SystemQueue::isRunning() {
#ifndef UNDERLAY_OMNET
	return systemQueueRunning;
#else
	return true;
#endif
}

void SystemQueue::enterMethod(){
	// TODO: omnet case and delay scheduler
	directScheduler.enter();
}

void SystemQueue::leaveMethod(){
	// TODO: omnet case and delay scheduler
	directScheduler.leave();
}

//***************************************************************
#ifndef UNDERLAY_OMNET

SystemQueue::QueueThread::QueueThread(QueueThread* _transferQueue)
	: transferQueue( _transferQueue ), running( false ) {
}

SystemQueue::QueueThread::~QueueThread(){
}

void SystemQueue::QueueThread::run(){
	running = true;

	queueThread = new boost::thread(
		boost::bind(&QueueThread::threadFunc, this) );
}

void SystemQueue::QueueThread::cancel(){

	logging_debug("cancelling system queue");

	// cause the thread to exit
	{
		// get the lock, when we got the lock the
		// queue thread must be in itemsAvailable.wait()
		boost::mutex::scoped_lock lock(queueMutex);

		// set the running indicator and signal to run on
		// this will run the thread and quit it
		running = false;
		itemsAvailable.notify_all();
	}

	// wait until the thread has exited
	logging_debug("joining system queue thread");
	queueThread->join();

	// delete pending events
	logging_debug("deleting pending system queue events");
	while( eventsQueue.size() > 0 ){
		eventsQueue.erase( eventsQueue.begin() );
	}

	// delete the thread, so that a subsuquent run() can be called
	delete queueThread;
	queueThread = NULL;
}

bool SystemQueue::QueueThread::isEmpty(){
	boost::mutex::scoped_lock lock( queueMutex );
	return eventsQueue.empty();
}

void SystemQueue::QueueThread::insert( const SystemEvent& event, uint32_t delay ){

	// if this is called from a module that is currently handling
	// a thread (called from SystemQueue::onNextQueueItem), the
	// thread is the same anyway and the mutex will be already
	// aquired, otherwise we aquire it now

	boost::mutex::scoped_lock lock( queueMutex );

	eventsQueue.push_back( event );
	eventsQueue.back().scheduledTime = boost::posix_time::microsec_clock::local_time();
	eventsQueue.back().delayTime = delay;
	eventsQueue.back().remainingDelay = delay;

	onItemInserted( event );
	itemsAvailable.notify_all();
}

void SystemQueue::QueueThread::dropAll( const SystemEventListener* mlistener) {
	boost::mutex::scoped_lock lock( queueMutex );

	bool deleted;
	do{
		deleted = false;
		EventQueue::iterator i = eventsQueue.begin();
		EventQueue::iterator iend = eventsQueue.end();

		for( ; i != iend; i++){
			if((*i).getListener() == mlistener){
				eventsQueue.erase(i);
				deleted = true;
				break;
			}
		}
	}while(deleted);
}

void SystemQueue::QueueThread::threadFunc( QueueThread* obj ) {

	boost::mutex::scoped_lock lock( obj->queueMutex );

	while( obj->running ) {

		// wait until an item is in the queue or we are notified
		// to quit the thread. in case the thread is about to
		// quit, the queueThreadRunning variable will indicate
		// this and cause the thread to exit

		while ( obj->running && obj->eventsQueue.empty() ){

//			const boost::system_time duration =
//					boost::get_system_time() +
//					boost::posix_time::milliseconds(100);
//			obj->itemsAvailable.timed_wait( lock, duration );

			obj->itemsAvailable.wait( lock );
		}

		//
		// work all the items that are currently in the queue
		//

		while( obj->running && (!obj->eventsQueue.empty()) ) {

			// fetch the first item in the queue
			// and deliver it to the queue handler
			SystemEvent ev = obj->eventsQueue.front();
			obj->eventsQueue.erase( obj->eventsQueue.begin() );

			// call the queue and this will
			// call the actual event handler
			obj->queueMutex.unlock();
			obj->onNextQueueItem( ev );
			obj->queueMutex.lock();

		} // !obj->eventsQueue.empty() )
	} // while (obj->running)

	logging_debug("system queue exited");
}

void SystemQueue::QueueThread::enter(){
	queueMutex.lock();
}

void SystemQueue::QueueThread::leave(){
	queueMutex.unlock();
}


//***************************************************************

SystemQueue::QueueThreadDirect::QueueThreadDirect(){
}

SystemQueue::QueueThreadDirect::~QueueThreadDirect(){
}

void SystemQueue::QueueThreadDirect::onItemInserted( const SystemEvent& event ){
	// do nothing here
}

void SystemQueue::QueueThreadDirect::onNextQueueItem( const SystemEvent& event ){
	// directly deliver the item to the
	event.getListener()->handleSystemEvent( event );
}

//***************************************************************

SystemQueue::QueueThreadDelay::QueueThreadDelay(QueueThread* _transferQueue)
	: QueueThread( _transferQueue ), isSleeping( false ) {

	assert( _transferQueue != NULL );
}

SystemQueue::QueueThreadDelay::~QueueThreadDelay(){
}

void SystemQueue::QueueThreadDelay::onItemInserted( const SystemEvent& event ){

	if( !isSleeping) return;

	// break an existing sleep and
	// remember the time that was actually slept for
	// and change it for every event in the queue

	assert( !eventsQueue.empty());
	sleepCond.notify_all();

	ptime sleepEnd = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration duration = sleepEnd - sleepStart;
	uint32_t sleptTime = duration.total_milliseconds();

	EventQueue::iterator i = eventsQueue.begin();
	EventQueue::iterator iend = eventsQueue.end();

	for( ; i != iend; i++ ) {

		if( sleptTime >= i->remainingDelay)
			i->remainingDelay = 0;
		else
			i->remainingDelay -= sleptTime;

	} // for( ; i != iend; i++ )

	// now we have to reorder the events
	// in the queue with respect to their remaining delay
	// the SystemQueue::operator< takes care of the
	// ordering with respect to the remaining delay

	std::sort( eventsQueue.begin(), eventsQueue.end() );

}

void SystemQueue::QueueThreadDelay::onNextQueueItem( const SystemEvent& event ){

	// sleeps will be cancelled in the
	// onItemInserted function when a new
	// event arrives during sleeping

	assert( !isSleeping );

	// the given item is the one with the least
	// amount of sleep time left. because all
	// items are reordered in onItemInserted

	if( event.remainingDelay > 0 ) {

		const boost::system_time duration =
			boost::get_system_time() +
			boost::posix_time::milliseconds(event.remainingDelay);

		{
			boost::unique_lock<boost::mutex> lock( sleepMutex );

			sleepStart = boost::posix_time::microsec_clock::local_time();
			isSleeping = true;

			sleepCond.timed_wait( lock, duration );

			isSleeping = false;
		}

	} // if( event.remainingDelay > 0 )

	// if the sleep succeeded and was not
	// interrupted by a new incoming item
	// we can now deliver this event

	ptime sleepEnd = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration duration = sleepEnd - sleepStart;
	uint32_t sleptTime = duration.total_milliseconds();

	if (event.remainingDelay <= sleptTime)
		transferQueue->insert( event, 0 );
}

#endif // #ifndef UNDERLAY_OMNET

//***************************************************************

}} // spovnet, common
