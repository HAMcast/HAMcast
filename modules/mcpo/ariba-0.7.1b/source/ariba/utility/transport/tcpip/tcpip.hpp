#ifndef TCPIP_HPP_
#define TCPIP_HPP_

#include "ariba/utility/transport/transport.hpp"
#include <pthread.h>

// forward declaration
namespace protlib {
template<class X, class Y>
class ThreadStarter;
class TPoverTCP;
class TPoverTCPParam;
}

namespace ariba {
namespace transport {

using namespace protlib;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class tcpip : public transport_protocol {
public:
	tcpip( uint16_t port );
	virtual ~tcpip();
	virtual void start();
	virtual void stop();
	virtual void send( const address_v* remote, const uint8_t* data, uint32_t size );
	virtual void send( const endpoint_set& endpoints, const uint8_t* data, uint32_t size );
	virtual void terminate( const address_v* remote );
	virtual void register_listener( transport_listener* listener );

private:
	volatile bool done, running;
	uint16_t port;
	pthread_t tpreceivethread;
	ThreadStarter<TPoverTCP, TPoverTCPParam>* tpthread;
	static void* receiverThread( void* ptp );
	transport_listener* listener;
};

}} // namespace ariba::transport

#endif /* TCPIP_HPP_ */
