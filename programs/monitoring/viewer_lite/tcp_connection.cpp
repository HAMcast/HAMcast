#include "tcp_connection.hpp"
#include <QTcpSocket>
#include <QtNetwork>
#include <string>
#include <http_message.hpp>

tcp_connection::tcp_connection(QObject *parent) :
    QObject(parent), m_socket(this)
{
}

bool tcp_connection::connect_to_host(QString& addr,int& port){
    m_socket.abort();
    m_socket.connectToHost(addr,port);
    return m_socket.waitForConnected(2000);
}

void tcp_connection::send(QString& data){
     m_socket.write(data.toUtf8());
}

QString tcp_connection::receive(){
     m_socket.waitForReadyRead(100000);
     QString raw = m_socket.readAll();
     std::string buf =raw.toStdString();
     http_message msg(buf);

     int a =raw.length();
     int c = msg.get_header_size();
     int d = msg.get_payload_size();
     int b = msg.get_payload_size()+msg.get_header_size();
     int trying = 0;
     while( a < b){
        m_socket.waitForReadyRead(10);
        trying++;
        QString data = m_socket.readAll();
        if(data.size() > 0)
        raw = raw+data;
        if(trying >= 30){
            break;
        }
    }
     return raw;
}
