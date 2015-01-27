#ifndef TCPIP_ENDPOINT_HPP_
#define TCPIP_ENDPOINT_HPP_

#include<string>

#include<boost/tr1/functional.hpp>
#include<boost/asio.hpp>

#include "detail/address_convenience.hpp"
#include "port_address.hpp"
#include "ip_address.hpp"

namespace ariba {
namespace addressing {

using boost::asio::ip::tcp;
using namespace std;

struct tcp_address_info {
	static const uint16_t type_id;
	static const std::string type_name;
};

typedef port_address_tpl<tcp_address_info> tcp_port_address;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class tcpip_endpoint : public detail::address_convenience<tcpip_endpoint> {
private:
	ip_address ip;
	tcp_port_address port_;
	static const std::string protocol_name;

public:
	typedef tcpip_endpoint self;

public:
	tcpip_endpoint() :
		ip(), port_() {
	}

	tcpip_endpoint( const tcpip_endpoint& copy ) {
		assign(copy);
	}

	tcpip_endpoint( uint16_t port ) : ip(address_v6::any()), port_(port) {
	}

	tcpip_endpoint( const ip_address& ip, const tcp_port_address& port) :
		ip(ip), port_(port) {
	}

	tcpip_endpoint(const  std::string& ip, const std::string& port) :
		ip(ip), port_(port) {
	}

	tcpip_endpoint(const std::string& ip, uint16_t port) :
		ip(ip), port_(port) {
	}

	tcpip_endpoint(const std::string& text) {
		assign( text );
	}

	tcpip_endpoint(const char* text) {
		assign( std::string(text) );
	}

	tcpip_endpoint(const uint8_t* bytes, size_t size) {
		assign( bytes, size );
	}

	bool assign( const self& copy ) {
		this->ip = copy.ip;
		this->port_ = copy.port_;
		return false;
	}

	//--- compare operations --------------------------------------------------

	/// implements comparison operators
	int compare_to(const self& rhs) const {
		if (ip.compare_to(rhs.ip)==0 && port_.compare_to(rhs.port_)==0) return 0;
		return 1;
	}

	//--- bytes representation ------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	bool is_bytes_size_static() const {
		return false;
	}

	/// returns the number of bytes used for serialization of this address
	size_t to_bytes_size() const {
		return ip.to_bytes_size() + port_.to_bytes_size();
	}

	/// converts this address to a binary representation
	void to_bytes(uint8_t* bytes) const {
		ip.to_bytes(bytes);
		port_.to_bytes(bytes+ip.to_bytes_size());
	}

	/// Assigns an address using a bunch of bytes
	bool assign(const uint8_t* bytes, size_t size) {
		assert(size==6 || size==18);
		if (size==6) {
			ip.assign(bytes,4);
			port_.assign(bytes+4,2);
		} else {
			ip.assign(bytes,16);
			port_.assign(bytes+16,2);
		}
		return false;
	}

	//--- text representation -------------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	std::string to_string() const {
		if (ip.asio().is_v4())
			return ip.to_string()+std::string(":")+port_.to_string();
		else
			return std::string("[")+ip.to_string()+std::string("]:")+port_.to_string();
	}

	/// Assigns an address using a human-readable
	bool assign(const std::string& text) {
		std::string ip_str;
		std::string port_str;
		if (text.at(0)=='[') {
			int i = text.find(']',1);
			ip_str = text.substr(1,i-1);
			port_str = text.substr(i+2, text.size()-i-1);
		} else {
			int i = text.find(':',1);
			ip_str = text.substr(0,i);
			port_str = text.substr(i+1, text.size()-i-1);
		}
		return ip.assign(ip_str) || port_.assign(port_str);
	}

	//--- address info --------------------------------------------------------

	/// returns the name of the address
	const string& type_name() const {
		return protocol_name;
	}

	/// returns the id of the address
	uint16_t type_id() const {
		return 6;
	}

	//--- endpoint elements ---------------------------------------------------

	ip_address& address() {
		return ip;
	}

	const ip_address& address() const {
		return ip;
	}

	tcp_port_address& port() {
		return port_;
	}

	const tcp_port_address& port() const {
		return port_;
	}

	//--- conversions ---------------------------------------------------------

	/// returns the asio endpoint
	tcp::endpoint asio() const {
		return tcp::endpoint(ip.asio(), port_.asio());
	}

	/// sets the asio endpoint
	void asio( tcp::endpoint& endp ) {
		ip.asio(endp.address());
		port_.asio(endp.port());
	}
};

}} // namespace ariba::addressing

namespace boost {

template<>
struct hash<ariba::addressing::tcpip_endpoint>:
	public std::unary_function<ariba::addressing::tcpip_endpoint, std::size_t> {
	std::size_t operator()(const ariba::addressing::tcpip_endpoint& ep) const {
		return hash_value(ep.to_string());
	}
};

}


#endif /* TCPIP_ENDPOINT_HPP_ */
