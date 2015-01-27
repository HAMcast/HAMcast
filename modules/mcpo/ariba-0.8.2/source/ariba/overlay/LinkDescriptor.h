#ifndef __LINK_DESCRIPTOR_H
#define __LINK_DESCRIPTOR_H

#include <iostream>
#include <sstream>
#include <ctime>
#include <deque>
#include <boost/foreach.hpp>

#include "ariba/utility/messages.h"
#include "ariba/utility/types.h"
#include "ariba/communication/EndpointDescriptor.h"
#include "ariba/CommunicationListener.h"

namespace ariba {
	class CommunicationListener;
}

using std::deque;
using ariba::utility::Message;
using ariba::utility::NodeID;
using ariba::utility::SpoVNetID;
using ariba::utility::ServiceID;
using ariba::utility::LinkID;
using ariba::CommunicationListener;
using ariba::communication::EndpointDescriptor;

namespace ariba {
namespace overlay {

class LinkDescriptor;

std::ostream& operator<<(std::ostream& s, const LinkDescriptor* ld );
std::ostream& operator<<(std::ostream& s, const LinkDescriptor& ld );

/// LinkDescriptor
class LinkDescriptor {
public:
	// ctor
	LinkDescriptor() {
		// default values
		this->up = false;
		this->fromRemote = false;
		this->remoteNode = NodeID::UNSPECIFIED;
		this->overlayId  = LinkID::create();
		this->communicationUp = false;
		this->communicationId = LinkID::UNSPECIFIED;
		this->keepAliveTime = time(NULL);
		this->keepAliveMissed = 0;
		this->relaying     = false;
		this->timeRelaying = time(NULL);
		this->dropAfterRelaying = false;
		this->service  = ServiceID::UNSPECIFIED;
		this->listener = &CommunicationListener::DEFAULT;
		this->relayed = false;
		this->remoteLink = LinkID::UNSPECIFIED;
		this->autolink = false;
		this->lastuse = time(NULL);
		this->retryCounter = 0;
	}

	// dtor
	~LinkDescriptor() {
		flushQueue();
	}

	// general information about the link --------------------------------------
	bool up;           ///< flag whether this link is up and running
	bool fromRemote;   ///< flag, whether this link was requested from remote
	NodeID remoteNode; ///< remote end-point node
	bool isVital() {
		return up && keepAliveMissed == 0;
	}
	bool isDirectVital() {
		return isVital() && communicationUp && !relayed;
	}


	// link identifiers --------------------------------------------------------
	LinkID overlayId;       ///< the base overlay link id
	LinkID communicationId; ///< the communication id
	bool   communicationUp;   ///< flag, whether the communication is up

	// direct link retries -----------------------------------------------------
	EndpointDescriptor endpoint;
	int retryCounter;

	// link alive information --------------------------------------------------
	time_t keepAliveTime; ///< the last time a keep-alive message was received
	int keepAliveMissed;  ///< the number of missed keep-alive messages
	void setAlive() {
		keepAliveMissed = 0;
		keepAliveTime = time(NULL);
	}

	// relay information -------------------------------------------------------
	bool   relayed;    ///< flag whether this link is a relayed link
	LinkID remoteLink; ///< the remote link id
	vector<NodeID> routeRecord;

	// relay state -------------------------------------------------------------
	bool   relaying;     ///< flag, wheter this link has been used as relay
	bool   dropAfterRelaying;
	time_t timeRelaying; ///< last time the link has been used as relay
	void setRelaying() {
		relaying = true;
		timeRelaying = time(NULL);
	}

	// owner -------------------------------------------------------------------
	ServiceID service; ///< service using this link
	CommunicationListener* listener; ///< the listener using this node

	// auto links --------------------------------------------------------------
	bool autolink;  ///< flag, whether this link is a auto-link
	time_t lastuse; ///< time, when the link was last used
	deque<Message*> messageQueue; ///< waiting messages to be delivered
	void setAutoUsed() {
		if (autolink) lastuse = time(NULL);
	}
	/// drops waiting auto-link messages
	void flushQueue() {
		BOOST_FOREACH( Message* msg, messageQueue )	delete msg;
		messageQueue.clear();
	}

	// string representation ---------------------------------------------------
	std::string to_string() const {
		std::ostringstream s;
		s << "up=" << up << " ";
		s << "init=" << !fromRemote << " ";
		s << "id=" << overlayId.toString().substr(0,4) << " ";
		s << "serv=" << service.toString() << " ";
		s << "node=" << remoteNode.toString().substr(0,4) << " ";
		s << "relaying=" << relaying << " ";
		s << "miss=" << keepAliveMissed << " ";
		s << "auto=" << autolink << " ";
		if ( relayed ) {
			s << "| Relayed: ";
			s << "remote link=" << remoteLink.toString().substr(0,4) << " ";
			if (routeRecord.size()>0) {
				s << "route record=";
				for (size_t i=0; i<routeRecord.size(); i++)
					s << routeRecord[i].toString().substr(0,4) << " ";
			}
		} else {
			s << "| Direct: ";
			s << "using id=" << communicationId.toString().substr(0,4) << " ";
			s << "(up=" << communicationUp << ") ";
		}
		return s.str();
	}
};

}} // namespace ariba, overlay

#endif // __LINK_DESCRIPTOR_H

