#include "ariba/config.h"

#ifdef HAVE_LIBBLUETOOTH

#ifndef RFCOMM_HPP_
#define RFCOMM_HPP_

#include "ariba/utility/transport/transport.hpp"
#include "ariba/utility/transport/asio/asio_io_service.h"
#include "ariba/utility/transport/asio/bluetooth_endpoint.hpp"
#include "ariba/utility/transport/asio/rfcomm.hpp"

#include <boost/thread.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/io_service.hpp>

#include "ariba/utility/logging/Logging.h"

namespace ariba {
namespace transport {

using boost::system::error_code;
using namespace boost::asio;

class link_info;
class link_data;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class rfcomm : public transport_protocol {
	use_logging_h(rfcomm)
public:
	rfcomm( uint16_t channel );
	virtual ~rfcomm();
	virtual void start();
	virtual void stop();
	virtual void send( const address_v* remote, const uint8_t* data, size_t size );
	virtual void send( const endpoint_set& endpoints, const uint8_t* data, size_t size );
	virtual void terminate( const address_v* remote );
	virtual void register_listener( transport_listener* listener );

private:
	uint16_t channel;
	boost::mutex links_mutex;
	vector<link_info*> links;
	io_service& io;
	transport_listener* listener;
	bluetooth::rfcomm::acceptor* acceptor;
	int accept_retries;
	link_data* send_data;

	void start_accept();

	void handle_accept(  const error_code& error, link_info* info );

	void start_read( link_info* info );

	void handle_read_header( const error_code& error, size_t bytes, link_info* info );

	void handle_read_data(  const error_code& error, size_t bytes, link_info* info );

	void handle_connect( const error_code& error, link_info* info );

	void start_write( link_info* info );

	void handle_write_data(const error_code& error, size_t bytes,
		link_info* info, size_t size, uint8_t* buffer );

	void shutdown(link_info* info);
};

}} // namespace ariba::transport

#endif /* RFCOMM_HPP_ */
#endif
