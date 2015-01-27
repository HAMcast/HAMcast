#ifndef CLIENT_H
#define CLIENT_H
#include "network.h"

using namespace std;
class Client
{
public:
    Client(string ip,Network* nw);
    ~Client();
    void Send(Data& data);
    void Send(unsigned char* data, int len);
    string GetIP();
private:
    string ip;
    Network* network;

};

#endif // CLIENT_H
