#ifndef PINGPONGMESSAGE_H_
#define PINGPONGMESSAGE_H_

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
	PingPongMessage( uint8_t _id );
	virtual ~PingPongMessage();

	string info();
private:
	uint8_t id;
};

}}} // namespace ariba, appplication , pingpong

sznBeginDefault( ariba::application::pingpong::PingPongMessage, X ) {
	X && id;
} sznEnd();

#endif //PINGPONGMESSAGE_H_
