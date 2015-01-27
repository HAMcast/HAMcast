#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <QObject>
#include <tcp_connection.hpp>
#include <QAbstractSocket>
#include "http_message.hpp"

#include <QDebug>
#include "hamcast_node.hpp"
#include <QMap>
#include <boost/property_tree/ptree.hpp>
#include "hamcast_node.hpp"

class monitor : public QObject
{
    Q_OBJECT
private:
    tcp_connection              m_tcp_connection;

    void connect_all_slots();
    QList<QString> node_list_received(boost::property_tree::ptree& data);
    QList<QString> group_list_received(boost::property_tree::ptree& data);
    QList<QString> group_data_received(boost::property_tree::ptree& data);
    hamcast_node node_data_received(boost::property_tree::ptree& data);
    QList<QPair<QPair<QString,QString>, QString >  > group_tree_received(boost::property_tree::ptree& data);

public:
    monitor();
    ~monitor();
    bool connect_to_collector(QString& addr, int& port);
    QList<QString> get_group_data(QString & group);
    hamcast_node   get_node_data(QString & group_name);
    QList<QPair<QPair<QString,QString>, QString >  > get_group_tree(QString & group_name);
    QList<QString> get_group_list();
    QList<QString> get_node_list();
    QString hamcast_node_string(QString& name);
    QString edge_list_string(QString& group);
inline QTcpSocket& get_the_sock(){
    return m_tcp_connection.get_socket();
}
};

#endif // MONITOR_HPP
