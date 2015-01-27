#ifndef PINGPONGMESSAGES_H_
#define PINGPONGMESSAGES_H_

#include <string>
#include "ariba/ariba.h"

using namespace ariba;
using std::string;

namespace ariba {
namespace application {
namespace pingpong {

using_serialization;

class PingPongMessage : public Message {
	VSERIALIZEABLE;
public:
	PingPongMessage();
	PingPongMessage( uint8_t _id, string name = string("<ping>") );
	virtual ~PingPongMessage();

	string info();
	uint8_t getid();

	inline string getName() const {
		return name;
	}
private:
	uint8_t id;
	string name;
};

}}} // namespace ariba, appplication , pingpong

sznBeginDefault( ariba::application::pingpong::PingPongMessage, X ) {
	X && id && T(name);
} sznEnd();

#endif /* PINGPONGMESSAGES_H_ */
