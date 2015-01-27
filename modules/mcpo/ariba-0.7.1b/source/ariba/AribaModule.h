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

#ifndef ARIBAMODULE_H_
#define ARIBAMODULE_H_

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>

using std::vector;
using std::string;

// forward declaration
namespace ariba {
	class AribaModule;
	class SideportListener;
}

// local includes
#include "Name.h"
#include "Module.h"

namespace ariba {

// forward declaration
namespace communication {
	class EndpointDescriptor;
	class BaseCommunication;
}

/** \addtogroup public
 * @{
 * This class implements a container class for ariba base services. Each node
 * is a running using this base-module. It also manages Bootstrap information
 * in a abstract simple way.
 *
 *        +---+   +---+
 *        |N1 |   |N2 |
 *     +--|   |---|   |--+
 *     |  +---+   +---+  |
 *     |                 |
 *     |     AribaModule |
 *     +-----------------+
 *
 * N1, N2 are nodes.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 * @author Christoph Mayer <mayer@tm.uka.de>
 */
class AribaModule: public Module {
	friend class Node;
	use_logging_h(AribaModule);
public:
	/**
	 * Constructor of the ariba underlay module
	 */
	AribaModule();

	/**
	 * Destructor of the ariba underlay module
	 */
	virtual ~AribaModule();

	/**
	 * Returns all known bootstrap endpoints to this ariba module in
	 * a human-readable string. This information can be used by other
	 * nodes for bootstraping. It may also be used to publish this info
	 * to other nodes via the web, for example.
	 *
	 * @param The name of the spovnet
	 * @return A human-readable string containing all known bootstrap
	 *   information known to this module.
	 */
	string getBootstrapHints(const Name& spoVNetName = Name::UNSPECIFIED) const;

	/**
	 * Adds bootstrap hints to the local database. The format of the string
	 * must is the same as returned by <code>getBootstrapInfo</code>.
	 *
	 * @param bootinfo A string containing bootstrap information.
	 */
	void addBootstrapHints(string bootinfo);

	/**
	 * Register a sideport for sniffing on communication events
	 * and get advanced information. The sniffer is attached to
	 * every node that is created on the module. Only one such
	 * sniffer can be active system-wide, a new call to this
	 * register function will only attach the sniffer to nodes
	 * created after the registration call.
	 *
	 * @param sideport The SideportListener to integrate
	 */
	void registerSideportListener(SideportListener* sideport);

	// --- module implementation ---

	/**
	 * Module Property information:
	 *
	 * ip.addr  = preferred ip address (otherwise bind to all)
	 * tcp.port = preferred tcp port (or use default value)
	 * udp.port = preferred udp port (or use default value)
	 * bootstrap.hints = used bootstrap hints
	 * bootstrap.file  = used file for bootstrap information
	 */
	void initialize();                          ///< @see Module.h
	void start();                               ///< @see Module.h
	void stop();                                ///< @see Module.h
	string getName() const;                     ///< @see Module.h
	void setProperty(string key, string value); ///< @see Module.h
	const string getProperty(string key) const; ///< @see Module.h
	const vector<string> getProperties() const; ///< @see Module.h

	/**
	 * Get the local endpoint information
	 */
	const string getLocalEndpoints();

private:

	/**
	 * Available bootstrap mechanisms
	 */
	enum BootstrapMechanism {
		BootstrapMechanismInvalid = 0,
		BootstrapMechanismStatic = 1,
		BootstrapMechanismBroadcast = 2,
		BootstrapMechanismMulticastDNS = 3,
		BootstrapMechanismSDP = 4,
	};
	static const string BootstrapMechanismNames[5];

	/**
	 * bootstrap node information
	 */
	class BootstrapNode {
	public:
		inline BootstrapNode() :
			timestamp(0), desc(NULL), mechanism(BootstrapMechanismInvalid), info("") {

		}
		inline BootstrapNode(const BootstrapNode& copy) :
			timestamp(copy.timestamp), desc(copy.desc), mechanism(copy.mechanism), info(copy.info) {
		}
		inline BootstrapNode(
				uint32_t timestamp,
				communication::EndpointDescriptor* desc,
				BootstrapMechanism mechanism, string info) :
			timestamp(timestamp), desc(desc), mechanism(mechanism), info(info) {
		}
		uint32_t timestamp;
		communication::EndpointDescriptor* desc;
		BootstrapMechanism mechanism;
		string info;
	};

	/*
	 * bootstrap info, all bootstrap nodes
	 * for a specific spovnet
	 */
	class BootstrapInfo {
	public:
		BootstrapInfo() :
			spovnetName(), nodes() {
		}

		BootstrapInfo(const BootstrapInfo& copy) :
			spovnetName(copy.spovnetName), nodes(copy.nodes) {
		}

		Name spovnetName;
		vector<BootstrapNode> nodes;
	};

	vector<BootstrapInfo> bootstrapNodes; //< all available bootstrap information

protected:
	// members
	string endpoints; //< local endpoints the ariba module is bound to
	bool started; //< flag, if module has been started

	/**
	 * bootstrap node management
	 * add a bootstrap node
	 */
	void addBootstrapNode(
			const Name& spovnet,
			communication::EndpointDescriptor* desc,
			const string& info,
			const BootstrapMechanism& mechanism
			);

	/**
	 * bootstrap node management
	 * add a bootstrap node
	 */
	void addBootstrapNode(
			const Name& spovnet,
			const BootstrapNode& node
			);

	/**
	 * bootstrap node management
	 * get all available bootstrap mechanisms
	 * where bootstrap nodes are available for
	 */
	vector<AribaModule::BootstrapMechanism> getBootstrapMechanisms(
			const Name& spovnet
			) const;

	/**
	 * get a endpoint descriptor for a spovnet
	 * using a specific bootstrap mechanisms.
	 * will currently only work with static
	 */
	const communication::EndpointDescriptor* getBootstrapNode(
			const Name& spovnet,
			const BootstrapMechanism mechanism
			) const;

	/**
	 * get the info field associated for a given
	 * spovnet through a given mechanism
	 */
	string getBootstrapInfo(
			const Name& spovnet,
			const BootstrapMechanism mechanism
			) const;

	communication::BaseCommunication* base_comm; //< the base communication
	SideportListener* sideport_sniffer; //< the sideport listener
};

} // namespace ariba

/** @} */

#endif /* ENVIRONMENT_H_ */
