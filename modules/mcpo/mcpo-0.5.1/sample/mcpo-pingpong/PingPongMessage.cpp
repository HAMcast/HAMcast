#include "PingPongMessage.h"

namespace ariba {
namespace application {
namespace pingpong {

vsznDefault(PingPongMessage);

PingPongMessage::PingPongMessage() : id(0) {
}

PingPongMessage::PingPongMessage(uint8_t _id) : id(_id) {
}

PingPongMessage::~PingPongMessage(){
}

string PingPongMessage::info(){
	ostringstream os;
	os << "mcpo ping message id " << (uint32_t)id;
	return os.str();
}

}}} // namespace ariba, appplication, pingpong
