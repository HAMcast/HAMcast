#ifndef TCPIP_HPP_
#define TCPIP_HPP_

#include "ariba/utility/transport/transport.hpp"
#include "ariba/utility/transport/asio/unique_io_service.h"
#include "ariba/utility/transport/transport_connection.hpp"
#include "ariba/utility/addressing/tcpip_endpoint.hpp"
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <queue>
#include "ariba/utility/transport/messages/buffers.hpp"
#include "ariba/utility/logging/Logging.h"

namespace ariba {
namespace transport {

using namespace std;
using ariba::transport::detail::unique_io_service;
using ariba::addressing::tcpip_endpoint;
using boost::asio::ip::tcp;
using boost::asio::ip::address_v6;
using boost::system::error_code;
using reboost::shared_buffer_t;
using reboost::message_t;

class tcpip;
typedef boost::shared_ptr<tcpip> TcpIpPtr;

class tcpip :
    public transport_protocol,
    public boost::enable_shared_from_this<tcpip>
{
    typedef tcpip self;
use_logging_h(tcpip)

private:
    class tcpip_connection :
        public transport_connection,
        public boost::enable_shared_from_this<tcpip_connection>
    {
    public:
        typedef reboost::message_t Packet;
        typedef std::queue<Packet> OutQueue;
        
        struct header_t
        {
            uint32_t length;
            uint16_t prot;  // XXX protlib
        } __attribute__((packed));
            
        tcpip_connection(boost::asio::io_service& io_service, TcpIpPtr parent);
        
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
        tcp::socket sock;
        bool valid;
        TcpIpPtr parent;
        
        tcp::endpoint partner;
        tcpip_endpoint remote;
        tcpip_endpoint local;
        
        vector<OutQueue> out_queues;     // to be locked with out_queues_lock 
        boost::mutex out_queues_lock;
        
        bool sending;       // to be locked with out_queues_lock
        
        header_t header;
        shared_buffer_t buffy;
    };
    typedef boost::shared_ptr<tcpip_connection> ConnPtr;
    typedef std::map<tcp::endpoint, ConnPtr> ConnectionMap;
    
public:
	tcpip( const tcp::endpoint& endp );
	virtual ~tcpip();
	virtual void start();
	virtual void stop();
	
	/**
     * enqueues message for sending
     * create new connection if necessary
     * starts sending mechanism (if not already running)
     */
    void send(
            const tcp::endpoint&,
            reboost::message_t message,
            uint8_t priority = 0 );
	
	/**
	 * Converts address_v to tcp::endpoint and calls the real send() function
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
	virtual void terminate( const tcp::endpoint& remote );
	virtual void register_listener( transport_listener* listener );

	
    /**
     *  returns a vector of (interesting) network interfaces
     *  
     *  [NOTE: The current implementation returns the scope_ids of
     *  all ethX and wlanX network interfaces, to be used for
     *  connections to link-local ipv6 addresses.]
     *  
     *  TODO move to ariba/communication/networkinfo/AddressDiscovery ??
     *  
     */
    static vector<uint64_t> get_interface_scope_ids();

private:
	void accept();
	void async_accept_handler(ConnPtr conn, const error_code& error);
	tcp::endpoint convert_address(const address_v* endpoint);
	tcpip_endpoint convert_address(const tcp::endpoint& endpoint);
	
private:
	transport_listener* listener;
	unique_io_service u_io_service;
	tcp::acceptor acceptor;
	
	ConnectionMap connections;
	boost::mutex connections_lock;
};

}} // namespace ariba::transport

#endif /* TCPIP_HPP_ */
