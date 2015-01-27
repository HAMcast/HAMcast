#include "client.h"


Client::Client(string ip,Network* nw):network(nw)
{
    this->ip=ip;
#ifdef HAMCAST
    network->join(ip);
#endif
}

Client::~Client(){
#ifdef HAMCAST
    network->leave(ip);
#endif
}

void Client::Send(Data& data){
    Send(data.GetData(),data.GetSize());
}

void Client::Send(unsigned char* data, int len){
    network->Send(ip,data,len);
}

string Client::GetIP(){
    return ip;
}
















//}
