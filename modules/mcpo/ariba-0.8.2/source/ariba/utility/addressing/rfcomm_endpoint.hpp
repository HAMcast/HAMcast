#ifndef RFCOMM_ENDPOINT_HPP_
#define RFCOMM_ENDPOINT_HPP_

#include<string>

#include<boost/tr1/functional.hpp>
#include<boost/asio.hpp>

#include "detail/address_convenience.hpp"

#include "port_address.hpp"
#include "mac_address.hpp"

namespace ariba {
namespace addressing {

using namespace std;

struct rfcomm_channel_address_info {
	static const uint16_t type_id;
	static const std::string type_name;
};
typedef port_address_tpl<rfcomm_channel_address_info> rfcomm_channel_address;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class rfcomm_endpoint : public detail::address_convenience<rfcomm_endpoint> {
private:
	mac_address mac_;
	rfcomm_channel_address channel_;
	static const std::string protocol_name;

public:
	typedef rfcomm_endpoint self;

public:
	rfcomm_endpoint() :
		mac_(), channel_() {
	}

	rfcomm_endpoint( const rfcomm_endpoint& copy ) {
		assign(copy);
	}

	rfcomm_endpoint( const mac_address& mac, const rfcomm_channel_address& channel) :
		mac_(mac), channel_(channel) {
	}

	rfcomm_endpoint(const  std::string& mac, const std::string& channel) :
		mac_(mac), channel_(channel) {
	}

	rfcomm_endpoint(const std::string& mac, uint16_t channel) :
		mac_(mac), channel_(channel) {
	}

	rfcomm_endpoint(const std::string& text) {
		assign( text );
	}

	rfcomm_endpoint(const char* text) {
		assign( std::string(text) );
	}

	rfcomm_endpoint(const uint8_t* bytes, size_t size) {
		assign( bytes, size );
	}

	bool assign( const self& copy ) {
		this->mac_ = copy.mac_;
		this->channel_ = copy.channel_;
		return false;
	}

	//--- compare operations --------------------------------------------------

	/// implements comparison operators
	int compare_to(const self& rhs) const {
		if (mac_.compare_to(rhs.mac_)==0 && channel_.compare_to(rhs.channel_)==0) return 0;
		return 1;
	}

	//--- bytes representation ------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	bool is_bytes_size_static() const {
		return true;
	}

	/// returns the number of bytes used for serialization of this address
	size_t to_bytes_size() const {
		return mac_.to_bytes_size() + channel_.to_bytes_size();
	}

	/// converts this address to a binary representation
	void to_bytes(uint8_t* bytes) const {
		mac_.to_bytes(bytes);
		channel_.to_bytes(bytes+mac_.to_bytes_size());
	}

	/// Assigns an address using a bunch of bytes
	bool assign(const uint8_t* bytes, size_t size) {
		assert(size==8);
		return false;
	}

	//--- text representation -------------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	std::string to_string() const {
		return std::string("[")+mac_.to_string()+std::string("]:")+channel_.to_string();
	}

	/// Assigns an address using a human-readable
	bool assign(const std::string& text) {
		std::string mac_str;
		std::string channel_str;
		if (text.at(0)=='[') {
			int i = text.find(']',1);
			mac_str = text.substr(1,i-1);
			channel_str = text.substr(i+2, text.size()-i-1);
			return mac_.assign(mac_str) || channel_.assign(channel_str);
		}
		return true;
	}

	//--- address info --------------------------------------------------------

	/// returns the name of the address
	const string& type_name() const {
		return protocol_name;
	}

	/// returns the id of the address
	uint16_t type_id() const {
		return 0xFE05;
	}

	//--- endpoint elements ---------------------------------------------------

	mac_address& mac() {
		return mac_;
	}

	const mac_address& mac() const {
		return mac_;
	}

	rfcomm_channel_address& channel() {
		return channel_;
	}

	const rfcomm_channel_address& channel() const {
		return channel_;
	}
};

}} // namespace ariba::addressing

namespace boost {

template<>
struct hash<ariba::addressing::rfcomm_endpoint>: public std::unary_function<ariba::addressing::rfcomm_endpoint, std::size_t> {
	std::size_t operator()(const ariba::addressing::rfcomm_endpoint& ep) const {
		return hash_value(ep.to_string());
	}
};

}


#endif /* RFCOMM_ENDPOINT_HPP_ */
