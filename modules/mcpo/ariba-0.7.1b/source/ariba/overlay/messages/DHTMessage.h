#ifndef DHTMESSAGE_H_
#define DHTMESSAGE_H_

#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"

namespace ariba {
namespace overlay {

using ariba::utility::Message;
using_serialization;

class DHTMessage : public Message { VSERIALIZEABLE
public:
	DHTMessage();
	DHTMessage( const Data& key );
	DHTMessage( const Data& key, const Data& value );
	DHTMessage( const Data& key, const vector<Data>& values );
	virtual ~DHTMessage();

	const NodeID& getHashedKey() const {
		return hash;
	}

	const Data& getKey() const {
		return key;
	}

	/// returns the first element of the key vector
	const Data& getValue() const {
		return values.at(0);
	}

	bool hasValues() const {
		return values.size() != 0;
	}

	uint16_t getTTL() const {
		return ttl;
	}

	void setTTL( uint16_t ttl ) {
		this->ttl = ttl;
	}

	void setReplace( bool replace ) {
		this->replace = replace;
	}

	bool doReplace() const {
		return replace;
	}

	/// return alles values for the key
	const vector<Data>& getValues() const {
		return values;
	}

private:
	NodeID hash;
	uint16_t ttl;
	bool replace;
	Data key;
	vector<Data> values;
};

}} // namespace ariba::overlay

sznBeginDefault( ariba::overlay::DHTMessage, X ) {

	// serialize flags
	X && replace && cI(0,7);

	// serialize tll
	X && ttl;

	// key serialization
	uint16_t key_length = key.isUnspecified() ? 0 : key.getLength();
	X && key_length;
	if (X.isDeserializer()) key.setLength( key_length );
	X && this->key;

	// store number of values
	uint16_t num_values = values.size();
	X && num_values;

	// value serialization
	for (size_t i=0; i<num_values; i++) {
		Data value;
		if (X.isSerializer()) value = values[i];
		uint16_t value_length = value.isUnspecified() ? 0 : value.getLength();
		X && value_length;
		if (X.isDeserializer()) value.setLength( value_length );
		X && value;
		if (X.isDeserializer()) values.push_back(value);
	}
} sznEnd();

#endif /* DHTMESSAGE_H_ */
