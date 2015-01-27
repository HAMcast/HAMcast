#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <string>

class tcp_connection : public QObject
{
    Q_OBJECT

private:
    QTcpSocket      m_socket;

public:
    tcp_connection(QObject *parent);
    bool connect_to_host(QString& addr,int& port);
    void send(QString& data);
    QString receive();
    inline void close_socket(){
        m_socket.close();
    }
inline QTcpSocket& get_socket(){ return m_socket;}
};

#endif // TCP_CONNECTION_HPP
