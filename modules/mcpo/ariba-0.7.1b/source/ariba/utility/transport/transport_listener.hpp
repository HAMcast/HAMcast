// transport_listener.hpp, created on 01.07.2009 by Sebastian Mies

#ifndef TRANSPORT_LISTENER_HPP_
#define TRANSPORT_LISTENER_HPP_

#include "ariba/utility/addressing/addressing.hpp"

// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;

class transport_protocol;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class transport_listener {
public:
	/// called when a message is received
	virtual void receive_message(
		transport_protocol* transport,
		const address_vf local, const address_vf remote,
		const uint8_t* data, uint32_t size
	) {
		std::cout << "transport_listener: not implemented" << std::endl;
	}
};

}} // namespace ariba::transport


#endif /* TRANSPORT_LISTENER_HPP_ */
