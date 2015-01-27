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

#ifndef SYSTEMEVENT_H_
#define SYSTEMEVENT_H_

#include <boost/date_time.hpp>
#include <iostream>
#include "SystemEventType.h"

using boost::posix_time::ptime;

namespace ariba {
namespace utility {

class SystemQueue;
class SystemEventListener;

class SystemEvent {
	friend class SystemQueue;
private:
	SystemEventListener* listener; //< handler of the event
	SystemEventType type; //< type of the event
	const void* data; //< data attached to the event

public:
	// TODO: these should be private, but the gcc 3.4 in scratchbox for cross-compiling
	// for Maemo (Nokia N810) gets confused when the friend class SystemQueue has a 
	// private class that accesses these variables. Therefore they are public for now.
	ptime scheduledTime; //< time the event was originally given to the queue
	uint32_t delayTime; //< time the event is scheduled at, or 0 if it is to be fired immediately
	uint32_t remainingDelay; //< remaining delay time for sleeping

public:
	inline SystemEvent(SystemEventListener* mlistener, SystemEventType mtype =
			SystemEventType::DEFAULT, void* mdata = NULL) :
		listener(mlistener), type(mtype), data(mdata), scheduledTime(
				boost::posix_time::not_a_date_time), delayTime(0),
				remainingDelay(0)

	{
	}

	template<typename T>
	inline SystemEvent(SystemEventListener* mlistener, SystemEventType mtype =
			SystemEventType::DEFAULT, T* mdata = NULL) :
		listener(mlistener), type(mtype), data((void*) mdata), scheduledTime(
				boost::posix_time::not_a_date_time), delayTime(0),
				remainingDelay(0) {
	}

	inline SystemEvent(const SystemEvent& copy) {
		this->scheduledTime = copy.scheduledTime;
		this->delayTime = copy.delayTime;
		this->remainingDelay = copy.remainingDelay;
		this->listener = copy.listener;
		this->type = copy.type;
		this->data = copy.data;
	}

	inline void operator=(const SystemEvent& right) {
		this->scheduledTime = right.scheduledTime;
		this->delayTime = right.delayTime;
		this->remainingDelay = right.remainingDelay;
		this->listener = right.listener;
		this->type = right.type;
		this->data = right.data;
	}

	inline ~SystemEvent() {
	}

	template<typename T>
	inline operator T() const {
		return (T) (data);
	}

	inline SystemEventListener* getListener() const {
		return listener;
	}

	template<typename T>
	inline T* getData() const {
		return (T*) *this;
	}

	inline const SystemEventType getType() const {
		return type;
	}

	inline const ptime getScheduledTime() const {
		return scheduledTime;
	}

	inline const uint32_t getRemainingDelay() const {
		return remainingDelay;
	}

	inline bool operator<(const SystemEvent& right) const {
		return remainingDelay < right.remainingDelay;
	}

};

}
} // spovnet, common

#endif /* SYSTEMEVENT_H_ */
