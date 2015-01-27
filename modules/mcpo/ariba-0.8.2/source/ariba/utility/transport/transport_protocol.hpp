#ifndef TRANSPORT_PROTOCOL_HPP_
#define TRANSPORT_PROTOCOL_HPP_

#include "ariba/utility/addressing/addressing.hpp"
#include "ariba/utility/transport/transport_listener.hpp"
#include "ariba/utility/transport/messages/message.hpp"

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
    /// Allow deleting implementing classes by pointer
    virtual ~transport_protocol() {}
    
	virtual void start() = 0;
	virtual void stop() = 0;
	
	virtual void send(
	        const endpoint_set& endpoints,
	        reboost::message_t message,
	        uint8_t priority = 0) = 0;
	
	/// @deprecated: Use terminate() from transport_connection instead
	virtual void terminate( const address_v* remote ) = 0;
	
	virtual void register_listener( transport_listener* listener ) = 0;
};

}} // namespace ariba::transport

#endif /* TRANSPORT_PROTOCOL_HPP_ */
