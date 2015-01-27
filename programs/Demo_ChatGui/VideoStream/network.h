#ifndef NETWORK_H
#define NETWORK_H

#include <string>
#include <stdio.h> /* standard C i/o facilities */
#include <stdlib.h> /* needed for atoi() */
#include <unistd.h> /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>/* IP address conversion stuff */
#include <netdb.h>/* gethostbyname */
#include <iostream>

#include "data.h"
#include "coreexception.h"

#include <hamcast/hamcast.hpp>



#define MAXBUF 1500000
#define PORT 42001
class Network
{
public:
    Network(bool server,unsigned int port=PORT);
    ~Network();
    void Send(std::string ip,Data& data);
    void Send(std::string ip,unsigned char* data, unsigned int len);
    void Receive(Data& data);
    void Receive(unsigned char* data,unsigned int* len);


    void join(std::string group);
    void leave(std::string group);

private:

    int s_in,s_out;

    hamcast::multicast_socket sendSock;
    hamcast::multicast_socket receiveSock;

    bool server;
};

#endif // NETWORK_H
