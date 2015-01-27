#ifndef DHTMESSAGE_H_
#define DHTMESSAGE_H_

#include "ariba/utility/messages.h"
#include "ariba/utility/serialization.h"
#include "ariba/Name.h"

namespace ariba_service {
namespace dht {

using ariba::utility::Message;
using_serialization;

class DhtMessage : public Message { VSERIALIZEABLE
public:
	typedef enum {
		DhtInvalid = 0,
		DhtGet = 1,
		DhtPut = 2,
		DhtPutAndGet = 3,
		DhtRemove = 4,
		DhtRepublish = 5,
		DhtAnswer = 8
	} DhtMessageType;
	
	DhtMessage();
	DhtMessage( DhtMessageType type, const std::string& key );
	DhtMessage( DhtMessageType type, const std::string& key,
			const std::string& value, uint16_t ttl = 0 );
	
	DhtMessage( DhtMessageType type, const std::string& key,
			const vector<std::string>& values, uint16_t ttl = 0 );
	
	virtual ~DhtMessage();

	DhtMessageType getType() const {
		return static_cast<DhtMessageType>(type);
	}
	
	NodeID getHashedKey() const {
		return ariba::Name(key).toNodeId();
	}

	const std::string& getKey() const {
		return key;
	}

	/// returns the first element of the key vector
	const std::string& getValue() const {
		return values.at(0);
	}
	
	/// return all values for the key
	const vector<std::string>& getValues() const {
		return values;
	}

    /// return all values for the key
    vector<std::string>& getValues() {
        return values;
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


private:
	uint8_t type;
	uint16_t ttl;
	bool replace;
	std::string key;
	vector<std::string> values;
};

}} // namespace ariba_service::dht

sznBeginDefault( ariba_service::dht::DhtMessage, X ) {
	X && type;

	// serialize tll
	X && ttl;

	// key serialization
	X && T(key);

	// store number of values
	uint16_t num_values = values.size();
	X && num_values;

	// value serialization
	for (size_t i=0; i<num_values; i++) {
		if (X.isSerializer()) {
			X && T(values[i]);
		}
		
		if (X.isDeserializer()) {
			std::string value;
			X && T(value);
			values.push_back(value);
		}
	}
} sznEnd();

#endif /* DHTMESSAGE_H_ */
