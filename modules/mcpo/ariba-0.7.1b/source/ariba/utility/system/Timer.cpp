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

#include "Timer.h"

namespace ariba {
namespace utility {

use_logging_cpp(Timer);
SystemEventType TimerEventType("Timer");

Timer::Timer() {
	running = false;

#ifndef UNDERLAY_OMNET
	timerThread = NULL;
#endif // UNDERLAY_OMNET
}

Timer::~Timer() {
#ifndef UNDERLAY_OMNET
	stop();
	if(timerThread != NULL){
		delete timerThread;
		timerThread = NULL;
	}
#endif // UNDERLAY_OMNET
}

void Timer::setInterval(unsigned int millis, bool oneshot) {
	this->millis = millis;
	this->oneshot = oneshot;
}

void Timer::start() {
	if( running ) return;

#ifndef UNDERLAY_OMNET
	// running var will be set in the thread function
	if( timerThread == NULL )
		timerThread = new boost::thread(boost::bind(&Timer::threadFunc, this) );
#else
	running = true;
 	SystemQueue::instance().scheduleEvent(
		SystemEvent( this, TimerEventType, NULL), millis );
#endif
}

void Timer::reset(){
#ifndef UNDERLAY_OMNET
	if(timerThread == NULL) return;
	timerThread->interrupt();
#else
	#error timer interruption not implemented for omnet
#endif
}

bool Timer::isRunning(){
	return running;
}

void Timer::stop() {
	running = false;
	reset(); // cause the sleep to abort
	SystemQueue::instance().dropAll(this);
	if(timerThread != NULL)
		timerThread->join();
}

void Timer::eventFunction() {
	//std::cout << "unimplemented eventFunction Timer(" << millis << ")" << std::endl;
}

#ifndef UNDERLAY_OMNET
void Timer::threadFunc( Timer* obj ) {
	obj->running = true;

	while( obj->running ) {

		try{

			boost::this_thread::sleep( boost::posix_time::milliseconds(obj->millis) );
			if (obj->running)
				SystemQueue::instance().scheduleEvent( SystemEvent( obj, TimerEventType, NULL), 0 );

		}catch(boost::thread_interrupted e){
			// exception called when Timer::reset is called
			// don't need to handle the exception
		}

		if( obj->oneshot ) break;
	}

	if(! obj->oneshot )
		obj->running = false;
}
#endif // UNDERLAY_OMNET

void Timer::handleSystemEvent( const SystemEvent& event ) {
	if( running ){
		if( oneshot ) running = false;
		eventFunction();
	}

#ifdef UNDERLAY_OMNET
	if( ! oneshot && running )
		SystemQueue::instance().scheduleEvent(
			SystemEvent( this, TimerEventType, NULL), millis );
#endif
}

}} // namespace ariba, common
