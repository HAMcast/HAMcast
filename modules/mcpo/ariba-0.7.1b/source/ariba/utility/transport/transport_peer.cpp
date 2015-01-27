
#include "ariba/config.h"
#include "transport_peer.hpp"
#include "transport.hpp"

// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;

transport_peer::transport_peer( endpoint_set& local_set ) : local(local_set) {
	// setup tcp transports
	tcp = NULL;
	//cout << "#tcpip_transports = " << local.tcp.size() << endl;
	if (local.tcp.size()==1) {
		tcp = new tcpip(local.tcp.begin()->value());
		//cout << "Started tcpip_transport on port "  << local.tcp.begin()->value() << endl;
	}

	#ifdef HAVE_LIBBLUETOOTH
	// setup rfcomm transports
	rfc = NULL;
	//cout << "#rfcomm_transports = " << local.rfcomm.size() << endl;
	if ( local.rfcomm.size() == 1 ) {
		rfc = new rfcomm( local.rfcomm.begin()->value() );
		//cout << "Started rfcomm_transport on port "  << local.rfcomm.begin()->value() << endl;
	}
	#endif
}

transport_peer::~transport_peer() {
	if (tcp !=NULL ) delete tcp;
#ifdef HAVE_LIBBLUETOOTH
	if (rfc !=NULL ) delete rfc;
#endif
}

void transport_peer::start() {
	if (tcp!=NULL) tcp->start();
#ifdef HAVE_LIBBLUETOOTH
	if (rfc!=NULL) rfc->start();
#endif
}

void transport_peer::stop() {
	if (tcp!=NULL) tcp->stop();
#ifdef HAVE_LIBBLUETOOTH
	if (rfc!=NULL) rfc->stop();
#endif
}

void transport_peer::send( const address_v* remote, const uint8_t* data, uint32_t size ) {
	if (remote->instanceof<tcpip_endpoint>() && tcp!=NULL) {
		tcp->send(remote,data,size);
	} else
#ifdef HAVE_LIBBLUETOOTH
	if (remote->instanceof<rfcomm_endpoint>() && rfc!=NULL) {
		rfc->send(remote,data,size);
	} else
#endif
		cerr << "Could not send message to " << remote->to_string() << endl;
}

void transport_peer::send( const endpoint_set& endpoints, const uint8_t* data, uint32_t size ) {
	if (tcp!=NULL) tcp->send(endpoints,data,size);
#ifdef HAVE_LIBBLUETOOTH
	if (rfc!=NULL) rfc->send(endpoints,data,size);
#endif
}

void transport_peer::terminate( const address_v* remote ) {
	if (remote->instanceof<tcpip_endpoint>() && tcp!=NULL)
		tcp->terminate(remote);
#ifdef HAVE_LIBBLUETOOTH
	if (remote->instanceof<rfcomm_endpoint>() && rfc!=NULL)
		rfc->terminate(remote);
#endif
}

void transport_peer::register_listener( transport_listener* listener ) {
	if (tcp!=NULL) tcp->register_listener(listener);
#ifdef HAVE_LIBBLUETOOTH
	if (rfc!=NULL) rfc->register_listener(listener);
#endif
}

}} // namespace ariba::transport
