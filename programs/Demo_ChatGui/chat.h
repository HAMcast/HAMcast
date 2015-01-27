#ifndef CHAT_H
#define CHAT_H
#include <stdlib.h>

#include <string>
#include <QThread>
/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "chatthreads.h"
#include <QListWidget>
#include <QMap>
#include <QTextBrowser>
#include <QDateTime>
#include <QLabel>
using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::map;

using boost::lexical_cast;

using namespace hamcast;



class Chat : public QObject
{
    Q_OBJECT
public:
    Chat(string nicknamearg, string grouparg,bool serverarg,QListWidget* userList,QTextBrowser * textbrowser,QListWidget* questionList,QLabel* usersLabel,QLabel* questionsLabel,QObject *parent = 0);
    void stop();
    void sendMsg(string msg);

private:
    HeloThread * h_thread;
    ReciveThread * r_thread;
    hamcast::multicast_socket ssock;
    QListWidget* userList;
    QMap<QString,QListWidgetItem*> userL;
    QTextBrowser * textbrowser;
    QLabel* questionsLabel;
    bool server;
    QLabel* usersLabel;
     hamcast::uri crgroup;
       hamcast::uri csgroup;
       hamcast::uri vgroup;
       string nickname;
  int vport ;
  int crport ;
  int  csport;
  QDateTime time;
  QListWidget* questionList;
public slots:
    void addUser(QString user);
    void deleteUser(QString user);
    void addMessage(QString message,QString from);
};


#endif // CHAT_H
