
#ifndef TRANSPORT_CONNECTION_HPP_
#define TRANSPORT_CONNECTION_HPP_

#include "ariba/utility/addressing/addressing.hpp"
#include "ariba/utility/transport/messages/message.hpp"
#include <boost/shared_ptr.hpp>

using ariba::addressing::address_vf;

namespace ariba {
namespace transport {

class transport_connection
{
public:
    typedef boost::shared_ptr<transport_connection> sptr;
    
    /// Allow deleting implementing classes by pointer
    virtual ~transport_connection() {}
    
    virtual void send(reboost::message_t message, uint8_t priority = 0) = 0;
    
    virtual address_vf getLocalEndpoint() = 0;
    virtual address_vf getRemoteEndpoint() = 0;
    
    virtual void terminate() = 0;
};

} /* namespace transport */
} /* namespace ariba */
#endif /* TRANSPORT_CONNECTION_HPP_ */
