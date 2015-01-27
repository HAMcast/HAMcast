#ifndef CHATTHREADS_H
#define CHATTHREADS_H


#include "QThread"


#include <string>
#include <stdlib.h>
#include "hamcast/hamcast.hpp"

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::map;

using boost::lexical_cast;

using namespace hamcast;






int init (string grouparg, string nickarg,bool serverarg);

class HeloThread : public QThread
 {
     Q_OBJECT

 protected:
     void run();


 private:
 signals:
     void memberLeft(QString);

 };


class ReciveThread : public QThread
 {
     Q_OBJECT

 protected:
     void run();
     void parse_msg (const multicast_packet mp);

 private:
 signals:
     void memberjoined(QString);
     void messeageReceived(QString,QString);



 };

#endif // CHATTHREADS_H
