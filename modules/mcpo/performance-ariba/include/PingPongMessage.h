#ifndef PINGPONGMESSAGES_H_
#define PINGPONGMESSAGES_H_

#include <string>
#include "ariba/ariba.h"

using namespace ariba;
using std::string;


using_serialization;

class PingPongMessage : public Message {
	VSERIALIZEABLE;
public:
    PingPongMessage();
    PingPongMessage( uint8_t _id, uint32_t nr, string name = string("blub") );
    virtual ~PingPongMessage();

	string info();
	uint8_t getid();

	inline string getName() const {
		return name;
	}
    inline uint32_t getNum() const {
    return nr;
    }

private:
    uint32_t nr;
	uint8_t id;
	string name;
};



sznBeginDefault( PingPongMessage, X ) {
    X &&  nr && id && T(name);
} sznEnd();

#endif /* PINGPONGMESSAGES_H_ */
