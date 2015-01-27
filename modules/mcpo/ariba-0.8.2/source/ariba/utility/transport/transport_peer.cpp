
#include "ariba/config.h"
#include "transport_peer.hpp"
#include "transport.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/error.hpp>
#include <boost/foreach.hpp>

#ifdef ECLIPSE_PARSER
    #define foreach(a, b) for(a : b)
#else
    #define foreach(a, b) BOOST_FOREACH(a, b)
#endif

// namespace ariba::transport
namespace ariba {
namespace transport {

using namespace ariba::addressing;
using boost::asio::ip::tcp;

#ifdef HAVE_LIBBLUETOOTH
using boost::asio::bluetooth::rfcomm;
#endif

use_logging_cpp(transport_peer);

transport_peer::transport_peer( endpoint_set& local_set ) : local(local_set) {
    
    // setup tcp transports
    foreach(tcp_port_address port, local.tcp) {
        
        if (local.ip.size() > 0) {
            foreach(ip_address ip_addr, local.ip) {
                
                tcp::endpoint endp(ip_addr.asio(), port.asio());
                create_service(endp);
            }
        } else {
            tcp::endpoint endp_v6(tcp::v6(), port.asio());
            tcp::endpoint endp_v4(tcp::v4(), port.asio());
            
            create_service(endp_v6);
            create_service(endp_v4);
        }
        
    }
    
	#ifdef HAVE_LIBBLUETOOTH
    foreach(rfcomm_channel_address channel, local.rfcomm) {
    	if (local.bluetooth.size() > 0) {
    		foreach(mac_address mac, local.bluetooth) {
    			rfcomm::endpoint endp(mac.bluetooth(), channel.value());
    			create_service(endp);
    		}
    	} else {
    		rfcomm::endpoint endp(channel.value());
    		create_service(endp);
    	}
    }
	#endif
}

void transport_peer::create_service(tcp::endpoint endp) {
    try {
        TcpIpPtr tmp_ptr(new tcpip(endp));
        tcps.push_back(tmp_ptr);
        logging_info("Listening on IP/TCP " << endp);
        
    } catch (boost::system::system_error& e) {
        if (e.code() == boost::asio::error::address_in_use) {
            logging_warn("[WARN] Address already in use: "
                    << endp << ". Endpoint will be ignored!");
        } else {
            // Rethrow
            throw;
        }
    }
}

#ifdef HAVE_LIBBLUETOOTH
void transport_peer::create_service(rfcomm::endpoint endp) {
    try {
        rfcomm_transport::sptr tmp_ptr(new rfcomm_transport(endp));
        rfcomms.push_back(tmp_ptr);
        logging_info("Listening on bluetooth/RFCOMM " << endp);
        
    } catch (boost::system::system_error& e) {
        if (e.code() == boost::asio::error::address_in_use) {
            logging_warn("[WARN] Address already in use: "
                    << endp << ". Endpoint will be ignored!");
        } else {
            // Rethrow
            throw;
        }
    }
}
#endif

transport_peer::~transport_peer() {
}

void transport_peer::start() {
    foreach(TcpIpPtr tcp, tcps) {
        tcp->start();
    }
    
#ifdef HAVE_LIBBLUETOOTH
    foreach(rfcomm_transport::sptr x, rfcomms) {
    	x->start();
    }
#endif
}

void transport_peer::stop() {
    foreach(TcpIpPtr tcp, tcps) {
        tcp->stop();
    }
    
#ifdef HAVE_LIBBLUETOOTH
	foreach(rfcomm_transport::sptr x, rfcomms) {
		x->stop();
	}
#endif
}


void transport_peer::send(
        const endpoint_set& endpoints,
        reboost::message_t message,
        uint8_t priority)
{
    foreach(TcpIpPtr tcp, tcps) {
        tcp->send(endpoints, message, priority);
    }
    
#ifdef HAVE_LIBBLUETOOTH
    foreach(rfcomm_transport::sptr x, rfcomms) {
		x->send(endpoints, message, priority);
	}
#endif
}

void transport_peer::terminate( const address_v* remote ) {
	if (remote->instanceof<tcpip_endpoint>())// TODO direkt auf der richtigen verbindung
	{
	    foreach(TcpIpPtr tcp, tcps) {
	        tcp->terminate(remote);
	    }
	}
#ifdef HAVE_LIBBLUETOOTH
	if (remote->instanceof<rfcomm_endpoint>()) {
		foreach(rfcomm_transport::sptr x, rfcomms) {
			x->terminate(remote);
		}
	}
#endif
}

void transport_peer::register_listener( transport_listener* listener ) {
    foreach(TcpIpPtr tcp, tcps) {
        tcp->register_listener(listener);
    }
    
#ifdef HAVE_LIBBLUETOOTH
    foreach(rfcomm_transport::sptr x, rfcomms) {
    	x->register_listener(listener);
    }
#endif
}

}} // namespace ariba::transport
