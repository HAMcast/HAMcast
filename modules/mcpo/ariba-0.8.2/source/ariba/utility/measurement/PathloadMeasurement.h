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

#ifndef __PATHLOAD_MEASUREMENT_H
#define __PATHLOAD_MEASUREMENT_H

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <boost/utility.hpp>
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/types/NodeID.h"
#include "ariba/utility/system/BlockingMethod.h"
#include "ariba/communication/EndpointDescriptor.h"
#include "ariba/overlay/BaseOverlay.h"

using ariba::utility::NodeID;
using ariba::utility::BlockingMethod;
using ariba::communication::EndpointDescriptor;
using ariba::overlay::BaseOverlay;

namespace ariba {
namespace utility {

//*************************************************

class PathloadMeasurementListener {
	friend class PathloadMeasurement;
protected:
	// mbps hols the MBit/s as floating point and -1 if the
	// measurement was triggered by two nodes on our local machines
	virtual void onMeasurement( NodeID node, double mbps ) = 0;
};

//*************************************************

class PathloadMeasurement : private boost::noncopyable, public BlockingMethod {
	use_logging_h( PathloadMeasurement );
private:
	volatile bool running;
	PathloadMeasurementListener* listener;

	double resultMbps;
	NodeID resultNode;
	BaseOverlay* baseoverlay;
	pid_t serverpid;

protected:
	PathloadMeasurement(BaseOverlay* _overlay);
	virtual ~PathloadMeasurement();

	virtual void dispatchFunction();
	virtual void blockingFunction();

public:
	static PathloadMeasurement& instance(BaseOverlay* _overlay = NULL) {
		static PathloadMeasurement the_inst( _overlay );
		return the_inst;
	}

	void measure( const NodeID& destnode,  PathloadMeasurementListener* _listener );
};

//*************************************************

}} // namespace ariba, utility

#endif // __PATHLOAD_MEASUREMENT_H
