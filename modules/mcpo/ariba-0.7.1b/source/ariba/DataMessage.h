#ifndef DATAMESSAGE_H_
#define DATAMESSAGE_H_

#define USE_MESSAGE_UTILITY

#include <memory>
#include <inttypes.h>

// use message utility
#ifdef USE_MESSAGE_UTILITY
  #include "ariba/utility/messages.h"
  namespace ariba {
    typedef utility::Message Message;
  }
#endif

namespace ariba {

// define sequence number type
typedef uint16_t seqnum_t;

/**
 * \addtogroup public
 * @{
 * This class wraps different representations of a message. In its current
 * version is allows to specify binary data (as void*) with a size specifying
 * the number of bytes of data or an message object that can be
 * serialized if necessary. The main idea is, that simulation environments
 * do not necessarily need to serialize messages.
 *
 * For performance reasons methods of this class are inlined where possible!
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class DataMessage {
private:
	void* data; //< internal buffer pointer
	size_t size; //< internal buffer pointer size
public:
	static const DataMessage UNSPECIFIED; //< default implementation of a data message

	/**
	 * Default constructor for a data message
	 */
	inline DataMessage() {
		this->data = NULL;
		this->size = 0;
	}

	/**
	 * Constructor for a data message
	 * @param data Data buffer to carry in the message
	 * @param size Size of the buffer pointed to
	 */
	inline DataMessage( const void* data, const size_t size ) {
		this->data = const_cast<void*>(data);
		this->size = size;
	}

	/**
	 * Copy constructor for a data message
	 * @param message The other message to copy from
	 */
	inline DataMessage(const DataMessage& message){
		this->data = message.data;
		this->size = message.size;
	}

#ifdef USE_MESSAGE_UTILITY
	/**
	 * Construct a data message from a normal message
	 * @param message The normal message to store
	 */
	inline DataMessage( const Message* message ) {
		this->data = (void*)const_cast<Message*>(message);
		this->size = ~0;
	}

	/**
	 * Construct a data message from a normal message
	 * @param message The normal message to store
	 */
	inline DataMessage( const Message& message ) {
		this->data = (void*)const_cast<Message*>(&message);
		this->size = ~0;
	}

	/**
	 * Get the internal message when constructued through one
	 * @return pointer to the message
	 */
	inline Message* getMessage() const {
		if (isData()) {
			return new Message( Data((uint8_t*)data,size*8) );
		}
		return (Message*)data;
	}

	/**
	 * Conversion function to convert to Message*
	 * @return internal message
	 */
	inline operator Message* () const {
		return getMessage();
	}
#endif

	/**
	 * Is the data message a normal message?
	 * @return true, if the data message is a normal message
	 */
	inline bool isMessage() const {
		return size == ~(size_t)0;
	}

	/**
	 * Is the data message a data message
	 * @return true, if the data message is not a normal message
	 */
	inline bool isData() const {
		return !isMessage();
	}

	/**
	 * Directly access the internal data pointer
	 * @return internal data pointer
	 */
	inline void* getData() const {
		return data;
	}

	/**
	 * Get the size of the internal buffer
	 * @return internal buffer size
	 */
	inline uint32_t getSize() const {
		return size;
	}

	/**
	 * Is the data message invalid?
	 * @return true, if data message is invalid
	 */
	inline bool isUnspecified() const {
		return data == NULL;
	}

};

} // namespace ariba

/** @} */

#endif /* DATAMESSAGE_H_ */
