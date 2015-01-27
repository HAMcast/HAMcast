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

#ifndef __PERIODIC_BROADCAST_MESSAGE_H
#define __PERIODIC_BROADCAST_MESSAGE_H

#include <string>
#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"

using std::string;
using ariba::utility::Message;

namespace ariba {
namespace utility {

using_serialization;

class PeriodicBroadcastMessage : public Message {
	VSERIALIZEABLE;
private:
	string name;
	string info1;
	string info2;
	string info3;

public:
	PeriodicBroadcastMessage();
	virtual ~PeriodicBroadcastMessage();

	void setName(string _name);
	void setInfo1(string _info1);
	void setInfo2(string _info2);
	void setInfo3(string _info3);

	string getName();
	string getInfo1();
	string getInfo2();
	string getInfo3();
};

}} // ariba::utility

sznBeginDefault( ariba::utility::PeriodicBroadcastMessage, X ) {
	X && T(name) && T(info1) && T(info2) && T(info3);
} sznEnd();

#endif // __PERIODIC_BROADCAST_MESSAGE_H
