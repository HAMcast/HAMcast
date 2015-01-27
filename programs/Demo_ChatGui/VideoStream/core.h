#ifndef CORE_H
#define CORE_H

#include "window.h"
#include <pthread.h>
#include "client.h"
#include "data.h"
#include <vector>

#include <string>
#include <queue>
#include <boost/shared_ptr.hpp>
#include "gui.h"

//#include "rtp/davc_rtp.h"

#define RAW 0
#define CODEC 1
#define NETWORK 2

#define INTER 20

#define WIDTH 640
#define HEIGHT 480


#define CONTROLPORT 43001

using namespace boost;
using namespace std;

class Window;
#ifdef HAMCAST
using namespace hamcast;
#endif

static std::queue<unsigned char *> sendq;
static pthread_mutex_t sendq_mutex = PTHREAD_MUTEX_INITIALIZER;


class Core
{

public:
    Core(GUI* gui,bool server);
    ~Core();
    //void* GetVideoDevice();
    unsigned char* GetFrame();
    void SetRate(int kbps);
    int GetWidth(){return w;}
    int GetHeight(){return h;}

    //void* ClientThread(void* args);
    void Send();
    void Receive();

    void RTP_Pack();
    bool RTP_Unpack();

    void EncodeFrame();
    unsigned char* DecodeFrame();

    void Error(string error);
    void AddClient(string ip);
    void RemoveClient(string ip);
    void ClearClients();
    void Server(bool server);

    void* GetVideoDevice(){return vd;}
    GUI* GetWindow(){return gui;}


    Data in[3];
    Data out[3];

    queue<shared_ptr<Data> > network_out;
    queue<shared_ptr<Data> > network_in;

    bool running;
    bool control;

    Network* network;
    void startServer();
#ifdef HAMCAST
    multicast_packet receive();
#endif
    void setEPSValue(int eps);
    void* vd;

private:
    bool server;
    std::vector<Client*> clients;
    int videosource;
    void InitCodec();
    int w,h;
    // werden vom codec und vom rtp_pack benötigt
    int intra;
    int temporal_levels;
    //------------------



    //rtp pack
    void *p_rtp_pack;
    //------------
    // rtp unpack
    void *p_rtp_depack;
    unsigned char paketnum;
    unsigned int rtp_unpack_time;
    int oldtimestamp;
    unsigned char *memory;
    int rtp_unpack_in;
    //---------------------

    int enc,dec;

    int target_kbps;
    int avr_interlen;
    int slicelen;
    int eps_value;

    pthread_t thread_id[5];

    GUI* gui;
    unsigned char* mem;


};





static unsigned long uptime(void)
{
struct timespec r;
if(clock_gettime(CLOCK_MONOTONIC, &r)!=0) return (time_t) -1;
return (r.tv_sec*1000+r.tv_nsec/1000000);
}

static unsigned long timeGetTime(){
        timeval tv;
        unsigned long ret=uptime();
        return ret;
}

#endif // CORE_H
