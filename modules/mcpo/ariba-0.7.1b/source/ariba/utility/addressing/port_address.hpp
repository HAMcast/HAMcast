// port_address.hpp, created on 24.06.2009 by Sebastian Mies

#ifndef PORT_ADDRESS_HPP_
#define PORT_ADDRESS_HPP_

#include<string>
#include<cstdio>

#include<boost/tr1/functional.hpp>

#include "detail/address_convenience.hpp"

namespace ariba {
namespace addressing {

struct port_address_info {
	static const uint16_t type_id;
	static const std::string type_name;
};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
template<class AddressInfo = port_address_info>
class port_address_tpl: public detail::address_convenience<
		port_address_tpl<AddressInfo> > {
private:
	uint16_t port;

public:
	typedef port_address_tpl<AddressInfo> self;

	port_address_tpl() {
		port = 0;
	}

	port_address_tpl( const port_address_tpl& copy ) : port(copy.port) {

	}

	port_address_tpl(const std::string& text) {
		assign( text );
	}

	port_address_tpl(const char* text) {
		assign( std::string(text) );
	}

	port_address_tpl(uint16_t port) : port(port) {

	}

	port_address_tpl(const uint8_t* bytes, size_t size) {
		assign( bytes, size );
	}

	//--- compare operations --------------------------------------------------

	/// implements comparison operators
	int compare_to(const self& rhs) const {
		return port - rhs.port;
	}

	//--- bytes representation ------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	bool is_bytes_size_static() const {
		return true;
	}

	/// returns the number of bytes used for serialization of this address
	size_t to_bytes_size() const {
		return 2;
	}

	/// converts this address to a binary representation
	void to_bytes(uint8_t* bytes) const {
		bytes[0] = port >> 8;
		bytes[1] = port & 0xFF;
	}

	/// Assigns an address using a bunch of bytes
	bool assign(const uint8_t* bytes, size_t size) {
		port = ((bytes[0] << 8) + bytes[1]);
		return false;
	}

	//--- text representation -------------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	std::string to_string() const {
		char str[8];
		sprintf(str, "%d", port);
		return std::string(str);
	}

	/// Assigns an address using a human-readable
	bool assign(const std::string& text) {
		unsigned int port_;
		sscanf(text.c_str(), "%d", &port_);
		if (port_ >= 0 && port_ <= 65535) {
			port = (uint16_t) port_;
			return false;
		}
		return true;
	}

	//--- assignment ----------------------------------------------------------

	/// Assigns an address
	bool assign(const self& rhs) {
		port = rhs.port;
		return false;
	}

	//--- address info --------------------------------------------------------

	/// returns the type name
	const std::string& type_name() const {
		return AddressInfo::type_name;
	}

	/// returns the type identifier
	const uint16_t type_id() const {
		return AddressInfo::type_id;
	}

	//--- conversions ---------------------------------------------------------

	uint16_t asio() const {
		return port;
	}

	void asio( uint16_t port ) {
		this->port = port;
	}

	uint16_t value() const {
		return port;
	}

	void value( uint16_t v ) {
		port  = v;
	}
};

typedef port_address_tpl<> port_address;

}} // namespace ariba::addressing

namespace boost {

template<class T>
struct hash<ariba::addressing::port_address_tpl<T> >: public std::unary_function<ariba::addressing::port_address_tpl<T>, std::size_t> {
	std::size_t operator()(const ariba::addressing::port_address_tpl<T>& ep) const {
		return hash_value(ep.value());
	}
};

}

#endif /* PORT_ADDRESS_HPP_ */
