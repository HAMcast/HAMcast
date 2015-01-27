#ifndef ENDPOINT_SET_HPP_
#define ENDPOINT_SET_HPP_

#include "addressing.hpp"
#include "tcpip_endpoint.hpp"

#include <sstream>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

namespace ariba {
namespace addressing {

using boost::unordered_set;

/**
 * This end-point set shelters known addresses of a device.
 * Transport protocols use this class to address devices.
 *
 * Example of a string representation:
 * "tcp{500|501};ip{10.11.12.13};bluetooth{01:02:03:04:05:06};rfcomm{1234}"
 * Inside a address type specification, addresses are separated by a bar (|).
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class endpoint_set {
public:
	// layer 2
	unordered_set<mac_address> bluetooth;

	// layer 3
	unordered_set<ip_address> ip;

	// layer 4
	unordered_set<tcp_port_address> tcp;
	unordered_set<rfcomm_channel_address> rfcomm;

	// mutex
	boost::mutex io_mutex;
	typedef boost::mutex::scoped_lock scoped_lock;

private:
	template<uint8_t type, class V>
	size_t to_bytes_dynamic( const unordered_set<V>& set, uint8_t* bytes ) const {
		size_t size = 0;
		bytes[0] = type;
		uint8_t* size_ptr = bytes+1;
		bytes +=2;
		size += 2;
		BOOST_FOREACH( const V& value, set ) {
			bytes[0] = (uint8_t)value.to_bytes_size();
			bytes++;
			size++;
			value.to_bytes(bytes);
			bytes += value.to_bytes_size();
			size += value.to_bytes_size();
		}
		*size_ptr = size-2;
		return size;
	}

	template<class V>
	void from_bytes_dynamic( unordered_set<V>& set, const uint8_t* bytes, uint8_t size ) {
		size_t pos = 0;
		while (pos < size) {
			uint8_t length = bytes[0];
			bytes++; pos++;
			V obj(bytes,length);
			set.insert(obj);
			bytes+=length; pos+=length;
		}
	}

	template<uint8_t type, class V>
	size_t to_bytes_fixed( const unordered_set<V>& set, uint8_t* bytes ) const {
		size_t fixed_size = V().to_bytes_size();
		bytes[0] = type;
		bytes[1] = (uint8_t)(set.size()* fixed_size);
		bytes+=2;
		BOOST_FOREACH( const V& value, set ) {
			value.to_bytes(bytes);
			bytes += value.to_bytes_size();
		}
		return 2 + set.size() * fixed_size;
	}

	template<class V>
	void from_bytes_fixed( unordered_set<V>& set, const uint8_t* bytes, uint8_t size ) {
		size_t fixed_size = V().to_bytes_size();
		uint8_t num = size/fixed_size;
		for (uint8_t i=0; i<num; i++) {
			V obj(bytes, fixed_size);
			set.insert(obj);
			bytes += fixed_size;
		}
	}

	template<class V>
	std::string to_string_set( const unordered_set<V>& set, const std::string& type ) const {
		if (set.size()==0) return std::string("");
		std::ostringstream buf;
		buf << type << "{";
		bool first = true;
		BOOST_FOREACH( const V& value, set ) {
			if (!first) {
				buf << " | ";
			} else
				first = false;
			buf << value.to_string();
		}
		buf << "};";
		return buf.str();
	}

	static void trim(string& str) {
		string::size_type pos = str.find_last_not_of(' ');
		if(pos != string::npos) {
			str.erase(pos + 1);
			pos = str.find_first_not_of(' ');
			if(pos != string::npos) str.erase(0, pos);
		}
		else str.erase(str.begin(), str.end());
	}

	static string::size_type skip( const char* chars, string::size_type pos, const std::string& str ) {
		bool found = true;
		while (pos<str.size() && found) {
			found = false;
			for (size_t i=0; chars[i]!=0 && !found; i++)
				if (str.at(pos)==chars[i]) {
					pos++;
					found = true;
				}
		}
		return pos;
	}

	template<class V>
	size_t from_string_set( unordered_set<V>& set, string::size_type pos, const std::string& str ) {
		while (pos < str.size() && pos != string::npos) {
			pos = skip("} |\n\r", pos, str);
			string::size_type nend1 = str.find('}',pos);
			string::size_type nend2 = str.find('|',pos);
			if (nend1==string::npos && nend2==string::npos) break;
			if (nend1==string::npos) nend1=str.size();
			if (nend2==string::npos) nend2=str.size();
			string::size_type nend = nend2 < nend1 ? nend2:nend1;
			std::string sub = str.substr(pos, min(nend2,nend1)-pos);
			trim(sub);
			V obj( sub );
			set.insert(obj);
			pos = nend+1;
			if (nend1<nend2) break;
		}
		return pos-1;
	}

public:
	enum layers {
		Layer1 = 1, Layer2 = 2,	Layer3 = 4, Layer4 = 8, Layer5 = 16,
		Layer6 = 32, Layer7 = 64, Layer8 = 128, NoLoopback = 256,AllLayers = ~0,
		Layer1_3 = Layer1|Layer2|Layer3,
		Layer1_4 = Layer1|Layer2|Layer3|Layer4,
	};

	endpoint_set() {

	}

	endpoint_set( const endpoint_set& copy ) :
		bluetooth(copy.bluetooth), ip(copy.ip), tcp(copy.tcp), rfcomm(copy.rfcomm) {
	}

	endpoint_set( const std::string& str ) {
		assign(str);
	}

	endpoint_set( const uint8_t* bytes, size_t size ) {
		assign(bytes, size);
	}

	/// adds an address or endpoint to this set
	void add( const address_v* address, int layers = AllLayers ) {
		scoped_lock lock(io_mutex);
		if ( address->instanceof<tcpip_endpoint> () ) {
			const tcpip_endpoint& addr = *address;
			if (layers & Layer3 &&
					!((layers & NoLoopback) && addr.address().is_loopback()) )
				ip.insert( addr.address() );
			if (layers & Layer4) tcp.insert( addr.port() );
		} else
		if ( address->instanceof<ip_address>() ) {
			const ip_address& addr = *address;
			if ((layers & Layer3) &&
					!((layers & NoLoopback) && addr.is_loopback()))
				ip.insert( addr );
		} else
		if (address->instanceof<rfcomm_endpoint>() ) {
			const rfcomm_endpoint& endp = *address;
			if (layers & Layer2) bluetooth.insert( endp.mac() );
			if (layers & Layer4) rfcomm.insert( endp.channel() );
		} else
		if (address->instanceof<mac_address>() ) {
			const mac_address& endp = *address;
			if (layers & Layer2) bluetooth.insert( endp );
		}
	}

	/// adds addresses from another endpoint set
	void add( const endpoint_set& eps, int layers = AllLayers ) {
		scoped_lock lock(io_mutex);

		// merge layer 2 addresses
		if (layers & Layer2) {
			bluetooth.insert(eps.bluetooth.begin(), eps.bluetooth.end() );
		}

		// merge layer 3 addresses
		if (layers & Layer3) {
			ip.insert(eps.ip.begin(), eps.ip.end() );
		}

		// merge layer 4 addresses
		if (layers & Layer4) {
			tcp.insert(eps.tcp.begin(), eps.tcp.end() );
			rfcomm.insert(eps.rfcomm.begin(), eps.rfcomm.end() );
		}
	}

	/// removes an address or endpoint from this set
	void remove( const address_vf address ) {
		scoped_lock lock(io_mutex);
		if ( address->instanceof<tcpip_endpoint> () ) {
			const tcpip_endpoint& addr = *address;
			ip.erase( addr.address() );
			tcp.erase( addr.port() );
		} else
		if ( address->instanceof<ip_address>() ) {
			const ip_address& addr = *address;
			ip.erase( addr );
		} else
		if (address->instanceof<rfcomm_endpoint>() ) {
			const rfcomm_endpoint& endp = *address;
			bluetooth.erase( endp.mac() );
			rfcomm.erase( endp.channel() );
		}
	}

	/// checks whether two end-points are disjoint
	/// (only check lower level addresses)
	bool disjoint_to( const endpoint_set& set ) const {
		scoped_lock lock(const_cast<boost::mutex&>(io_mutex));
		BOOST_FOREACH( const mac_address& mac, bluetooth )
			if (set.bluetooth.count(mac) !=0 ) return false;
		BOOST_FOREACH( const ip_address& ip_, ip )
			if (set.ip.count(ip_) !=0 ) return false;
		return true;
	}

	bool intersects_with( const endpoint_set& set ) const {
		return !disjoint_to(set);
	}

	bool is_subset_of( const endpoint_set& set ) const {
		throw "Not implemented!";
		return false;
	}

	/// returns true, if this address has a fixed size in bytes
	bool is_bytes_size_static() const {
		return false;
	}

	/// returns the number of bytes used for serialization of this address
	size_t to_bytes_size() const {
		scoped_lock lock(const_cast<boost::mutex&>(io_mutex));
		size_t size = 0;

		// bluetooth mac list (layer 2)
		size += bluetooth.size() * mac_address().to_bytes_size();

		// ip list (layer 3)
		BOOST_FOREACH( const ip_address& ip_, ip )
			size += (ip_.to_bytes_size() + 1 /* =length */);

		// tcp ports (layer 4)
		size += tcp.size() * tcp_port_address().to_bytes_size();

		// bluetooth rfcomm channels (layer 4)
		size += rfcomm.size() * rfcomm_channel_address().to_bytes_size();

		// length/type encoding
		size += 4 /* number of items*/ * 2 /* length of type and length */;

		return size;
	}

	/// converts this address to a binary representation
	void to_bytes(uint8_t* bytes) const {
		scoped_lock lock(const_cast<boost::mutex&>(io_mutex));

		/// bluetooth mac list (layer 2)
		bytes += to_bytes_fixed<0x21, mac_address>( bluetooth, bytes );

		// ip list (layer 3)
		bytes += to_bytes_dynamic<0x31, ip_address>(ip, bytes);

		// tcp ports (layer 4)
		bytes += to_bytes_fixed<0x41, tcp_port_address>( tcp, bytes );

		// rfcomm channels (layer 4)
		bytes += to_bytes_fixed<0x42, rfcomm_channel_address>( rfcomm, bytes );
	}

	/// Assigns an address using a bunch of bytes
	bool assign(const uint8_t* bytes, size_t size) {
		scoped_lock lock(io_mutex);

		size_t pos = 0;
		while (pos < size) {
			uint8_t type = bytes[0];
			uint8_t length = bytes[1];
			bytes+=2; pos+=2;

			switch (type) {

			// bluetooth mac
			case 0x21: {
				from_bytes_fixed<mac_address>( bluetooth, bytes, length );
				break;
			}

			// ip
			case 0x31: {
				from_bytes_dynamic<ip_address>( ip, bytes, length );
				break;
			}
			// tcp
			case 0x41: {
				from_bytes_fixed<tcp_port_address>( tcp, bytes, length );
				break;
			}
			// rfcomm
			case 0x42: {
				from_bytes_fixed<rfcomm_channel_address>( rfcomm, bytes, length );
				break;
			}

			default: {
				pos = size;
				break;
			}
			}
			bytes += length; pos+=length;
		}
		return false;
	}

	/// generates a string out of this endpoint-set
	std::string to_string() const {
		scoped_lock lock(const_cast<boost::mutex&>(io_mutex));
		std::string smac = to_string_set<mac_address>(bluetooth, "bluetooth");
		std::string sip  = to_string_set<ip_address>(ip, "ip");
		std::string stcp = to_string_set<tcp_port_address>(tcp, "tcp");
		std::string srfcomm = to_string_set<rfcomm_channel_address>(rfcomm, "rfcomm");
		return smac+sip+stcp+srfcomm;
	}

	/// assigns an endpoint-set out of a string
	void assign( const std::string& str ) {
		scoped_lock lock(io_mutex);
		string::size_type pos = 0;
		while (pos < str.size() && pos!=string::npos) {
			pos = skip("}; \n\r", pos, str );
			string::size_type nend = str.find('{',pos);
			if (nend == string::npos) break;
			std::string type = str.substr(pos,nend-pos);
			pos = nend+1;
			trim(type);
			if (type=="bluetooth")
				pos = from_string_set<mac_address>(bluetooth, pos, str );
			else if (type=="ip")
				pos = from_string_set<ip_address>(ip, pos, str );
			else if (type=="tcp")
				pos = from_string_set<tcp_port_address>(tcp, pos, str );
			else if (type=="rfcomm")
				pos = from_string_set<rfcomm_channel_address>(rfcomm, pos, str );
			else
				pos = str.find('}',pos);
		}
	}

	endpoint_set& operator=( const endpoint_set& rhs ) {
		scoped_lock lock(io_mutex);
		this->bluetooth = rhs.bluetooth;
		this->ip = rhs.ip;
		this->rfcomm = rhs.rfcomm;
		this->tcp = rhs.tcp;
		return *this;
	}

	/// checks wheter the two endpoint sets are identical
	bool operator== ( const endpoint_set& rhs ) const {
		return (rhs.rfcomm == rfcomm && rhs.ip == ip && rhs.tcp == tcp &&
				rhs.bluetooth == bluetooth);
	}

	bool operator!= ( const endpoint_set& rhs ) const {
		return !(*this==rhs);
	}
};

}} // namespace ariba::addressing

#endif /* ENDPOINT_SET_HPP_ */
