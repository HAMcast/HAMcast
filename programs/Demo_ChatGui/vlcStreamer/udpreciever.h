#ifndef UDPRECIEVER_H
#define UDPRECIEVER_H

#include <QThread>
#include <string>

using namespace std;
class UdpReciever : public QThread
 {
     Q_OBJECT
 public:
     UdpReciever(QString group, bool server);
 protected:
     void run();

 private:
    bool server;
    int port;
    string group;
 public slots:

 };

#endif // UDPRECIEVER_H
