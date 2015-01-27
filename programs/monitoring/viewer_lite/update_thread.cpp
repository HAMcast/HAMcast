#include "update_thread.hpp"
#include "monitor.hpp"
#include "QTcpSocket"
update_thread::update_thread()
    :first_up(false), running(true)
{
}

void update_thread::run()
{
    QTimer t;
    t.singleShot(0, this, SLOT(do_hard_work()));
    QObject::moveToThread(this);
    this->exec();
}

void update_thread::stop()
{
    running = false;
}

void update_thread::do_hard_work()
{
    QObject::moveToThread(this);
    monitor m_monitor;
    m_monitor.moveToThread(this);
m_monitor.get_the_sock().moveToThread(this);
  bool connected = m_monitor.connect_to_collector(m_addr, m_port);
//    QTcpSocket m_sock;
//m_sock.connectToHost(m_addr,m_port);
//bool connected = m_sock.waitForConnected(100);
qDebug() << "try to update";
    if(connected){
    while(running){
        qDebug() << "update started";
        QList<QString> group_list = m_monitor.get_group_list();
        emit(group_list_update(group_list));

        foreach(QString group_name, group_list){
            QList<QString> group_data = m_monitor.get_group_data(group_name);
            emit(group_data_update(group_name,group_data));
        }

        QList<QString> node_list = m_monitor.get_node_list();
        emit(node_list_update(node_list));

        foreach(QString node_name, node_list){
            QString new_node = m_monitor.hamcast_node_string(node_name);
            emit(node_data_update(new_node));
        }

        foreach(QString group_name, group_list){
        QString edge_list = m_monitor.edge_list_string(group_name);
            emit(group_tree_update(group_name,edge_list));
        }
        if(!first_up){
            emit(update_finished());
            first_up = true;
        }
        qDebug() << "update finished";
        sleep(m_updaterate);
    }
    }
}


