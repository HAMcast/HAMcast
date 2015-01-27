#ifndef TRANSPORT_PROTOCOL_HPP_
#define TRANSPORT_PROTOCOL_HPP_

#include "ariba/utility/addressing/addressing.hpp"
#include "transport_listener.hpp"

// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class transport_protocol {
public:
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void send( const address_v* remote, const uint8_t* data, uint32_t size ) = 0;
	virtual void send( const endpoint_set& endpoints, const uint8_t* data, uint32_t size ) = 0;
	virtual void terminate( const address_v* remote ) = 0;
	virtual void register_listener( transport_listener* listener ) = 0;
};

}} // namespace ariba::transport

#endif /* TRANSPORT_PROTOCOL_HPP_ */
