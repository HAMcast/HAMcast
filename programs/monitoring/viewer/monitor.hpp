#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <QObject>
#include <tcp_connection.hpp>
#include <QTimer>
#include <QMessageBox>
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
    int                         m_update_rate;
    QTimer                      m_refresh_timer;
    QMessageBox&                m_connection_error;
    QMap<QString, hamcast_node> m_node_map;
    QList<QString>              m_group_list;
    QList<QString>              m_group_data;
    std::string                 m_group_name;
    QString                     m_current_group;

    void connect_all_slots();

    QList<QString> group_list_received(boost::property_tree::ptree& data);
    QList<QString> group_data_received(boost::property_tree::ptree& data);
    hamcast_node node_data_received(boost::property_tree::ptree& data);
    QList<QPair<QString,QString>  > group_tree_received(boost::property_tree::ptree& data);

public:
    monitor(QObject *parent, int& update_rate,QMessageBox& connection_error);
    ~monitor();
    bool connect_to_collector(QString& addr, int& port);

    inline QMap<QString,hamcast_node> get_nodes(){
        return m_node_map;
    }


    QList<QString> get_group_data(QString & group);
    hamcast_node   get_node_data(QString & group_name);
    QList<QPair<QString,QString>  > get_group_tree(QString & group_name);
    QList<QString> get_group_list();
    inline void set_current_group(QString group){
          m_current_group = group;
    }

private slots:
    void handel_connection_error(QAbstractSocket::SocketError err);
};

#endif // MONITOR_HPP
