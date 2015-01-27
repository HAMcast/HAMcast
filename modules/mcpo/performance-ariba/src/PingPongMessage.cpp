#include "PingPongMessage.h"



vsznDefault(PingPongMessage);

PingPongMessage::PingPongMessage() : id(0) {
}

PingPongMessage::PingPongMessage(uint8_t _id, uint32_t nr, string name) : id(_id),name(name), nr(nr) {
}

PingPongMessage::~PingPongMessage(){
}

string PingPongMessage::info(){
	return "ping pong message id " + ariba::utility::Helper::ultos(id);
}

uint8_t PingPongMessage::getid(){
	return id;
}

