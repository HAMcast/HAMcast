#include "ariba/config.h"

#ifdef HAVE_LIBBLUETOOTH

#ifndef BOOST_ASIO_BLUETOOTH_BLUETOOTH_ENDPOINT_HPP__
#define BOOST_ASIO_BLUETOOTH_BLUETOOTH_ENDPOINT_HPP__

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <iostream>

#include <boost/asio/basic_stream_socket.hpp>

namespace boost {
namespace asio {
namespace bluetooth {

/**
 * Describes an endpoint for a RFCOMM Bluetooth socket.
 *
 * @author Martin Florian <mflorian@lafka.net>
 */
template<typename BluetoothProtocol>
class bluetooth_endpoint {
private:
	static const bdaddr_t addr_any;

public:
	/// The protocol type associated with the endpoint.
	typedef BluetoothProtocol protocol_type;

	/// The type of the endpoint structure. This type is dependent on the
	/// underlying implementation of the socket layer.
	typedef boost::asio::detail::socket_addr_type data_type; // <-- Do I need this?
	//typedef sockaddr_rc data_type;

	/// Default constructor.
	bluetooth_endpoint() :
		data_() {
		data_.rc_family = AF_BLUETOOTH;
		data_.rc_bdaddr = addr_any;
		data_.rc_channel = 0;
	}

	bluetooth_endpoint(const BluetoothProtocol& protocol,
			uint8_t channel) :
		data_() {
		data_.rc_family = AF_BLUETOOTH;
		data_.rc_bdaddr = addr_any;
		data_.rc_channel = channel;
	}

	/// Construct an endpoint using a port number, specified in the host's byte
	/// order. The IP address will be the any address (i.e. INADDR_ANY or
	/// in6addr_any). This constructor would typically be used for accepting new
	/// connections.
	bluetooth_endpoint(uint8_t channel) :
		data_() {
		data_.rc_family = AF_BLUETOOTH;
		data_.rc_bdaddr = addr_any;
		data_.rc_channel = channel;
	}

	/// Construct an endpoint using a port number and an BT address.
	/// The address is in human readable form as a string.
	bluetooth_endpoint(const char *addr, uint8_t channel) :
		data_() {
		data_.rc_family = AF_BLUETOOTH;
		data_.rc_channel = channel;
		str2ba(addr, &data_.rc_bdaddr);
	}

	/// Construct an endpoint using a port number and an BT address.
	/// The address is given in the bluetooth-internal format.
	bluetooth_endpoint(const bdaddr_t& addr, uint8_t channel) :
		data_() {
		data_.rc_family = AF_BLUETOOTH;
		data_.rc_channel = channel;
		data_.rc_bdaddr = addr;
	}

	/// Copy constructor.
	bluetooth_endpoint(const bluetooth_endpoint& other) :
		data_(other.data_) {
	}

	/// Assign from another endpoint.
	bluetooth_endpoint& operator=(const bluetooth_endpoint& other) {
		data_ = other.data_;
		return *this;
	}

	/// The protocol associated with the endpoint.
	protocol_type protocol() const {
		return protocol_type::get();
	}

	/// Get the underlying endpoint in the native type.
	/// TODO: make this nice and generic -> union like in tcp
	data_type* data() {
		return (boost::asio::detail::socket_addr_type*) &data_;
	}

	/// Get the underlying endpoint in the native type.
	const data_type* data() const {
		return (boost::asio::detail::socket_addr_type*) &data_;
	}

	/// Get the underlying size of the endpoint in the native type.
	std::size_t size() const {
		return sizeof(data_type);
	}

	/// Set the underlying size of the endpoint in the native type.
	void resize(std::size_t size) {
		if (size > sizeof(data_type)) {
			boost::system::system_error e(boost::asio::error::invalid_argument);
			boost::throw_exception(e);
		}
	}

	/// Get the capacity of the endpoint in the native type.
	std::size_t capacity() const {
		return sizeof(data_type);
	}

	/// Get the channel associated with the endpoint. The port number is always in
	/// the host's byte order.
	uint8_t channel() const {
		return data_.rc_channel;
	}

	/// Set the channel associated with the endpoint. The port number is always in
	/// the host's byte order.
	void channel(uint8_t channel_num) {
		data_.rc_channel = channel_num;
	}

	/// Get the Bluetooth address associated with the endpoint.
	const bdaddr_t& address() const {
		return data_.rc_bdaddr;
	}

	/// Set the Bluetooth address associated with the endpoint.
	void address(const boost::asio::ip::address& addr) {
		bluetooth_endpoint<BluetoothProtocol> tmp_endpoint(addr, channel());
		data_ = tmp_endpoint.data_;
	}

	/// Compare two endpoints for equality.
	friend bool operator==(const bluetooth_endpoint& e1,
			const bluetooth_endpoint& e2) {
		return e1.address() == e2.address() && e1.channel() == e2.channel();
	}

	/// Compare two endpoints for inequality.
	friend bool operator!=(const bluetooth_endpoint& e1,
			const bluetooth_endpoint& e2) {
		return e1.address() != e2.address() || e1.channel() != e2.channel();
	}

	/// Compare endpoints for ordering.
	friend bool operator<(const bluetooth_endpoint<BluetoothProtocol>& e1,
			const bluetooth_endpoint<BluetoothProtocol>& e2) {
		int cmp = bacmp(&e1.address(), &e2.address());
		
		if (cmp == 0) {
			return e1.channel() < e2.channel();
		} else {
			return cmp < 0;
		}
	}
	
	friend ostream& operator<<(
			ostream& out,
			const bluetooth_endpoint<BluetoothProtocol>& endp) {
		char* mac_str = batostr(&endp.data_.rc_bdaddr);
		out << "[" << mac_str << "]:" << (int)endp.data_.rc_channel;
		free(mac_str);
		return out;
	}

private:
	// The underlying rfcomm socket address structure thingy.
	//struct data_type data_;
	struct sockaddr_rc data_;
};

template<typename X>
const bdaddr_t bluetooth_endpoint<X>::addr_any = { {0u, 0u, 0u, 0u, 0u, 0u} };

}}} // namespace boost::asio::bluetooth

#endif /* BOOST_ASIO_BLUETOOTH_BLUETOOTH_ENDPOINT_HPP__ */
#endif /* HAVE_LIBBLUETOOTH */
