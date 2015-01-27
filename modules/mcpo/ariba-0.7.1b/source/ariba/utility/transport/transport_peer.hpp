#ifndef TRANSPORT_PEER_HPP_
#define TRANSPORT_PEER_HPP_

#include "ariba/config.h"
#include "transport_protocol.hpp"
#include "ariba/utility/addressing/endpoint_set.hpp"

// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;

class tcpip;
#ifdef HAVE_LIBBLUETOOTH
class rfcomm;
#endif

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// this transport peer allocates implementations of various transport
/// protocols and can send messages to an entire set of endpoints
class transport_peer : public transport_protocol {
public:
	transport_peer( endpoint_set& local_set );
	virtual ~transport_peer();
	virtual void start();
	virtual void stop();
	virtual void send( const address_v* remote, const uint8_t* data, uint32_t size );
	virtual void send( const endpoint_set& endpoints, const uint8_t* data, uint32_t size );
	virtual void terminate( const address_v* remote );
	virtual void register_listener( transport_listener* listener );

private:
	endpoint_set&  local;
	tcpip* tcp;
#ifdef HAVE_LIBBLUETOOTH
	rfcomm* rfc;
#endif
};

}} // namespace ariba::transport

#endif /* TRANSPORT_PEER_HPP_ */
