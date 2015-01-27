
#include "network.h"
#include <string.h>

using namespace hamcast;

Network::Network(bool server,unsigned int port)
{
    this->server=server;

}
Network::~Network()
{
}

void Network::join(std::string group){
    if(server == false){
        uri u(group);
        receiveSock.join(u);
    }
}

void Network::leave(std::string group){
    if(server==false){
        uri u(group);
        receiveSock.leave(u);
    }
}

void Network::Send(std::string ip, unsigned char* data, unsigned int len){
    uri u(ip);
    sendSock.send(u,len,data);
}

void Network::Receive(Data &data){
    unsigned int len;
    Receive(data.GetData(),&len);
    data.SetSize(len);

}
void Network::Receive(unsigned char* data,unsigned int* len){
    multicast_packet packet=receiveSock.receive();
    memcpy(data,packet.data(),packet.size());
    *len=packet.size();
}


//}
