#ifndef UPDATE_THREAD_HPP
#define UPDATE_THREAD_HPP

#include <QObject>
#include <QThread>
#include <QList>
#include <QPair>
#include <QTimer>
#include "monitor.hpp"

class update_thread : public QThread
{
    Q_OBJECT
private:
    int m_updaterate;
    bool running;
    QString m_addr;
    int m_port;
    bool first_up;

public:
    update_thread();
    inline void init(int& up, QString& addr, int& port ){
        m_updaterate = up;
        m_addr = addr;
        m_port = port;
    }

    void run();
    void stop();
public slots:
    void do_hard_work();
signals:
    void group_list_update(QList<QString> group_list);
    void group_data_update(QString group_name ,QList<QString> group_data);
    void node_list_update(QList<QString> node_list);
    void node_data_update(QString node);
    void group_tree_update(QString group,QString edge_list);
    void update_finished();

};

#endif // UPDATE_THREAD_HPP
