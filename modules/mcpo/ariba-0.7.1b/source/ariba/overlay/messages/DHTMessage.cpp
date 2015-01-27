#include "DHTMessage.h"

#include<boost/foreach.hpp>

namespace ariba {
namespace overlay {

vsznDefault(DHTMessage);

DHTMessage::DHTMessage() {
	this->key.setLength(0);
	this->ttl = 0;
	this->replace = false;
}

DHTMessage::DHTMessage( const Data& key ) {
	// calculate hash of key
	this->hash = NodeID::sha1( key.getBuffer(), key.getLength() / 8 );
	this->key = key.clone();
	this->ttl = 0;
	this->replace = false;
}

DHTMessage::DHTMessage( const Data& key, const Data& value ) {
	// calculate hash of key
	this->hash = NodeID::sha1( key.getBuffer(), key.getLength() / 8 );
	this->key = key.clone();
	this->values.push_back( value.clone() );
	this->ttl = 0;
	this->replace = false;
}

DHTMessage::DHTMessage( const Data& key, const vector<Data>& values ) {
	this->hash = NodeID::sha1( key.getBuffer(), key.getLength() / 8 );
	this->key = key.clone();
	BOOST_FOREACH(const Data value, values )
		this->values.push_back( value.clone() );
	this->ttl = 0;
	this->replace = false;
}

DHTMessage::~DHTMessage() {
	this->key.release();
	BOOST_FOREACH( Data& value, values ) value.release();
}

}}
