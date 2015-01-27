/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/

/* standard headers */
#include "chat.h"
#include <stdlib.h>
#include <string>
#include <QDebug>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include <QMap>

using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::map;

using boost::lexical_cast;

using namespace hamcast;



Chat::Chat(string nicknamearg, string grouparg,bool serverarg,QListWidget* userList,QTextBrowser * textbrowser,QListWidget* questionList,QLabel* usersLabel,QLabel* questionsLabel,QObject *parent)
    : QObject(parent)
{
    this->questionList = questionList;
    this->usersLabel = usersLabel;
    this->questionsLabel = questionsLabel;
     vport = 7580;
     crport =7581;
     csport =7582;
    this->userList = userList;
    this->textbrowser = textbrowser;
    init(grouparg,nicknamearg,serverarg);
    h_thread = new HeloThread;
    r_thread = new ReciveThread;
    nickname = nicknamearg;
    connect(h_thread,SIGNAL(memberLeft(QString)),this,SLOT(deleteUser(QString)));
    connect(r_thread,SIGNAL(memberjoined(QString)),this,SLOT(addUser(QString)));
    connect(r_thread,SIGNAL(messeageReceived(QString,QString)),this,SLOT(addMessage(QString,QString)));
    this->server = serverarg;
        // put together chat receive group URI
    QString g =  QString::fromStdString(grouparg)+ ":" + QString().setNum(crport);
        crgroup = uri (g.toStdString());
//        qDebug()<< crgroup.c_str();
        if ( crgroup.empty()) {
            cerr << "Invalid group URI (chat receive)." << endl;
        }

        // put together chat send group URI
        g = QString::fromStdString(grouparg) + ":" + QString().setNum(csport);
        csgroup = uri (g.toStdString());
//         qDebug()<< csgroup.c_str();
        if ( csgroup.empty()) {
            cerr << "Invalid group URI (chat send)." << endl;
        }
        g = QString::fromStdString(grouparg) + ":" + QString().setNum(vport);
        // put together video group URI
        vgroup = uri (g.toStdString());
//         qDebug()<< vgroup.c_str();
        if (vgroup.empty()) {
            cerr << "Invalid group URI (video)." << endl;
        }
    h_thread->start();
    r_thread->start();
    QListWidgetItem* w = new QListWidgetItem(QString::fromStdString(nicknamearg));
    if(!userL.contains(QString::fromStdString(nicknamearg))){
     userList->addItem(w);
     userL.insert(QString::fromStdString(nicknamearg),w);
    }
}

void Chat::stop(){
//    s_run= false;
    h_thread->quit();
    r_thread->quit();
}

void Chat::addUser(QString user){
    QListWidgetItem* w = new QListWidgetItem(user);
    if(!userL.contains(user)){
     userList->addItem(w);
     userL.insert(user,w);
     usersLabel->setText(QString().setNum(userL.size()));
    }
}

void Chat::deleteUser(QString user){
    QListWidgetItem * w = userL.value(user);
    if(w !=0){
    userList->removeItemWidget(w);

    usersLabel->setText(QString().setNum(userL.size()));
    }
}


void Chat::sendMsg(string msg){
    string message;
    message  = "POST /message HTTP/1.0\n";
    message += "From: " + nickname + "\n";
    message += "User-Agent: HAMcastDemo/1.0\n";
    message += "Content-Type: text/text\n";
    message += "Content-Length: ";
    message += boost::lexical_cast<string>(msg.size());
    message += "\n";
    message += "\n";
    message += msg;
    if(server){
        ssock.send(crgroup, message.size(), message.c_str());
    }
    else{
        ssock.send(csgroup,message.size(), message.c_str());
    }
}

void Chat::addMessage(QString message, QString from){
    questionList->addItem(new QListWidgetItem(from+" : "+message));
//    if(server){
       questionsLabel->setText(QString().setNum(questionList->count()));
//    }
    textbrowser->append("<strong>"+from+" ( "+time.currentDateTime().time().toString()+" )"+"</strong>"+" : "+message);
}
