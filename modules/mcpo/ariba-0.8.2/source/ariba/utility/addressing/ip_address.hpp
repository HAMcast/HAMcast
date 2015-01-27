#ifndef IP_ADDRESS_HPP_
#define IP_ADDRESS_HPP_

#include <string>
#include <boost/tr1/functional.hpp>
#include <boost/asio/ip/address.hpp>

#include "detail/address_convenience.hpp"

namespace ariba {
namespace addressing {

using boost::asio::ip::address;
using boost::asio::ip::address_v4;
using boost::asio::ip::address_v6;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class ip_address: public detail::address_convenience<ip_address> {
private:
	address addr;
	static const std::string type_name_ip;

public:
	ip_address() : addr() {
	}

	ip_address(const ip_address& copy) : addr(copy.addr) {
	}

	ip_address(const address& addr) : addr(addr) {

	}

	ip_address(const address_v4& addr) : addr(addr) {

	}

	ip_address(const address_v6& addr) : addr(addr) {

	}

	ip_address(const std::string& text) {
		assign(text);
	}

	ip_address(const char* text) {
		assign(std::string(text));
	}

	ip_address(const uint8_t* bytes, size_t size) {
		assign(bytes, size);
	}

	//--- compare operations --------------------------------------------------

	/// implements comparison operators
	int compare_to(const ip_address& rhs) const {
		if (addr == rhs.addr) return 0;
		if (addr < rhs.addr) return -1; else return 1;
	}

	//--- byte representation -------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	bool is_bytes_size_static() const {
		return false;
	}

	/// returns the number of bytes used for serialization of this address
	size_t to_bytes_size() const {
		return addr.is_v4() ? 4 : 16;
	}

	/// converts this address to a binary representation
	void to_bytes(uint8_t* bytes) const {
		if (addr.is_v4()) {
			address_v4::bytes_type bytes_ = addr.to_v4().to_bytes();
			for (size_t i=0; i<bytes_.size(); i++) bytes[i] = bytes_[i];
		} else {
			address_v6::bytes_type bytes_ = addr.to_v6().to_bytes();
			for (size_t i=0; i<bytes_.size(); i++) bytes[i] = bytes_[i];
		}
	}

	/// Assigns an address using a bunch of bytes
	bool assign(const uint8_t* bytes, size_t size) {
		if (size == 4) {
			address_v4::bytes_type bytes_;
			for (size_t i=0; i<bytes_.size(); i++) bytes_[i] = bytes[i];
			addr = address_v4(bytes_);
			return false;
		} else
		if (size == 16) {
			address_v6::bytes_type bytes_;
			for (size_t i=0; i<bytes_.size(); i++) bytes_[i] = bytes[i];
			addr = address_v6(bytes_);
			return false;
		}
		return true;
	}

	//--- string representation -----------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	std::string to_string() const {
		if (addr.is_v6() && (addr.to_v6().is_v4_compatible()
			|| addr.to_v6().is_v4_mapped()))
			return addr.to_v6().to_v4().to_string();
		return addr.to_string();
	}

	/// Assigns an address using a human-readable
	bool assign(const std::string& text) {
		addr = address::from_string(text);
		return false;
	}

	//--- assignment ----------------------------------------------------------

	/// Assigns an address
	bool assign(const ip_address& rhs) {
		addr = rhs.addr;
		return false;
	}

	//--- address info --------------------------------------------------------

	static const std::string& type_name() {
		return type_name_ip;
	}

	static const uint16_t type_id() {
		return 0x81DD;
	}

	//--- address delegates ---------------------------------------------------

	bool is_multicast() const {
		if (addr.is_v4()) return addr.to_v4().is_multicast();
		return addr.to_v6().is_multicast();
	}

	bool is_loopback() const {
		if (addr.is_v4()) return addr.to_v4() == address_v4::loopback();
		return addr.to_v6().is_loopback();
	}

	bool is_link_local() const {
		if (addr.is_v4()) return false;
		return addr.to_v6().is_link_local();
	}

	bool is_multicast_link_local() const {
		if (addr.is_v4()) return false;
		return addr.to_v6().is_multicast_link_local();

	}

	bool is_multicast_node_local() const {
		if (addr.is_v4()) return false;
		return addr.to_v6().is_multicast_node_local();
	}

	bool is_multicast_site_local() const {
		if (addr.is_v4()) return false;
		return addr.to_v6().is_multicast_site_local();
	}

	bool is_any() const {
		if (addr.is_v4()) return addr.to_v4() == address_v4::any();
		return addr.to_v6() == address_v6::any();
	}

	bool is_v4_compatible() const {
		if (addr.is_v4()) return true;
		return addr.to_v6().is_v4_compatible();
	}

	bool is_v4_mapped() {
		if (addr.is_v4()) return true;
		return addr.to_v6().is_v4_mapped();
	}

	bool is_v4() const {
		return addr.is_v4();
	}

	bool is_v6() const {
		return addr.is_v6();
	}

	//--- address conversions -------------------------------------------------

	const address& asio() const {
		return addr;
	}

	void asio( const address& addr ) {
		this->addr = addr;
	}
};

}} // namespace ariba::addressing

namespace boost {

// boost hash function
template<>
struct hash<ariba::addressing::ip_address>: public std::unary_function<ariba::addressing::ip_address, std::size_t> {
	std::size_t operator()(const ariba::addressing::ip_address& ep) const {
		return hash_value(ep.to_string());
	}
};

}


#endif /* IP_ADDRESS_HPP_ */
