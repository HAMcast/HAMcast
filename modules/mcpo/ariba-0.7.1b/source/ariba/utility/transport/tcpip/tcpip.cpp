#include "tcpip.hpp"

#define _NO_LOGGING

// std includes
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <boost/foreach.hpp>

// protlib includes
#include "protlib/network_message.h"
#include "protlib/tp_over_tcp.h"
#include "protlib/tperror.h"
#include "protlib/logfile.h"
#include "protlib/queuemanager.h"
#include "protlib/threadsafe_db.h"
#include "protlib/setuid.h"

// protlib namespaces
using namespace protlib;
using namespace protlib::log;

logfile commonlog;
protlib::log::logfile& protlib::log::DefaultLog(commonlog);

namespace ariba {
namespace transport {

using namespace ariba::addressing;


tcpip_endpoint convert( const appladdress* addr ) {
	const char* ip_str = addr->get_ip_str();
	tcpip_endpoint endpoint( std::string(ip_str), addr->get_port() );
	return endpoint;
}

appladdress convert( const tcpip_endpoint& endpoint ) {
	tcpip_endpoint* e = const_cast<tcpip_endpoint*>(&endpoint);
	appladdress
		peer(e->address().to_string().c_str(), "tcp", e->port().asio() );
//	cout << endpoint.to_string() << " to " << peer.get_ip_str() << ":" << peer.get_port() << endl;
	return peer;
}

tcpip::tcpip( uint16_t port ) : 
	done ( false ),
	running ( false ),
	port( port ),
	tpreceivethread ( NULL ),
	tpthread ( NULL ),
	listener ( NULL ) {
}

tcpip::~tcpip() {
	if (running) stop();
}

bool get_message_length( NetMsg& m, uint32& clen_bytes ) {
	clen_bytes = m.decode32();
	m.set_pos_r(-4);
	return true;
}

void tcpip::start() {
	done = false;
	running = false;

	// initalize netdb and setuid
	protlib::tsdb::init();
	protlib::setuid::init();

	// set tcp parameters
	port_t port = this->port; // port
	TPoverTCPParam tppar(4, get_message_length, port);

	// create receiver thread
	FastQueue* tpchecker_fq = new FastQueue("TCPTransport", true);
	QueueManager::instance()->register_queue(tpchecker_fq,
			message::qaddr_signaling);

	// start thread
	pthread_create( &tpreceivethread, NULL, tcpip::receiverThread, this );
	tpthread = new ThreadStarter<TPoverTCP, TPoverTCPParam> ( 1, tppar );
	tpthread->start_processing();
}

void tcpip::stop() {
	// stop receiver thread
	done = true;

	// stop TPoverTCP
	tpthread->stop_processing();
	tpthread->abort_processing(true);
	tpthread->wait_until_stopped();

	// unregister TPoverTCP
	delete QueueManager::instance()->get_queue( message::qaddr_signaling );
	QueueManager::instance()->unregister_queue( message::qaddr_signaling );

	// destroy QueueManager
	QueueManager::clear();

	// de-initalize netdb and setuid
	protlib::setuid::end();
	protlib::tsdb::end();

	// wait for thread to finish and delete
	pthread_join(tpreceivethread, NULL);
}

void tcpip::send( const address_v* remote, const uint8_t* data, uint32_t size ) {

	// prepare netmsg with length and and port
	NetMsg* datamsg = new NetMsg(size + 6);
	datamsg->encode32( size + 2,  true );
	datamsg->encode16( this->port,true );

	for (size_t i=0; i<size; i++)
		datamsg->encode8( data[i],true );

	// send message
	tcpip_endpoint endpoint = *remote;
	appladdress peer = convert(endpoint);

	// add to output queue
	tpthread->get_thread_object()->send( datamsg, peer, false );
}

void tcpip::send( const endpoint_set& endpoints, const uint8_t* data, uint32_t size ) {
	// send a message to each combination of ip-address and port
	BOOST_FOREACH( const ip_address ip, endpoints.ip ) {
		BOOST_FOREACH( const tcp_port_address port, endpoints.tcp ) {
			tcpip_endpoint endpoint(ip,port);
			address_vf vf = endpoint;
			send(vf,data,size);
		}
	}
}

void tcpip::terminate( const address_v* remote) {
	tcpip_endpoint endpoint = *remote;
	appladdress peer = convert(endpoint);
	peer.convert_to_ipv6();
	tpthread->get_thread_object()->terminate( peer );
}

void tcpip::register_listener( transport_listener* listener ) {
	this->listener = listener;
}

void* tcpip::receiverThread( void* ptp ) {
	// get reference to transport object
	tcpip& tp = *((tcpip*)ptp);

	// get queue
	FastQueue* fq =
		QueueManager::instance()->get_queue(message::qaddr_signaling);

	// main processing loop
	tp.running = true;
	while (!tp.done) {

		// wait for new message to approach
		message* msg = fq->dequeue_timedwait(300);

		// message has arrived? no-> continue
		if (!msg) continue;

		// handle transport message
		TPMsg* tpmsg = dynamic_cast<TPMsg*> (msg);
		if (!tpmsg) {
			delete msg;
			continue;
		}


		// get address & message
		const appladdress* remote_peer = static_cast<const appladdress*>( tpmsg->get_peeraddress() );
		const appladdress* local_peer  = static_cast<const appladdress*>( tpmsg->get_ownaddress() );
		NetMsg* datamsg = tpmsg->get_message();
		// not a data message? -> continue!
		if (!datamsg) {
			delete tpmsg;
			continue;
		}

		// get length and remote endpoint port
		datamsg->set_pos(0);
		uint32_t message_size = datamsg->decode32(true)-2;
		//uint16_t remote_port = datamsg->decode16(true);
		// inform listener
		if (tp.listener != NULL) {
			tcpip_endpoint remote = convert(remote_peer);
			tcpip_endpoint local  = convert(local_peer);
			tp.listener->receive_message(
					&tp, local, remote, datamsg->get_buffer()+6, message_size );
		}

		tpmsg->set_message(NULL);
		delete datamsg;
		delete tpmsg;
	}
	// clean queue & stop
    fq->cleanup();
	tp.running = false;
	return NULL;
}

}} // namespace ariba::transport
