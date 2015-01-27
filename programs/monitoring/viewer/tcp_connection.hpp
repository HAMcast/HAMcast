#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <QObject>
#include <QTcpSocket>
#include <QtNetwork>
#include <QMessageBox>
#include <string>

class tcp_connection : public QObject
{
    Q_OBJECT

private:
    QTcpSocket      m_socket;

    void connect_slots();

private slots:
    void handel_Error(QAbstractSocket::SocketError);
    void read_data();
public:
    tcp_connection(QObject *parent);
    bool connect_to_host(QString& addr,int& port);
    void send(QString& data);
    QString receive();
    inline void close_socket(){
        m_socket.close();
    }

signals:
    void connection_error(QAbstractSocket::SocketError);
    void data_received(std::string);
public slots:

};

#endif // TCP_CONNECTION_HPP
