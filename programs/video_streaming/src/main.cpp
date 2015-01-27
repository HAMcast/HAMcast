#include <iostream>
#include <hamcast/hamcast.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

using std::cout;
using std::endl;
using std::string;
using namespace hamcast;
using namespace std;

#define MAX_MSG 5000

#define REMOTE_SERVER_ADDRESS "127.0.0.1"

struct sockaddr_in remoteServAddr;

int initReciveUDPSock(int vlcport){
    int rc; // return code
    int sd = 0;
    struct sockaddr_in  servAddr; // Socket Struct

    /* socket creation */
    sd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sd<0) {
        printf("cannot open UDP socket \n");
        return sd;
    }

    /* bind local server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(vlcport);
    rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
    if(rc<0) {
        printf("cannot bind port number %d \n",
               vlcport);
        return rc;
    }
    return sd;
}

int reciveFromSocket(unsigned char *buffer, int sd){
    int n; // msg length
    int cliLen; // client length
    int flags; // socket Flags
    struct sockaddr_in cliAddr;
    flags = 0;

    /* receive message */
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, buffer, MAX_MSG, flags,
                 (struct sockaddr *) &cliAddr,(socklen_t *) &cliLen);

    if(n<0) {
        printf("cannot receive data \n");
        return n;
    }
    return n;
}


int initSendUDPSock(int vlcport){
    int rc;
    struct sockaddr_in cliAddr;
    struct hostent *h;
    int sd = 0;
    /* get server IP address (no check if input is IP address or DNS name */
    h = gethostbyname(REMOTE_SERVER_ADDRESS);
    if(h==NULL) {
        printf("unknown host \n");
        return -1;
    }
    remoteServAddr.sin_family = h->h_addrtype;
    memcpy((char *) &remoteServAddr.sin_addr.s_addr,
           h->h_addr_list[0], h->h_length);
    remoteServAddr.sin_port = htons(vlcport);

    /* socket creation */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if(sd<0) {
        printf("cannot open socket \n");
        return sd;
    }

    /* bind any port */
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
    if(rc<0) {
        printf("cannot bind port\n");
        return rc;
    }
    return sd;
}


int sendData( const char *sendbuffer,int size, int sd){
    int flags = 0;
    int rc = 0;

    rc = sendto(sd, sendbuffer,size, flags,
                (struct sockaddr *) &remoteServAddr,
                sizeof(remoteServAddr));

    if(rc<0) {
        printf("cannot send data \n");
        close(sd);
    }
    //        printf("%d \n",rc);
    return rc;
}


void startClient(string group, int vlcport){


    int sd = initSendUDPSock(vlcport);
    if(sd < 0){
        return;
    }
    multicast_socket ms;
    ms.join(group);
    while(true){
//        std::cout << "waiting for multicast Data" << std::endl;
        multicast_packet mp = ms.receive();
        const  char* msg = reinterpret_cast< const  char*>(mp.data());
//        std::cout << mp.size() << std::endl;
        sendData(msg,mp.size(),sd);
    }
}

void startServer(string group, int vlcport){
    multicast_socket s;
    unsigned char sendbuffer[MAX_MSG];
    memset(sendbuffer,0x0,MAX_MSG);

    int sd = initReciveUDPSock(vlcport);
    if(sd <0){
        return;
    }
    while(true){
        int rc = reciveFromSocket(sendbuffer,sd);
//        std::cout << rc << std::endl;
        if(rc<0){
            continue;
        }
        s.send(group, rc, sendbuffer);
    }
    return;
}

namespace { int server = 0;
    struct option long_options[] =
    {

      {"server", no_argument,       &server, 1},
      {"address",  required_argument, 0, 'a'},
      {"port",  required_argument, 0, 'p'},


    };

}

int main(int argc, char *argv[])
{
    int c;
    int port =0;
    string address;
    bool addressset= false;
//	std::string test_string = "--server";
//	for (int i = 1; i < argc; ++i) { if (test_string == argv[i]) server = 1; }

   int option_index = 0;
         while (1)
           {

             c = getopt_long (argc, argv, "a:p:",
                              long_options, &option_index);

             // Detect the end of the options
             if (c == -1)
               break;

             switch (c)
               {
               case 0:
                 // If this option set a flag, do nothing else now.
                 if (long_options[option_index].flag != 0)
                   break;
                 printf ("option %s", long_options[option_index].name);
                 if (optarg)
                   printf (" with arg %s", optarg);
                 printf ("\n");
                 break;

               case 'a':
                 address = optarg;
                 addressset = true;
                 printf ("multicast address is `%s'\n", optarg);
                 break;

               case 'p':
                 port= atoi(optarg);
                 printf ("VLC port is `%s'\n", optarg);
                 break;
               case '?':
                 // getopt_long already printed an error message.
                 break;

               default:
                 abort ();
               }
           }

//	addressset = true;
//	address = "foo:bar";
//	port = 1;

         if(port == 0 || addressset== false){
            printf ("missing argument  \n");
            return 0;
         }

         if (server){
          printf ("Programm running in server mode \n");
          startServer(address,port);

         }
         else{
             printf ("Programm running in client mode \n");
             startClient(address,port);
         }

        

         return 0;
       }




