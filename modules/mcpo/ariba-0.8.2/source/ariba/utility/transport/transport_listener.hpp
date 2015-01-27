// transport_listener.hpp, created on 01.07.2009 by Sebastian Mies

#ifndef TRANSPORT_LISTENER_HPP_
#define TRANSPORT_LISTENER_HPP_

#include "ariba/utility/addressing/addressing.hpp"
#include "ariba/utility/transport/messages/buffers.hpp"
#include "ariba/utility/transport/transport_connection.hpp"

// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class transport_listener {
public:
    /// Allow deleting implementing classes by pointer
    virtual ~transport_listener() {}
    
	/// called when a message is received
	virtual void receive_message(
        transport_connection::sptr connection,
		reboost::message_t msg
	) {
		std::cout << "transport_listener: not implemented" << std::endl;
	}
};

}} // namespace ariba::transport


#endif /* TRANSPORT_LISTENER_HPP_ */
