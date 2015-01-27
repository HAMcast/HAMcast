#include "tcpip.hpp"

#include <boost/array.hpp>

// interface discovery for link-local destinations
#include <ifaddrs.h>

namespace ariba {
namespace transport {

use_logging_cpp(tcpip)

using namespace ariba::addressing;

typedef boost::mutex::scoped_lock unique_lock;

tcpip::tcpip( const tcp::endpoint& endp )  :
        listener(NULL),
        acceptor(u_io_service.get_asio_io_service(), endp)
{
}

tcpip::~tcpip(){}

void tcpip::start()
{
    // open server socket
    accept();
    
    u_io_service.start();
}


void tcpip::stop()
{
    acceptor.close();
    
    u_io_service.stop();
}


/* see header file for comments */
void tcpip::send(
        const tcp::endpoint& dest_addr,
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
                    new tcpip_connection(
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
                        &tcpip_connection::async_connect_handler,
                        conn,
                        boost::asio::placeholders::error));
    }
}


/* see header file for comments */
void tcpip::send(
        const address_v* remote,
        reboost::message_t message,
        uint8_t priority)
{
    send(convert_address(remote), message, priority);
}


/* see header file for comments */
void tcpip::send(
        const endpoint_set& endpoints,
        reboost::message_t message,
        uint8_t priority )
{
    // network interfaces scope_ids, for link-local connections (lazy initialization)
    vector<uint64_t> scope_ids;
    
    // send a message to each combination of address-address and port
    BOOST_FOREACH( const ip_address address, endpoints.ip ) {
        BOOST_FOREACH( const tcp_port_address port, endpoints.tcp ) {
            tcp::endpoint endp(address.asio(), port.asio());
            
            // special treatment for link local addresses
            //   ---> send over all (suitable) interfaces
            if ( endp.address().is_v6() )
            {
                boost::asio::ip::address_v6 v6_addr = endp.address().to_v6();
                
                if ( v6_addr.is_link_local() )
                {
                    // initialize scope_ids
                    if ( scope_ids.size() == 0 )
                        scope_ids = get_interface_scope_ids();
                    
                    BOOST_FOREACH ( uint64_t id, scope_ids )
                    {                        
                        v6_addr.scope_id(id);
                        endp.address(v6_addr);
    
                        logging_debug("------> SEND TO (link-local): " << endp);
                        // * send *
                        send(endp, message, priority);
                    }
                }
                
                continue;
            }
            
            // * send *
            send(endp, message, priority);
        }
    }
}


void tcpip::register_listener( transport_listener* listener )
{
    this->listener = listener;
}


void tcpip::terminate( const address_v* remote )
{
    terminate(convert_address(remote));
}

void tcpip::terminate( const tcp::endpoint& remote )
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
void tcpip::accept()
{
    // create new connection object
    ConnPtr conn(
            new tcpip_connection(
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

void tcpip::async_accept_handler(ConnPtr conn, const error_code& error)
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

inline tcp::endpoint tcpip::convert_address( const address_v* address )
{
    tcpip_endpoint endpoint = *address;
    
    return tcp::endpoint(
        endpoint.address().asio(), endpoint.port().value()
    );
}


inline tcpip_endpoint tcpip::convert_address(const tcp::endpoint& endpoint)
{
    ip_address address;
    address.asio(endpoint.address());
    tcp_port_address port;
    port.value(endpoint.port());
    return tcpip_endpoint(address, port);
}


vector<uint64_t> tcpip::get_interface_scope_ids()
{
    vector<uint64_t> ret;
    
    struct ifaddrs* ifaceBuffer = NULL;
    void*           tmpAddrPtr  = NULL;
    
    int ok = getifaddrs( &ifaceBuffer );
    if( ok != 0 ) return ret;

    for( struct ifaddrs* i=ifaceBuffer; i != NULL; i=i->ifa_next ) {

        // ignore devices that are disabled or have no ip
        if(i == NULL) continue;
        struct sockaddr* addr = i->ifa_addr;
        if (addr==NULL) continue;

        // only use ethX and wlanX devices
        string device = string(i->ifa_name);
        if ( (device.find("eth") == string::npos) &&
              (device.find("wlan")  == string::npos) /* &&
              (device.find("lo")  == string::npos) XXX */ )
        {
            continue;
        }

        // only use interfaces with ipv6 link-local addresses 
        if (addr->sa_family == AF_INET6)
        {
            // convert address
            // TODO should be possible without detour over strings
            char straddr[INET6_ADDRSTRLEN];
            tmpAddrPtr= &((struct sockaddr_in6*)addr)->sin6_addr;
            inet_ntop( i->ifa_addr->sa_family, tmpAddrPtr, straddr, sizeof(straddr) );

            address_v6 v6addr = address_v6::from_string(straddr);
            if ( v6addr.is_link_local() )
            {
                // * append the scope_id to the return vector *
                ret.push_back(if_nametoindex(i->ifa_name));
            }

        }
    }

    freeifaddrs(ifaceBuffer);
    
    return ret;
}


/*****************
 ** inner class **
 *****************/

tcpip::tcpip_connection::tcpip_connection(boost::asio::io_service & io_service, TcpIpPtr parent)  :
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
void tcpip::tcpip_connection::send(
        reboost::message_t message,
        uint8_t priority)
{
    enqueue_for_sending(message, priority);
}


address_vf tcpip::tcpip_connection::getLocalEndpoint()
{
    return local;
}


address_vf tcpip::tcpip_connection::getRemoteEndpoint()
{
    return remote;
}


void tcpip::tcpip_connection::terminate()
{
    parent->terminate(partner);
}


/*------------------------------
 | things we defined ourselves |
 ------------------------------*/
void tcpip::tcpip_connection::async_connect_handler(const error_code& error)
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


void tcpip::tcpip_connection::listen()
{
    boost::asio::async_read(
            this->sock,
            boost::asio::mutable_buffers_1(&this->header, sizeof(header_t)),
            boost::bind(
                    &tcpip::tcpip_connection::async_read_header_handler,
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred
            )
    );
}


void tcpip::tcpip_connection::async_read_header_handler(const error_code& error, size_t bytes_transferred)
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
                    &tcpip::tcpip_connection::async_read_data_handler,
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred
            )
    );
}

void tcpip::tcpip_connection::async_read_data_handler(
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
void tcpip::tcpip_connection::async_write_handler(reboost::shared_buffer_t packet, const error_code& error, size_t bytes_transferred)
{
    if ( error )
    {        
        // remove this connection
        parent->terminate(partner); 

        return;
    }
    
    send_next_package();
}



void tcpip::tcpip_connection::enqueue_for_sending(Packet packet, uint8_t priority)
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
void tcpip::tcpip_connection::send_next_package()
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
                        &tcpip::tcpip_connection::async_write_handler,
                        this->shared_from_this(),
                        packet,  // makes sure our shared pointer lives long enough ;-)
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)
        );
    }
}

}} // namespace ariba::transport
