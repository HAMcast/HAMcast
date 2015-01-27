#ifndef BOOST_ASIO_BLUETOOTH_RFCOMM_HPP__
#define BOOST_ASIO_BLUETOOTH_RFCOMM_HPP__

#include "bluetooth_endpoint.hpp"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

namespace boost {
namespace asio {
namespace bluetooth {

/**
 * The rfcomm class contains flags necessary for RFCOMM sockets.
 *
 * @author Martin Florian <mflorian@lafka.net>
 */
class rfcomm {
public:
	/// The type of endpoint.
	typedef bluetooth_endpoint<rfcomm> endpoint;

	/// Get an Instance.
	/// We need this to fulfill the asio Endpoint requirements, I think.
	static rfcomm get() {
		return rfcomm();
	}

	/// Obtain an identifier for the type of the protocol.
	int type() const {
		return SOCK_STREAM;
	}

	/// Obtain an identifier for the protocol.
	int protocol() const {
		return BTPROTO_RFCOMM;
	}

	/// Obtain an identifier for the protocol family.
	int family() const {
		return AF_BLUETOOTH;
	}

	/// The RFCOMM socket type, lets pray that this will work.
	typedef basic_stream_socket<rfcomm> socket;

	/// The RFCOMM acceptor type.
	typedef basic_socket_acceptor<rfcomm> acceptor;

};

}}} // namespace boost::asio::bluetooth

#endif /* BOOST_ASIO_BLUETOOTH_RFCOMM_HPP__ */
