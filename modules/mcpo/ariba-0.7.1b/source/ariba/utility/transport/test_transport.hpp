// test_transport.hpp, created on 01.07.2009 by Sebastian Mies

#ifndef TEST_TRANSPORT_HPP_
#define TEST_TRANSPORT_HPP_

#include "transport_peer.hpp"
#include "transport_listener.hpp"

#include "rfcomm/rfcomm.hpp"

#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace ariba {
namespace transport {
namespace detail {

using namespace std;
using namespace ariba::transport;
using namespace ariba::addressing;

class listener : public transport_listener {
public:
	virtual void receive_message(
		transport_protocol* transport,
		const address_vf local, const address_vf remote,
		const uint8_t* data, uint32_t size
	) {
		cout << "transport_listener: " << endl;
		cout << "received message data='" << data << "' local=" << local->to_string() << " remote=" << remote->to_string() << endl;
	}
};

void test_transport_process( endpoint_set& local, endpoint_set& remote ) {
	cout << "started " << local.to_string() << endl;
	transport_peer* peer = new transport_peer( local );
	peer->register_listener( new listener() );
	peer->start();
	peer->send( remote, (uint8_t*)"Hello!", 7 );
	getchar();
}

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
void tcp_test() {

	endpoint_set local  = string("tcp{5001};ip{127.0.0.1 | 2001:638:204:6:216:d3ff:fece:1070}");
	endpoint_set remote = string("tcp{5002};ip{127.0.0.1 | 2001:638:204:6:216:d3ff:fece:1070}");

	pid_t pID = fork();
	if (pID < 0) {
		cerr << "Failed to fork" << endl;
	} else
	if (pID == 0) {
		test_transport_process(local,remote);
	} else {
		getchar();
		test_transport_process(remote,local);
	}
	getchar();
}

void bluetooth_test( string& endp_str ) {
	cout << endp_str << endl;
	rfcomm* rfc = new rfcomm( 3 );
	rfc->register_listener( new listener() );
	rfc->start();
	if (endp_str.size()!=0) {
		rfcomm_endpoint endp = endp_str;
		rfc->send( endp, (uint8_t*)"Hello!", 7 );
	}
	getchar();
}

}}} // namespace ariba::transport::detail

#endif /* TEST_TRANSPORT_HPP_ */
