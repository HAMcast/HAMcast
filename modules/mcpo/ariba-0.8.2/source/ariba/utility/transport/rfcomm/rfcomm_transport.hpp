#include "ariba/config.h"

#ifdef HAVE_LIBBLUETOOTH

#ifndef RFCOMM_TRANSPORT_HPP_
#define RFCOMM_TRANSPORT_HPP_

#include "ariba/utility/transport/transport.hpp"
#include "ariba/utility/transport/asio/unique_io_service.h"
#include "ariba/utility/transport/transport_connection.hpp"
#include "ariba/utility/addressing/rfcomm_endpoint.hpp"
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <queue>
#include "ariba/utility/transport/messages/buffers.hpp"
#include "ariba/utility/logging/Logging.h"
#include "bluetooth_rfcomm.hpp"

namespace ariba {
namespace transport {

using namespace std;
using ariba::transport::detail::unique_io_service;
using ariba::addressing::rfcomm_endpoint;
using boost::system::error_code;
using reboost::shared_buffer_t;
using reboost::message_t;


class rfcomm_transport :
    public transport_protocol,
    public boost::enable_shared_from_this<rfcomm_transport>
{
public:
	typedef boost::shared_ptr<rfcomm_transport> sptr;

private:
    typedef rfcomm_transport self;
    typedef boost::asio::bluetooth::rfcomm rfcomm;
    use_logging_h(rfcomm_transport)

    class rfcomm_connection :
        public transport_connection,
        public boost::enable_shared_from_this<rfcomm_connection>
    {
    public:
        typedef reboost::message_t Packet;
        typedef std::queue<Packet> OutQueue;
        
        struct header_t
        {
            uint32_t length;
            uint16_t prot;  // XXX protlib
        } __attribute__((packed));
            
        rfcomm_connection(boost::asio::io_service& io_service,
                rfcomm_transport::sptr parent);
        
        /// Inherited from transport_connection
        virtual void send(reboost::message_t message, uint8_t priority = 0);
        virtual address_vf getLocalEndpoint();
        virtual address_vf getRemoteEndpoint();
        virtual void terminate();
        
        void listen();
        
        void async_connect_handler(const error_code& error);
        
        void async_read_header_handler(const error_code& error, size_t bytes_transferred);
        void async_read_data_handler(const error_code& error, size_t bytes_transferred);
        
        /*
         * is called from asio when write operation "returns",
         * calls private function `send_next_package()`
         */
        void async_write_handler(
                reboost::shared_buffer_t packet,
                const error_code& error,
                size_t bytes_transferred);

        
        void enqueue_for_sending(Packet packet, uint8_t priority);
        
    private:
        /*
         * is called from `send` or `async_write_handler` to begin/keep sending
         * sends the next message with the highest priority in this connection
         */
        void send_next_package();


    public:
        rfcomm::socket sock;
        bool valid;
        rfcomm_transport::sptr parent;
        
        rfcomm::endpoint partner;
        rfcomm_endpoint remote;
        rfcomm_endpoint local;
        
        vector<OutQueue> out_queues;     // to be locked with out_queues_lock 
        boost::mutex out_queues_lock;
        
        bool sending;       // to be locked with out_queues_lock
        
        header_t header;
        shared_buffer_t buffy;
    };
    typedef boost::shared_ptr<rfcomm_connection> ConnPtr;
    typedef std::map<rfcomm::endpoint, ConnPtr> ConnectionMap;
    
public:
    /* constructor */
	rfcomm_transport( const rfcomm::endpoint& endp );
	virtual ~rfcomm_transport();
	
	virtual void start();
	virtual void stop();
	
	/**
     * enqueues message for sending
     * create new connection if necessary
     * starts sending mechanism (if not already running)
     */
    void send(
            const rfcomm::endpoint&,
            reboost::message_t message,
            uint8_t priority = 0 );
	
	/**
	 * Converts address_v to rfcomm::endpoint and calls the real send() function
	 */
	virtual void send(
	        const address_v* remote,
	        reboost::message_t message,
	        uint8_t priority = 0 );
	
	/**
	 * calls send for each destination endpoint in `endpoint_set& endpoints` 
	 */
	virtual void send(
	        const endpoint_set& endpoints,
	        reboost::message_t message,
	        uint8_t priority = 0 );
	
	virtual void terminate( const address_v* remote );
	virtual void terminate( const rfcomm::endpoint& remote );
	virtual void register_listener( transport_listener* listener );

	
private:
	void accept();
	void async_accept_handler(ConnPtr conn, const error_code& error);
	rfcomm::endpoint convert_address(const address_v* endpoint);
	rfcomm_endpoint convert_address(const rfcomm::endpoint& endpoint);
	
private:
	transport_listener* listener;
	unique_io_service u_io_service;
	rfcomm::acceptor acceptor;
	
	ConnectionMap connections;
	boost::mutex connections_lock;
};

}} // namespace ariba::transport

#endif /* RFCOMM_TRANSPORT_HPP_ */
#endif /* HAVE_LIBBLUETOOTH */
