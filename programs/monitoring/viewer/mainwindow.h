#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include <stdio.h>
#include <QGraphicsScene>
#include <QListWidget>
#include <QTextEdit>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QSplashScreen>
#include <infowidget.h>
#include <googlemaps.h>
#include <QDialog>


#include "drawgraph.h"
#include "monitor.hpp"
#include "QMessageBox"
#include "hamcast_node.hpp"

#include "connect_dialog.hpp"
#include "ledgend.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QSplashScreen* splash,QWidget *parent = 0);
    ~MainWindow();
    void createTree(QString group, bool redraw);
protected:
    void changeEvent(QEvent *e);

private:
    monitor*                                m_monitor;
    QMessageBox                             m_connection_error;
    QImage                                  m_network_image;
    QImage                                  m_node_image;
    QImage                                  m_group_image;
    QImage                                  m_big_node_image;
    QImage                                  m_cloud_image;
    QIcon                                   m_network_icon;
    QIcon                                   m_node_icon;
    QIcon                                   m_group_icon;
    QImage                                  m_img_image;
    QImage                                  m_ip_image;
    QImage                                  m_alm_image;
    bool                                    m_fullscreen;
    InfoWidget                              m_info_widget;
    Ui::MainWindow                          *ui;
    QMap<QString, DrawGraph*>               m_graph_list;
    QMap<QString, QList<QPair<QString,QString>  > > m_graph_edge_list;
    QSplashScreen*                          m_splash;
    connect_dialog                          m_connect_dialog;
    QMap<QString, QString>                  m_name_addr_list;
    QMap<QString,QPair<double,double> >     m_addr_geo;
    GoogleMaps                              m_map;
    QMap<QString, QMap<QString,hamcast_node> >    m_group_nodes;
    QString                                 m_current_group;
    QList<QString>                          m_current_groups;
    QList<QString>                          m_group_data;
    QList<hamcast_node>                     m_new_nodes;
    DrawGraph*                              m_current_graph;
    QTimer                                  m_timer;
    QList<QString>                          m_group_list;
    int                                     m_current_tab_index;
    int                                     m_update_rate;
    QWidget                                 m_fullscreen_window;
    bool                                    m_is_fullscreen;
    QShortcut                               m_fullscreen_Shortcut;
    QShortcut                               m_fullscreen_zoom_in;
    QShortcut                               m_fullscreen_zoom_out;
    QVBoxLayout                             m_fullscreen_layout;
    Ledgend                                 m_fullscreen_ledgend;
    DrawGraph*                              m_current_fullscreen;
    bool                                    m_refresh_tree;
    void create_fullscreen_widget();
    void create_icons();
    void scale_view(qreal scaleFactor);
    bool lcp(QString& a, QString& b);
    void connect_slots();
    void create_widgets();
    QString read_file();
    void create_node_content(QString& arg);
    QList<QPair<QList<ImageNode*>,QList<Edge*> > > decompose(QList<ImageNode*> nodes, QList<Edge*> edges);
    ImageNode* create_cloud(ImageNode* source_node, ImageNode* dest_node);
    bool compare_to_parent(hamcast_node child, QString group, hamcast_node parent);
    ImageNode * createNode(hamcast_node node, int id);
private slots:

    void onmy_listWidget_itemDoubleClicked(QListWidgetItem* item);
    inline void on_action_Fullscreen_triggered(){
        if(!m_fullscreen){
            m_fullscreen = true;
            this->showFullScreen();
        }
        else{
            m_fullscreen = false;
            this->showNormal();
        }
    }

    inline void on_actionBeenden_triggered(){this->close();}
    void on_tabWidget_tabCloseRequested(int index);
    void wheelEvent(QWheelEvent *event);
    void about_aktion();
    void config_action();
    inline void get_group_data(QListWidgetItem * entry){
        QString group = entry->text();
        if(m_current_group != group){
            m_refresh_tree = true;
        }
        m_current_group = group;
        if(!m_current_groups.contains(group)){
                m_current_groups.append(group);
        }
        m_timer.stop();
        update_current_data();
        m_timer.start();
    }

    void create_node_entry(hamcast_node& n);
    void update_current_data();
    void connect_to();
public slots:

    inline void close_splash(){
        m_splash->finish(this);
        delete m_splash;
        m_connect_dialog.show();
    }
    void add_map();
    void add_group_entrys(QList<QString> & groupList);
    void add_info();
    void create_graphics_tree(QList<QPair<QString,QString> >, QString group);
    void tab_closed(int index);
    void fullscreen_window_action();
    void zoom_in();
    void zoom_out();
};

#endif // MAINWINDOW_H
