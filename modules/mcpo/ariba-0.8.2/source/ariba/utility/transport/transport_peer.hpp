#ifndef TRANSPORT_PEER_HPP_
#define TRANSPORT_PEER_HPP_

#include "ariba/config.h"
#include "ariba/utility/logging/Logging.h"
#include "transport_protocol.hpp"
#include "ariba/utility/addressing/endpoint_set.hpp"
#include <boost/shared_ptr.hpp>
#include "rfcomm/bluetooth_rfcomm.hpp"


// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;

class tcpip;

#ifdef HAVE_LIBBLUETOOTH
class rfcomm_transport;
#endif

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// this transport peer allocates implementations of various transport
/// protocols and can send messages to an entire set of endpoints
class transport_peer : public transport_protocol {
	use_logging_h(transport_peer);
public:
	transport_peer( endpoint_set& local_set );
	virtual ~transport_peer();
	virtual void start();
	virtual void stop();
	
	virtual void send(
	        const endpoint_set& endpoints,
	        reboost::message_t message,
	        uint8_t priority = 0);
	
	/// @deprecated: Use terminate() from transport_connection instead
	virtual void terminate( const address_v* remote );
	
	virtual void register_listener( transport_listener* listener );

private:
	void create_service(tcp::endpoint endp);
#ifdef HAVE_LIBBLUETOOTH
	void create_service(boost::asio::bluetooth::rfcomm::endpoint endp);
#endif
	
	endpoint_set&  local;
	std::vector< boost::shared_ptr<tcpip> > tcps;
#ifdef HAVE_LIBBLUETOOTH
	std::vector< boost::shared_ptr<rfcomm_transport> > rfcomms;
#endif
};

}} // namespace ariba::transport

#endif /* TRANSPORT_PEER_HPP_ */
