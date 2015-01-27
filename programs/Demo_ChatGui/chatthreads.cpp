#include "chatthreads.h"
#include <hamcast/hamcast.hpp>
#include <QDebug>



static  string group = "ip://239.201.108.1";
 static  string vport ("7580");
  static string crport ("7581");
  static string csport ("7582");

static   hamcast::uri crgroup;
static   hamcast::uri csgroup;
static   hamcast::uri vgroup;

 static  bool s_run = true;
 static  string nickname;
 static  int helo_timer = 5000;
 static   map<string,int> s_members;
 static  boost::mutex s_mutex;
 static   int max_nohello = 5;
 static  bool server = true;

int init (string grouparg, string nickarg, bool serverarg)
{

server = serverarg;
    // put together chat receive group URI
    crgroup = uri (grouparg + ":" + crport);
    qDebug()<< crgroup.c_str();
    if ( crgroup.empty()) {
        cerr << "Invalid group URI (chat receive)." << endl;
    }

    // put together chat send group URI
    csgroup = uri (grouparg + ":" + csport);
     qDebug()<< csgroup.c_str();
    if ( csgroup.empty()) {
        cerr << "Invalid group URI (chat send)." << endl;
    }

    // put together video group URI
    vgroup = uri (grouparg + ":" + vport);
     qDebug()<< vgroup.c_str();
    if (vgroup.empty()) {
        cerr << "Invalid group URI (video)." << endl;
    }
    if(!nickarg.empty()){
        nickname =  nickarg;
    }


}



void HeloThread::run()
{
    hamcast::multicast_socket s;
    string helo = "POST /hello HTTP/1.0\n";
    helo += "From: " + nickname + "\n";
    helo += "User-Agent: HAMcastDemo/1.0\n";
    helo += "Content-Length: 0\n";
    helo += "\n";
    while (s_run) {
        if(server){
            s.send (crgroup, helo.size(), helo.c_str());
        }
        else{
            s.send (csgroup, helo.size(), helo.c_str());
        }

        boost::system_time tout = boost::get_system_time();
        tout += boost::posix_time::milliseconds(helo_timer);
        boost::system_time sleep_tout;
        do {
            sleep_tout = boost::get_system_time();
            sleep_tout += boost::posix_time::milliseconds(100);
            boost::this_thread::sleep(sleep_tout);
        } while (s_run && (sleep_tout < tout));
        s_mutex.lock();
        map<string, int>::iterator it;
        for (it = s_members.begin(); it != s_members.end(); ++it) {
            it->second--;
            if (it->second < 0)
                s_members.erase(it);
            emit(memberLeft(QString::fromStdString(it->first)));
        }
        s_mutex.unlock();
    }
}




void ReciveThread::run ()
{
    hamcast::multicast_socket s;
    if(server){
        s.join(csgroup);
    }
    else{
        s.join(crgroup);
        s.join(vgroup);
    }


    hamcast::multicast_packet mp;
    while (s_run)
    {
        if (s.try_receive(mp, 50))
        {
            if (server) {
                s.send (crgroup, mp.size(), mp.data());
                parse_msg(mp);

            }
            if (mp.from() == crgroup && !server) {
                parse_msg(mp);
            }
        }
        // else: check flag again
    }
}

void ReciveThread::parse_msg (const multicast_packet mp)
{
    string tmp;
    const char* msg = reinterpret_cast<const char*>(mp.data());
    std::copy(msg, msg + mp.size(),
                std::back_insert_iterator<std::string>(tmp));
    size_t p_post = tmp.find ("POST",0);
    size_t p_from = tmp.find ("From:", 0);
    bool valid = true;
    if ((p_post != string::npos) && (p_from != string::npos)) {
        size_t p_delim = tmp.find ('\n', p_from);
        string from = tmp.substr ((p_from+6), (p_delim-p_from-6));
        size_t p_hello = tmp.find ("/hello", p_post);
        size_t p_message = tmp.find ("/message", p_post);
        if (p_hello != string::npos) {
            s_mutex.lock();
            map<string, int>::iterator it = s_members.find(from);
            if (it != s_members.end()) {
                it->second++;
            } else {
                s_members.insert (std::pair<string, int>(from, max_nohello));
                emit(memberjoined(QString::fromStdString(from)));
            }
            s_mutex.unlock();
        } else if (p_message != string::npos) {
            string message;
            size_t p_size = tmp.find ("Content-Length", 0);
            if (p_size != string::npos) {
                p_delim = tmp.find ('\n', p_size);
                string stmp = tmp.substr (p_size+15,(p_delim-p_size-15));
                unsigned long usize = strtoul (stmp.c_str(), NULL, 10);
                message = tmp.substr(tmp.size()-usize, usize);
            }
            message += '\n';
            emit(messeageReceived(QString::fromStdString(message),QString::fromStdString(from)));
            cout << from << ": " << message << endl;
        } else {
            valid = false;
        }
    } else {
        valid = false;
    }
}
