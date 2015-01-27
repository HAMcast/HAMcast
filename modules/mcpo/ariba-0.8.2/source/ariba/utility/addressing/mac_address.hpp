#ifndef MAC_ADDRESS_HPP_
#define MAC_ADDRESS_HPP_

#include "detail/address_convenience.hpp"

#include "ariba/config.h"
#include<string>
#include<cassert>
#include<stdio.h>
#include<boost/tr1/functional.hpp>
#ifdef HAVE_LIBBLUETOOTH
#include<bluetooth/bluetooth.h>
#endif

namespace ariba {
namespace addressing {

struct mac_address_info {
	static const uint16_t type_id;
	static const std::string type_name;
};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
template<class AddressInfo = mac_address_info>
class mac_address_tpl: public detail::address_convenience<mac_address_tpl<AddressInfo> > {
private:
	uint8_t mac[6];

public:
	mac_address_tpl() {
		for (int i = 0; i < 6; i++)
			mac[i] = 0;
	}

	mac_address_tpl( const mac_address_tpl& copy ) {
		assign(copy);
	}

	mac_address_tpl(const std::string& text) {
		assign(text);
	}

	mac_address_tpl(const char* text) {
		assign(std::string(text));
	}

	mac_address_tpl(const uint8_t* bytes, size_t size) {
		assign(bytes, size);
	}

	//--- compare operations --------------------------------------------------

	/// implements comparison operators
	int compare_to(const mac_address_tpl& rhs) const {
		size_t i = 0;
		while (rhs.mac[i] == mac[i] && i < 6)
			i++;
		return (6 - i);
	}

	//--- bytes representation ------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	bool is_bytes_size_static() const {
		return true;
	}

	/// returns the number of bytes used for serialization of this address
	size_t to_bytes_size() const {
		return 6;
	}

	/// converts this address to a binary representation
	void to_bytes(uint8_t* bytes) const {
		for (size_t i = 0; i < 6; i++)
			bytes[i] = mac[i];
	}

	/// Assigns an address using a bunch of bytes
	bool assign(const uint8_t* bytes, size_t size) {
		assert(size==6);
		for (size_t i = 0; i < 6; i++)
			mac[i] = bytes[i];
		return false;
	}

	//--- string representation -----------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	std::string to_string() const {
		char str[80];
		sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return std::string(str);
	}

	/// Assigns an address using a human-readable
	bool assign(const std::string& text) {
		unsigned int mac_[6];
		sscanf(text.c_str(), "%x:%x:%x:%x:%x:%x", &mac_[0], &mac_[1], &mac_[2],
				&mac_[3], &mac_[4], &mac_[5]);
		for (size_t i = 0; i < 6; i++)
			mac[i] = (uint8_t) mac_[i];
		return false;
	}

	//--- assignment ----------------------------------------------------------

	/// Assigns an address
	bool assign(const mac_address_tpl& rhs) {
		for (size_t i = 0; i < 6; i++)
			mac[i] = rhs.mac[i];
		return false;
	}

	//--- address info --------------------------------------------------------

	const std::string& type_name() const {
		return AddressInfo::type_name;
	}

	const uint16_t type_id() const {
		return AddressInfo::type_id;
	}

	//--- conversion ----------------------------------------------------------
#ifdef HAVE_LIBBLUETOOTH

	bdaddr_t bluetooth() const {
		bdaddr_t addr;
		for (size_t i=0; i<6; i++) addr.b[i] = mac[5-i];
		return addr;
	}

	void bluetooth( bdaddr_t addr ) {
		for (size_t i=0; i<6; i++) mac[i] = addr.b[5-i];
	}
#endif
};

typedef mac_address_tpl<> mac_address;

}} // namespace ariba::addressing

namespace boost {

template<class T>
struct hash<ariba::addressing::mac_address_tpl<T> >: public std::unary_function<ariba::addressing::mac_address_tpl<T>, std::size_t> {
	std::size_t operator()(const ariba::addressing::mac_address_tpl<T>& ep) const {
		return hash_value(ep.to_string());
	}
};

}


#endif /* MAC_ADDRESS_HPP_ */
