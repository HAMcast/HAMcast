#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QSplashScreen>
#include <QShortcut>
#include <QGLWidget>

/** Own Classes*/
#include "ui_mainwindow.h"
#include "monitor.hpp"
#include "drawgraph.h"
#include "connect_dialog.hpp"
#include "node_details.hpp"
#include "table_widget.hpp"
#include "update_thread.hpp"


static  double scaleFactor = 1.15;
namespace Ui {
    class MainWindow;
}

Q_DECLARE_METATYPE(QList<QString>)
class MainWindow : public QMainWindow {

    Q_OBJECT

    #define GROUP_DETAIL 0
    #define NODE_LIST 1
    #define GROUP_LIST 2
    #define NODE_DETAILS 3
public:
    MainWindow(QSplashScreen* splash);
    ~MainWindow();
    void createTree(QString group, bool redraw);



protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow*                                 ui;


    /** Images and Icons */
    QImage                                          m_network_image;
    QImage                                          m_node_image;
    QImage                                          m_group_image;
    QImage                                          m_big_node_image;
    QImage                                          m_cloud_image;
    QImage                                          m_img_image;
    QImage                                          m_ip_image;
    QImage                                          m_alm_image;
    QImage                                          m_multicast_image;
    QImage                                          m_overlay_image;
    QImage                                          m_tunnel_image;

    QIcon                                           m_network_icon;
    QIcon                                           m_node_icon;
    QIcon                                           m_group_icon;
    QIcon                                           m_arrow_up;
    QIcon                                           m_arrow_down;

    QShortcut                                       m_fullscreen_zoom_in;
    QShortcut                                       m_fullscreen_zoom_out;

    int                                             m_update_rate;
    QString                                         m_current_group;

    QSplashScreen*                                  m_splash;

//    QMessageBox                                     m_connection_error;
    connect_dialog                                  m_connect_dialog;
    QTimer                                          m_timer;
    node_details                                    m_node_details;
    table_widget                                    m_node_list_widget;

    update_thread*                                  m_update_thread;

    DrawGraph*                                      m_current_graph;
    QList<QString>                                  m_node_list;
    QList<QString>                                  m_group_list;
    QMap<QString,hamcast_node>                      m_node_map;
    QMap<QString,QList<QString> >                   m_group_data;
    QMap<QString,QString>                           m_display_name;

    QMap< QString, QList< QPair< QPair< QString,QString>, QString > > > m_graph_edge_list;


    void create_icons();
    bool lcp(QString& a, QString& b);
    void connect_slots();
    void create_widgets();
    int tech_to_int(const QString& tech);
    void read_file();
    void create_node_content(QString &arg);

    int get_ip_interface(hamcast_node& n);


    QList<QPair<QList<ImageNode*>,QList<Edge*> > > decompose(QList<ImageNode*> nodes, QList<Edge*> edges);
    ImageNode* create_cloud(ImageNode* source_node, ImageNode* dest_node);
    bool compare_to_parent(hamcast_node child, QString group, hamcast_node parent);
    ImageNode * createNode(hamcast_node node, int id);


private slots:

     inline void get_group_data(QString& group){
        m_current_group = group;
        m_timer.stop();
        update_current_data();
        m_timer.start();
    }


    void update_group_list(QList<QString> group_list);
    void update_node_list(QList<QString> node_list);
    void update_node_data(hamcast_node new_node);
    void update_group_data(QString group_name, QList<QString> group_data);
    void update_group_tree(QString group, QList<QPair<QPair<QString, QString>, QString> > edge_list);

    void update_current_data();
    void connect_to();
    void on_close_button_clicked();
    void on_fullscreen_button_clicked();
    void on_window_button_clicked();
    void on_comboBox_activated(const QString &arg1);
    void config_action();

    void on_comboBox_2_activated(int index);

public slots:

    inline void close_splash(){
        m_splash->finish(this);
//        delete m_splash;
        m_connect_dialog.show();
    }
    void add_group_entrys(QList<QString> & groupList);
    void create_graphics_tree(QList<QPair<QPair<QString,QString>, QString >  >);
    void zoom_in();
    void zoom_out();
    void show_node_detail(QString name);
    void convert_node(QString node);
    void convert_edges(QString group, QString edges);
};

#endif // MAINWINDOW_H
