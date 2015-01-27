#include "DhtMessage.h"

#include<boost/foreach.hpp>

namespace ariba_service {
namespace dht {

vsznDefault(DhtMessage);

DhtMessage::DhtMessage() :
	ttl( 0 ),
	replace( false )
{}

DhtMessage::DhtMessage( DhtMessageType type, const std::string& key ) :
	type( static_cast<uint8_t>(type) ),
	ttl( 0 ),
	replace( false ),
	key( key )
{}

DhtMessage::DhtMessage( DhtMessageType type, const std::string& key,
		const std::string& value, uint16_t ttl ) :
	type( static_cast<uint8_t>(type) ),
	ttl( ttl ),
	replace( false ),
	key( key ),
	values(1, value)
{}

DhtMessage::DhtMessage( DhtMessageType type, const std::string& key,
		const vector<string>& values, uint16_t ttl ) :
	type( static_cast<uint8_t>(type) ),
	ttl( ttl ),
	replace( false ),
	key( key )
{
	// preallocate enough room so we don't need to copy a lot
	this->values.reserve(values.size());
	BOOST_FOREACH(const std::string value, values )
		this->values.push_back( value );
}

DhtMessage::~DhtMessage() {
	// empty
}

}}
