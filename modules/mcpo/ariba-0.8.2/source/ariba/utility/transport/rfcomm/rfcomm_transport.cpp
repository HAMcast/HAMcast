#include "rfcomm_transport.hpp"

#ifdef HAVE_LIBBLUETOOTH
#include <boost/array.hpp>

namespace ariba {
namespace transport {

use_logging_cpp(rfcomm_transport)

using namespace ariba::addressing;

typedef boost::mutex::scoped_lock unique_lock;

/* constructor */
rfcomm_transport::rfcomm_transport( const rfcomm_transport::rfcomm::endpoint& endp )  :
        listener(NULL),
        acceptor(u_io_service.get_asio_io_service(), endp)
{
}

rfcomm_transport::~rfcomm_transport(){}

void rfcomm_transport::start()
{
    // open server socket
    accept();
    
    u_io_service.start();
}


void rfcomm_transport::stop()
{
    acceptor.close();
    
    u_io_service.stop();
}


/* see header file for comments */
void rfcomm_transport::send(
        const rfcomm::endpoint& dest_addr,
        reboost::message_t message,
        uint8_t priority)
{
    ConnPtr conn;
    bool need_to_connect = false;
    
    {
        unique_lock lock(connections_lock);
        
        ConnectionMap::iterator it = connections.find(dest_addr);
        if (it == connections.end())
        {
            ConnPtr tmp_ptr(
                    new rfcomm_connection(
                            u_io_service.get_asio_io_service(),
                            shared_from_this() )
                    );
            conn = tmp_ptr;
            
            conn->partner = dest_addr;
            conn->remote = convert_address(dest_addr);
            
            // Note: starting the send is the obligation of the connect_handler
            // (avoids trying to send while not connected yet)
            conn->sending =  true;
            need_to_connect = true;
            
            ConnectionMap::value_type item(dest_addr, conn);
            connections.insert(item);
            
        } else {
            conn = it->second;
        }
    }
    
    
    // * the actual send *
    conn->enqueue_for_sending(message, priority);
    
    // if new connection connect to the other party
    if ( need_to_connect )
    {
        conn->sock.async_connect(
                dest_addr,
                boost::bind(
                        &rfcomm_connection::async_connect_handler,
                        conn,
                        boost::asio::placeholders::error));
    }
}


/* see header file for comments */
void rfcomm_transport::send(
        const address_v* remote,
        reboost::message_t message,
        uint8_t priority)
{
    send(convert_address(remote), message, priority);
}


/* see header file for comments */
void rfcomm_transport::send(
        const endpoint_set& endpoints,
        reboost::message_t message,
        uint8_t priority )
{
    // send a message to each combination of address-address and port
    BOOST_FOREACH( const mac_address mac, endpoints.bluetooth ) {
        BOOST_FOREACH( const rfcomm_channel_address channel, endpoints.rfcomm ) {
            rfcomm::endpoint endp(mac.bluetooth(), channel.value());
            
            // * send *
            send(endp, message, priority);
        }
    }
}


void rfcomm_transport::register_listener( transport_listener* listener )
{
    this->listener = listener;
}


void rfcomm_transport::terminate( const address_v* remote )
{
    terminate(convert_address(remote));
}

void rfcomm_transport::terminate( const rfcomm::endpoint& remote )
{
    ConnPtr conn;
    
    // find and forget connection
    {
        unique_lock lock(connections_lock);
        
        ConnectionMap::iterator it = connections.find(remote);
        if (it == connections.end())
        {
            return;
        }
        
        conn = it->second;
        
        connections.erase(it);
    }

    // close connection
    boost::system::error_code ec;
    conn->sock.shutdown(tcp::socket::shutdown_both, ec);
    conn->sock.close(ec);
}


/* private */
void rfcomm_transport::accept()
{
    // create new connection object
    ConnPtr conn(
            new rfcomm_connection(
                    u_io_service.get_asio_io_service(),
                    shared_from_this()
            )
    );
    
    // wait for incoming connection
    acceptor.async_accept(
            conn->sock,
            boost::bind(&self::async_accept_handler,
                    this->shared_from_this(),
                    conn,
                    boost::asio::placeholders::error)
    );
}

void rfcomm_transport::async_accept_handler(ConnPtr conn, const error_code& error)
{
    if ( ! error )
    {
        conn->partner = conn->sock.remote_endpoint();
        conn->remote = convert_address(conn->partner);
        conn->local = convert_address(conn->sock.local_endpoint());
        
        {
            unique_lock lock(connections_lock);
            
            ConnectionMap::value_type item(conn->sock.remote_endpoint(), conn);
            connections.insert(item);
        }
        
        // read
        conn->listen();
    }
    
    // accept further connections
    accept();
}

inline rfcomm_transport::rfcomm::endpoint 
rfcomm_transport::convert_address( const address_v* address )
{
    rfcomm_endpoint endpoint = *address;
    
    return rfcomm::endpoint(
        endpoint.mac().bluetooth(), endpoint.channel().value()
    );
}


inline rfcomm_endpoint rfcomm_transport::convert_address(const rfcomm::endpoint& endpoint)
{
    mac_address mac;
    mac.bluetooth(endpoint.address());
    rfcomm_channel_address channel;
    channel.value(endpoint.channel());
    return rfcomm_endpoint(mac, channel);
}


/*****************
 ** inner class **
 *****************/

rfcomm_transport::rfcomm_connection::rfcomm_connection(
    boost::asio::io_service & io_service,
    rfcomm_transport::sptr parent)  :
        sock(io_service),
        valid(true),
        parent(parent),
        out_queues(8), //TODO How much priorities shall we have?
        sending(false)
{
        header.length = 0;
        header.prot = 0;
}

/*-------------------------------------------
 | implement transport_connection interface |
 -------------------------------------------*/
void rfcomm_transport::rfcomm_connection::send(
        reboost::message_t message,
        uint8_t priority)
{
    enqueue_for_sending(message, priority);
}


address_vf rfcomm_transport::rfcomm_connection::getLocalEndpoint()
{
    return local;
}


address_vf rfcomm_transport::rfcomm_connection::getRemoteEndpoint()
{
    return remote;
}


void rfcomm_transport::rfcomm_connection::terminate()
{
    parent->terminate(partner);
}


/*------------------------------
 | things we defined ourselves |
 ------------------------------*/
void rfcomm_transport::rfcomm_connection::async_connect_handler(const error_code& error)
{
    if (error)
    {
        parent->terminate(partner);

        return;
    }
    
    // save address in ariba format
    local = parent->convert_address(sock.local_endpoint());
    
    // Note: sending has to be true at this point
    send_next_package();
    
    listen();
}


void rfcomm_transport::rfcomm_connection::listen()
{
    boost::asio::async_read(
            this->sock,
            boost::asio::mutable_buffers_1(&this->header, sizeof(header_t)),
            boost::bind(
                    &rfcomm_transport::rfcomm_connection::async_read_header_handler,
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred
            )
    );
}


void rfcomm_transport::rfcomm_connection::async_read_header_handler(const error_code& error, size_t bytes_transferred)
{
    if (error)
    {
        parent->terminate(partner);

        return;
    }

    // convert byte order
    header.length = ntohl(header.length);
    header.length -= 2;  // XXX protlib
    
    assert(header.length > 0);
    
    // new buffer for the new packet
    buffy = shared_buffer_t(header.length);

    // * read data *
    boost::asio::async_read(
            this->sock,
            boost::asio::buffer(buffy.mutable_data(), buffy.size()),
            boost::bind(
                    &rfcomm_transport::rfcomm_connection::async_read_data_handler,
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred
            )
    );
}

void rfcomm_transport::rfcomm_connection::async_read_data_handler(
        const error_code& error, size_t bytes_transferred)
{
    if (error)
    {
        parent->terminate(partner);

        return;
    }
    
    message_t msg;
    msg.push_back(buffy);
    buffy = shared_buffer_t();

    if ( parent->listener )
        parent->listener->receive_message(shared_from_this(), msg);
    
    listen();
}

/* see header file for comments */
void rfcomm_transport::rfcomm_connection::async_write_handler(reboost::shared_buffer_t packet, const error_code& error, size_t bytes_transferred)
{
    if ( error )
    {        
        // remove this connection
        parent->terminate(partner); 

        return;
    }
    
    send_next_package();
}



void rfcomm_transport::rfcomm_connection::enqueue_for_sending(Packet packet, uint8_t priority)
{
    bool restart_sending = false;
    
    // enqueue packet  [locked]
    {
        unique_lock(out_queues_lock);
        
        assert( priority < out_queues.size() );
        out_queues[priority].push(packet);
        
        if ( ! sending )
        {
            restart_sending = true;
            sending = true;
        }
    }
    
    // if sending was stopped, we have to restart it here
    if ( restart_sending )
    {
        send_next_package();
    }
}

/* see header file for comments */
void rfcomm_transport::rfcomm_connection::send_next_package()
{
    Packet packet;
    bool found = false;

    // find packet with highest priority  [locked]
    {
        unique_lock(out_queues_lock);
        
        for ( vector<OutQueue>::iterator it = out_queues.begin();
                it != out_queues.end(); it++ )
        {
            if ( !it->empty() )
            {
                packet = it->front();
                it->pop();
                found = true;
                
                break;
            }
        }
        
        // no packets waiting --> stop sending
        if ( ! found )
        {
            sending = false;
        }
    }
    
    // * send *
    if ( found )
    {
        reboost::shared_buffer_t header_buf(sizeof(header_t));
        header_t* header = (header_t*)(header_buf.mutable_data());
        header->length = htonl(packet.size()+2);  // XXX protlib
        
        packet.push_front(header_buf);
        
        // "convert" message to asio buffer sequence
        vector<boost::asio::const_buffer> send_sequence(packet.length());
        for ( int i=0; i < packet.length(); i++ )
        {
            shared_buffer_t b = packet.at(i);
            send_sequence.push_back(boost::asio::buffer(b.data(), b.size()));
        }
        
        // * async write *
        boost::asio::async_write(
                this->sock,
                send_sequence,
                boost::bind(
                        &rfcomm_transport::rfcomm_connection::async_write_handler,
                        this->shared_from_this(),
                        packet,  // makes sure our shared pointer lives long enough ;-)
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)
        );
    }
}

}} // namespace ariba::transport

#endif /* HAVE_LIBBLUETOOTH */
