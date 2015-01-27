#include "PingPongMessage.h"

namespace ariba {
namespace application {
namespace pingpong {

vsznDefault(PingPongMessage);

PingPongMessage::PingPongMessage() : id(0) {
}

PingPongMessage::PingPongMessage(uint8_t _id, string name) : id(_id),name(name) {
}

PingPongMessage::~PingPongMessage(){
}

string PingPongMessage::info(){
	return "ping pong message id " + ariba::utility::Helper::ultos(id);
}

uint8_t PingPongMessage::getid(){
	return id;
}

}}} // namespace ariba, appplication, pingpong
