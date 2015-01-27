#include "ariba/config.h"

#ifdef HAVE_LIBBLUETOOTH

#include "rfcomm.hpp"

#include "ariba/utility/transport/asio/asio_io_service.h"
#include "ariba/utility/transport/asio/rfcomm.hpp"
#include "ariba/utility/transport/asio/bluetooth_endpoint.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <memory.h>
#include <deque>
#include <cerrno>

namespace ariba {
namespace transport {

use_logging_cpp(rfcomm)

using namespace boost::asio;
using namespace detail;
using namespace std;

using boost::system::error_code;

class link_data {
public:
	uint8_t size_[4];
	size_t size;
	uint8_t* buffer;
};

class link_info {
public:
	link_info(io_service& io ) :
		io(io), up(false), connecting(false), local(), remote(), socket(new bluetooth::rfcomm::socket(io)), connect_retries(0),
		size(0), buffer(NULL), sending(false) {
	}

	~link_info() {
		delete socket;
	}

	void reinit() {
		delete socket;
		socket = new bluetooth::rfcomm::socket(io);
		up = false;
	}

	io_service& io;

	// state
	bool up;
	bool connecting;
	rfcomm_endpoint local, remote;
	bluetooth::rfcomm::socket* socket;
	int connect_retries;

	// read buffer
	uint8_t size_[4];
	size_t size;
	uint8_t* buffer;

	// send buffer
	bool sending;
	boost::mutex mutex;
	std::deque<link_data> send_buffer;
};

void rfcomm::shutdown(link_info* info) {
	if (info != NULL && info->up) {
		info->up = false;
		info->socket->shutdown( bluetooth::rfcomm::socket::shutdown_both );
	}
}


inline bluetooth::rfcomm::endpoint convert( const rfcomm_endpoint& endpoint ) {
	return bluetooth::rfcomm::endpoint(
		endpoint.mac().bluetooth(), endpoint.channel().value()
	);
}

inline rfcomm_endpoint convert( const bluetooth::rfcomm::endpoint& endpoint ) {
	mac_address mac;
	mac.bluetooth( endpoint.address() );
	rfcomm_channel_address channel;
	channel.value( endpoint.channel() );
	return rfcomm_endpoint( mac, channel );
}


rfcomm::rfcomm(uint16_t channel) :
	channel(channel), io(asio_io_service::alloc()) {
	accept_retries = 0;
}

rfcomm::~rfcomm() {
	asio_io_service::free();
}

void rfcomm::start() {

	// start io service
	asio_io_service::start();

	// create acceptor
	logging_info( "Binding to channel " << channel );
	acceptor = new bluetooth::rfcomm::acceptor(io,
		bluetooth::rfcomm::endpoint(bluetooth::rfcomm::get(), channel )
	);

	send_data = new link_data();

	// start accepting
	start_accept();
}

void rfcomm::stop() {
	logging_info( "Stopping asio rfcomm" );
}

void rfcomm::send(const address_v* remote, const uint8_t* data, size_t size) {

	// get end-point
	rfcomm_endpoint endpoint = *remote;
	endpoint = convert(convert(endpoint));

	// try to find established connector
	logging_debug("Trying to find a already existing link.");
	link_info* info = NULL;
	for (size_t i = 0; i < links.size(); i++)
		if (links[i]->remote.mac() == endpoint.mac()) {
			logging_debug("Using already established link");
			info = links[i];
			break;
		}

	// not found, or not up? ->try to (re-)connect
	if (info==NULL || ((!info->up || !info->socket->is_open()) && !info->connecting) ) {
		logging_debug( "Connecting to " << endpoint.to_string() );
		if (info != NULL && (!info->up || !info->socket->is_open())) {
			logging_error("Old link is down. Trying to re-establish link.");
			info->reinit();
		} else {
			info = new link_info(io);
		}
		info->connect_retries = 0;
		info->remote = endpoint;
		info->connecting = true;
		info->socket->async_connect( convert(endpoint), boost::bind(
			&rfcomm::handle_connect, this,
			boost::asio::placeholders::error, info
		));
		links.push_back(info);
	}

	// copy message
	link_data ldata;
	ldata.size = size;
	ldata.size_[0] = (size >> 24) & 0xFF;
	ldata.size_[1] = (size >> 16) & 0xFF;
	ldata.size_[2] = (size >> 8) & 0xFF;
	ldata.size_[3] = (size >> 0) & 0xFF;
	ldata.buffer = new uint8_t[size];
	memcpy(ldata.buffer, data, size);

	// enqueue message
	info->mutex.lock();
	info->send_buffer.push_back(ldata);
	info->mutex.unlock();

	// start writing
	io.post( boost::bind( &rfcomm::start_write, this, info) );
}

void rfcomm::send(const endpoint_set& endpoints, const uint8_t* data, size_t size) {
	// send a message to each combination of mac-address and channel
	BOOST_FOREACH( const mac_address mac, endpoints.bluetooth ) {
		BOOST_FOREACH( const rfcomm_channel_address channel, endpoints.rfcomm ) {
			rfcomm_endpoint endpoint(mac, channel);
			address_vf vf = endpoint;
			send(vf,data,size);
		}
	}
}

void rfcomm::terminate( const address_v* remote) {
	// get end-point
	rfcomm_endpoint endpoint = *remote;

	for (size_t i = 0; i < links.size(); i++)
		if (links[i]->remote.mac() == endpoint.mac()) {
			// close socket
			shutdown(links[i]);
			break;
		}
}

void rfcomm::register_listener(transport_listener* listener) {
	this->listener = listener;
}

void rfcomm::start_accept() {

	logging_info( "Waiting for connections ..." );

	// start accepting a connection
	link_info* info = new link_info(io);
	acceptor->async_accept(*info->socket, boost::bind(
		// bind parameters
		&rfcomm::handle_accept, this,

		// handler parameters
		boost::asio::placeholders::error, info
	));
	asio_io_service::start();
}

void rfcomm::handle_accept(const error_code& error, link_info* info) {
	if (error) {
		logging_error( "Error waiting for new connections. Error code: "<< error.message()
				<< ", trying to recover (attempt " << accept_retries << ")");

		// restart accepting
		if (accept_retries<3) {
			accept_retries++;
			start_accept();
		} else
			delete info;

		return;
	}

	links_mutex.lock();

	// convert endpoints
	info->up = true;
	info->local  = convert( info->socket->local_endpoint()  );
	info->remote = convert( info->socket->remote_endpoint() );

	logging_debug("Accepted incoming connection from "
		<< info->remote.to_string() );

	// add to list
	links.push_back(info);
	links_mutex.unlock();

	// start i/o
	start_read(info);
	start_write(info);

	// restart accept
	start_accept();
}

void rfcomm::handle_connect( const error_code& error, link_info* info ) {
	if (error) {
		logging_error( "Can not connect. Error code: "
				<< error.message() << " Retrying ... "
				"(attempt " << info->connect_retries << ")" );

		// do we retry this connection? yes->
		if (info->connect_retries<3) {
			// increase counter
			info->connecting = false;
			info->connect_retries++;
			info->reinit();

			// retry connection attempt
			info->socket->async_connect( convert(info->remote), boost::bind(
				&rfcomm::handle_connect, this,
				boost::asio::placeholders::error, info
			));

		} else { // no-> delete link and stop
			return;
		}
	}

	// convert endpoints
	info->up = true;
	info->connecting = false;
	info->local  = convert( info->socket->local_endpoint()  );
	info->remote = convert( info->socket->remote_endpoint() );

	logging_debug( "Connected to " << info->remote.to_string() );

	// add to list
	links_mutex.lock();
	links.push_back(info);
	links_mutex.unlock();

	// start i/o
	start_read(info);
	start_write(info);
}

void rfcomm::start_read(link_info* info) {
	logging_debug("Start reading ...");

	// start read
	boost::asio::async_read(*info->socket,

		// read size of packet
		boost::asio::buffer(info->size_, 4),

		// bind handler
		boost::bind(

			// bind parameters
			&rfcomm::handle_read_header, this,

			// handler parameters
			placeholders::error, placeholders::bytes_transferred, info
		)
	);
}

void rfcomm::handle_read_header(const error_code& error, size_t bytes,
	link_info* info) {

	// handle error
	if (error) {
		logging_error("Failed to receive message payload. Error code: "
				<< error.message() );
		shutdown(info);
		return;
	}

	// ignore errors and wait for all data to be received
	if (bytes != 4) return;

	// get size
	info->size = (info->size_[0]<<24) + (info->size_[1] << 16) +
			(info->size_[2]<< 8) + (info->size_[3] << 0);

	// allocate buffer
	info->buffer = new uint8_t[info->size];

	// start read
	boost::asio::async_read(*info->socket,
		// read size of packet
		boost::asio::buffer(info->buffer, info->size),
		// bind handler
		boost::bind(
			// bind parameters
			&rfcomm::handle_read_data, this,
			// handler parameters
			placeholders::error, placeholders::bytes_transferred, info
		)
	);
}

void rfcomm::handle_read_data(const error_code& error, size_t bytes,
	link_info* info) {

	// check error
	if (error) {
		logging_error("Failed to receive message payload. Error: " << error.message() );
		shutdown(info);
		return;
	}

	// wait for all data to be received
	if (bytes != info->size)
		return;

	// deliver data
	listener->receive_message(this, info->local, info->remote, info->buffer, info->size );

	// free buffers and reset size buffer
	delete [] info->buffer;
	for (size_t i=0; i<4; i++) info->size_[i] = 0;

	start_read(info);
}

void rfcomm::start_write( link_info* info ) {
	// do not start writing if sending is in progress
	if (info->sending || !info->up || info->send_buffer.size()==0) return;

	// set sending flag
	info->sending = true;

	// safely remove data from deque
	*send_data = info->send_buffer.front();
	info->send_buffer.pop_front();

	boost::array<boost::asio::mutable_buffer, 2> buffer;
	buffer[0] = boost::asio::buffer(send_data->size_,4);
	buffer[1] = boost::asio::buffer(send_data->buffer,send_data->size);

	// start writing
	boost::asio::async_write(*info->socket,
		// read size of packet
		buffer,
		// bind handler
		boost::bind(
			// bind parameters
			&rfcomm::handle_write_data, this,
			// handler parameters
			placeholders::error, placeholders::bytes_transferred,
			info, send_data->size, send_data->buffer
		)
	);
}

void rfcomm::handle_write_data(const error_code& error, size_t bytes,
	link_info* info, size_t size, uint8_t* buffer ) {

	// handle error
	if (error) {
		logging_error( "Message sent error. Error: " << error.message() );
		info->sending = false;
		shutdown(info);
		return;
	}

	//  wait for all data to be sent
	if (bytes != (size+4) )
		return;

	logging_debug( "Message sent" );

	// free buffer
	delete [] buffer;
	info->sending = false;

	// restart-write
	start_write(info);
}

}} // namespace ariba::transport

#endif
