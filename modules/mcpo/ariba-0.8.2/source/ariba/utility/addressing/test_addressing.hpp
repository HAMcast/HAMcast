#ifndef TEST_ADDRESSING_HPP_
#define TEST_ADDRESSING_HPP_

#include "addressing.hpp"

namespace ariba {
namespace addressing {
namespace detail {

using namespace ariba::addressing;

void test() {

	mac_address mac("01:02:03:04:05:06");
	port_address port("12345");
	ip_address ipv4("127.0.0.1");
	ip_address ipv6("::127.0.0.1");

	tcpip_endpoint tcpip;
	tcpip = "10.11.12.13:5001";
	cout << tcpip.to_string() << endl;
	tcpip = "[::10.11.12.13]:5001";
	cout << tcpip.to_string() << endl;

	address_vf addr_mac  = mac;
	address_vf addr_port = port;
	to_string_vf addr_to_string = ipv4;

	cout << addr_to_string->to_string() << endl;
	cout << addr_mac->clone()->to_string() << endl;
	cout << (*addr_mac == *addr_mac) << endl;

	uint8_t bytes[80];

	tcpip.to_bytes(bytes);
	for (size_t i=0; i<tcpip.to_bytes_size(); i++) printf("%02X",bytes[i]);
	printf("\n");
	tcpip = "10.11.12.13:5001";
	tcpip.assign( bytes, 18 );
	cout << tcpip.to_string() << endl;

	cout << "Testing endpoint_set: " << endl;
	cout << "-> to_string methods:" << endl;
	string set_test = "tcp{5001 | 5002};ip{::10.11.12.13 | 127.0.0.1};bluetooth{01:02:03:04:05:06};rfcomm{10 | 11 | 12 | 13};";
	endpoint_set set = set_test;
	cout << "   * This = " << set.to_string() << endl;
	cout << "   * That = " << set_test << endl;
	cout << "   * Ok = " << (set.to_string().size() == set_test.size()) << endl;
	cout << "-> to_bytes methods:" << endl;
	uint8_t* set_bytes = new uint8_t[set.to_bytes_size()];
	set.to_bytes(set_bytes);
	endpoint_set new_set(set_bytes, set.to_bytes_size());
	cout << "   * Bin  = ";
	for (size_t i=0; i<set.to_bytes_size(); i++) printf("%02X",set_bytes[i]);
	cout << endl;
	cout << "   * This = " << set.to_string() << endl;
	cout << "   * That = " << new_set.to_string() << endl;
	cout << "   * Ok = " << (set.to_string().size() == new_set.to_string().size()) << endl;

/*
	tcpip_endpoint tcp("10.11.12.13", "1234");
	endpoint_vf endp_tcp = tcp;
	*endp_tcp->get(0) = "::12.13.14.15";

	cout << endp_tcp->get(0)->to_string() << endl;
	cout << tcp.address().to_string() << endl;

	cout << endp_tcp->to_string() << endl;

//	*addr_mac = "0a:0b:0c:0d:0e:0f";

	cout << mac.to_string() << endl;
	cout << addr_mac->clone() << endl;
	cout << addr_port->clone() << endl;
	cout << (*addr_mac == *addr_mac) << endl;
*/
}

}}} // namespace ariba::addressing::detail

#endif /* TEST_ADDRESSING_HPP_ */
