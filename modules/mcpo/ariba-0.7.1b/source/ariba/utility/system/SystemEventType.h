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

#ifndef SYSTEMEVENTTYPE_H_
#define SYSTEMEVENTTYPE_H_

#include <iostream>
#include <vector>
#include <string>
#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>

using std::string;
using std::vector;
using std::ostream;

namespace ariba {
namespace utility {

class SystemEventType {
public:
	typedef uint16_t eventid_t;
	static const SystemEventType DEFAULT;

private:
	eventid_t id;

private:
	class Descriptor {
	public:
		string description;
		eventid_t parent;

		Descriptor(string _d, eventid_t _p = 0) :
			description(_d), parent(_p) {
		}
		
		Descriptor(const Descriptor& rh) 
			: description( rh.description ), parent( rh.parent ) {
		}

		~Descriptor() {
		}
	};
	static vector<Descriptor> ids;

	inline SystemEventType(eventid_t mtype) :
		id(mtype) {
	}

public:
	inline SystemEventType(string description,
			const SystemEventType parent = DEFAULT) {
		if (ids.size() == 0) ids.push_back(Descriptor(
				"<Default SystemEvent>", DEFAULT.id));
		id = (eventid_t) ids.size();
		ids.push_back(Descriptor(description, parent.id));
	}

	inline SystemEventType() {
		id = 0;
		if (ids.size() == 0) ids.push_back(Descriptor(
				"<Default SystemEvent>", DEFAULT.id));
	}

	inline SystemEventType(const SystemEventType& evtType) {
		this->id = evtType.id;
	}

	inline ~SystemEventType() {
	}

	inline string getDescription() const {
		return ids[id].description;
	}

	inline const SystemEventType getParent() const {
		return SystemEventType(ids[id].parent);
	}

	inline const int getDepth() const {
		int i = 0;
		SystemEventType type = *this;
		while (type != SystemEventType::DEFAULT) {
			type = type.getParent();
			i++;
		}
		return i;
	}

	inline const eventid_t getId() const {
		return id;
	}

	inline SystemEventType& operator=(const SystemEventType& evtType) {
		this->id = evtType.id;
		return *this;
	}

	inline bool operator==(const SystemEventType& evtType) const {
		return id == evtType.id;
	}

	inline bool operator!=(const SystemEventType& evtType) const {
		return id != evtType.id;
	}

	inline bool operator>(const SystemEventType& evtType) const {
		SystemEventType t = *this;
		while (t.getParent() != evtType && t.getParent() != DEFAULT)
			t = t.getParent();
		return (t.getParent() == evtType && t != evtType);
	}

	inline bool operator<(const SystemEventType& evtType) const {
		SystemEventType t = evtType;
		while (t.getParent() != *this && t.getParent() != DEFAULT)
			t = t.getParent();
		return (t.getParent() == *this && t != *this);
	}

	inline bool isDefault() {
		return ( id == 0 );
	}

};

ostream& operator<<(ostream& stream, SystemEventType type);

}} // spovnet, common

#endif /* SYSTEMEVENTTYPE_H_ */
