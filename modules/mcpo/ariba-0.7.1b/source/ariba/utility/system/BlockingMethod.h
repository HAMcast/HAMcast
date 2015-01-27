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

#ifndef BLOCKING_METHOD_H__
#define BLOCKING_METHOD_H__

#include "ariba/utility/system/SystemEventListener.h"
#include "ariba/utility/system/SystemQueue.h"
#include "ariba/utility/system/SystemEvent.h"
#include "ariba/utility/system/SystemEventType.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using ariba::utility::SystemEventType;
using ariba::utility::SystemQueue;
using ariba::utility::SystemEventListener;

namespace ariba {
namespace utility {

class BlockingMethod : public SystemEventListener {
public:
	BlockingMethod();
	virtual ~BlockingMethod();

	// start the blocking method
	void runBlockingMethod();

	// block until thread ended
	void joinThread();

protected:

	// call this from blockingFunction when you want
	// to join the main thread in sync again.and the blocking
	// function is done
	void dispatch();
	void handleSystemEvent(const SystemEvent& event);
	static void threadFunc(BlockingMethod* obj);

	// async called when dispatch() is triggered
	// and you are back in sync with the main thread
	virtual void dispatchFunction() = 0;

	// implement with your blocking code
	virtual void blockingFunction() = 0;


private:
	boost::thread* threadObj;

};

}} // namespace ariba, common

#endif // BLOCKING_METHOD_H__
