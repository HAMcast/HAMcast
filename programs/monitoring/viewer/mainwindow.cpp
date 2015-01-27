#include "mainwindow.h"
#include <QShortcut>
#include <QShortcutEvent>
#include <QDebug>
#include <QGraphicsScene>
#include <QImage>
#include <QTreeWidgetItem>
#include <QGraphicsView>
#include <QTimer>
#include <QIcon>
#include <QtOpenGL/QGLWidget>
#include <QTabWidget>
#include <QObject>
#include <QFile>
#include <QTimer>

#include "ui_mainwindow.h"
#include "animationellipse.h"
#include "googlemaps.h"
#include "edge.h"
#include "drawgraph.h"
#include "imagenode.h"
#include "ledgend.h"
#include "QAction"

MainWindow::MainWindow(QSplashScreen* splash,QWidget *parent) :
    QMainWindow(parent),m_connection_error(NULL),m_splash(splash),
    ui(new Ui::MainWindow), m_network_image(":/MIN_NETWORK"),m_node_image(":/MIN_NODE"),
    m_group_image(":/MIN_GROUP"),m_big_node_image(":/NODE"),m_cloud_image(":/CLOUD"),m_fullscreen(false),m_img_image(":/IMG_IP_OM")
  ,m_ip_image(":/NODE_IP"),m_alm_image(":/NODE_OM"), m_connect_dialog(this), m_is_fullscreen(false),
    m_fullscreen_window(),m_fullscreen_Shortcut(&m_fullscreen_window), m_fullscreen_zoom_in(&m_fullscreen_window),
    m_fullscreen_zoom_out(&m_fullscreen_window),m_fullscreen_layout(&m_fullscreen_window),
    m_fullscreen_ledgend(&m_fullscreen_window), m_refresh_tree(false)
{
    ui->setupUi(this);
    create_widgets();
    connect_slots();
    read_file();
    create_icons();
    m_connection_error.setIcon(QMessageBox::Critical);
    create_fullscreen_widget();
}

MainWindow::~MainWindow()
{
    delete m_monitor;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::create_icons(){
    if (!m_group_image.isNull()){m_group_icon.addPixmap(QPixmap::fromImage(m_group_image), QIcon::Normal, QIcon::On);}
    if (!m_node_image.isNull()){m_node_icon.addPixmap(QPixmap::fromImage(m_node_image), QIcon::Normal, QIcon::On);}
    if (!m_network_image.isNull()){m_network_icon.addPixmap(QPixmap::fromImage(m_network_image), QIcon::Normal, QIcon::On);}
}


void MainWindow::create_widgets(){
    ui->tabWidget->addTab(&m_info_widget,"Info");
    ui->tabWidget->setTabsClosable(true);
    //    ui->tabWidget->addTab(&m_map,"NodeMap");
    //    m_map.initWeb();
    QTreeWidgetItem *qtreewidgetitem = m_info_widget.getTree()->headerItem();
    qtreewidgetitem->setText(0, "Nodes");
    qtreewidgetitem->setText(1, "");
    m_info_widget.getTree()->setColumnWidth(0,250);
    m_info_widget.getTree()->setIconSize(QSize(24,24));
    m_info_widget.getTree()->setIndentation(10);

    // fullscreen widget
}

void MainWindow:: connect_slots(){
    connect(ui->action_GoogleMap,SIGNAL(triggered()),this,SLOT(add_map()));
    connect(ui->action_Info,SIGNAL(triggered()),this,SLOT(add_info()));
    connect(m_info_widget.getGrouList(),SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(get_group_data(QListWidgetItem*)));
    connect(m_info_widget.getGrouList(),SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(onmy_listWidget_itemDoubleClicked(QListWidgetItem*)));
    QTimer::singleShot(0,this,SLOT(close_splash()));
    connect(ui->action_About_2,SIGNAL(triggered()),this,SLOT(about_aktion()));
    connect(ui->tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(tab_closed(int)));
    connect(&m_connect_dialog,SIGNAL(exit()),this,SLOT(on_actionBeenden_triggered()));
    connect(&m_connect_dialog,SIGNAL(connect()),this,SLOT(connect_to()));
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(update_current_data()));
    connect(ui->actionConfig,SIGNAL(triggered()),this,SLOT(config_action()));
    connect(ui->actionFullscreen_Tree,SIGNAL(triggered()),this,SLOT(fullscreen_window_action()));
}

void MainWindow::create_fullscreen_widget(){
    m_fullscreen_zoom_in.setKey(Qt::Key_Plus); //Or any other non modifier key
    m_fullscreen_zoom_in.setContext(Qt::ApplicationShortcut);
    m_fullscreen_zoom_out.setKey(Qt::Key_Minus); //Or any other non modifier key
    m_fullscreen_zoom_out.setContext(Qt::ApplicationShortcut);

    m_fullscreen_Shortcut.setKey(Qt::Key_F8); //Or any other non modifier key
    m_fullscreen_Shortcut.setContext(Qt::ApplicationShortcut);
    m_fullscreen_window.setLayout(&m_fullscreen_layout);
    connect(&m_fullscreen_Shortcut,SIGNAL(activated()),this, SLOT(fullscreen_window_action()));
    connect(&m_fullscreen_zoom_in,SIGNAL(activated()),this, SLOT(zoom_in()));
    connect(&m_fullscreen_zoom_out,SIGNAL(activated()),this, SLOT(zoom_out()));
}

void MainWindow::zoom_in(){


    double scaleFactor = 1.15; //How fast we zoom

    QGraphicsView *graphicsView = m_current_fullscreen->get_view();
    if(graphicsView == NULL) return;
    graphicsView->scale(scaleFactor, scaleFactor);
}

void MainWindow::zoom_out(){

    double scaleFactor = 1.15; //How fast we zoom
    QGraphicsView *graphicsView = m_current_fullscreen->get_view();
    if(graphicsView == NULL) return;
    graphicsView->scale(1.0/scaleFactor, 1.0/scaleFactor);
}

void MainWindow::fullscreen_window_action(){

    if(m_is_fullscreen){
        m_is_fullscreen = false;
        m_fullscreen_window.showNormal();
        m_fullscreen_window.hide();
        m_fullscreen_layout.removeWidget(m_current_fullscreen->get_view());
        m_current_fullscreen->showGraph(false);
        this->show();
    }
    else{
        QString t = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
        if(m_graph_list.contains(t)){
            ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
            QGraphicsView *graphicsView = m_graph_list.value(t)->get_view();
            if(graphicsView == NULL) return;
            m_current_fullscreen = m_graph_list.value(t);
            m_fullscreen_window.setWindowTitle(t);
            m_fullscreen_layout.addWidget(graphicsView);
            m_fullscreen_layout.addWidget(&m_fullscreen_ledgend);
            this->hide();
            m_fullscreen_window.showFullScreen();
            m_is_fullscreen = true;
        }
    }
}

void MainWindow::config_action(){
    m_connect_dialog.show();
}

void MainWindow::connect_to(){
    QString ip = m_connect_dialog.get_ip();
    int port = m_connect_dialog.get_port().toInt();
    int up = m_connect_dialog.get_updaterate().toInt();
    if(!ip.isEmpty() || port !=0 || up > 0){
        m_update_rate = up;
        m_monitor = new monitor(this,up,m_connection_error);
        bool connected =  m_monitor->connect_to_collector(ip,port);
        if(connected){
            m_connect_dialog.close();
            update_current_data();
            m_timer.start(up);
            this->show();
        }
        else{
            m_connect_dialog.close();
            this->show();
        }
    }
}

void MainWindow:: tab_closed(int index){
    if(index == m_current_tab_index){
        m_current_graph=0;
    }
}

void MainWindow::wheelEvent(QWheelEvent *event){scale_view(pow((double)2, -event->delta() / 1000.0));}

void MainWindow::add_group_entrys(QList<QString> & groupList){

    m_info_widget.getTree()->clear();
    QListWidget * group_widget = m_info_widget.getGrouList();
    group_widget->clear();
    foreach(QString group, groupList){
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(group);
        if (!m_group_image.isNull()){
            QIcon icon;
            icon.addPixmap(QPixmap::fromImage(m_group_image), QIcon::Normal, QIcon::On);
            newItem->setIcon(icon);
        }
        group_widget->addItem(newItem);
    }
}

void MainWindow:: add_map(){
    ui->tabWidget->addTab(&m_map,"NodeMap");
    ui->tabWidget->setCurrentIndex( ui->tabWidget->indexOf(&m_map));
}

void MainWindow::add_info(){
    int indx =  ui->tabWidget->addTab(&m_info_widget,"Info");
    ui->tabWidget->setCurrentIndex(indx);
}

QString MainWindow::read_file(){
    QFile file("data/glab_sites.txt");
    file.open(QIODevice:: ReadWrite | QIODevice::Text);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line =in.readLine();
        create_node_content(line);
    }
    return QString();
}

void MainWindow::create_node_content(QString& arg){
    QStringList strList = arg.split(" ");
    m_name_addr_list.insert(strList.at(0),strList.at(2));
    QPair<double,double> p(QString(strList.at(3)).toDouble(),QString(strList.at(4)).toDouble());
    m_addr_geo.insert(strList.at(0),p);
}

void MainWindow::about_aktion(){
    QMessageBox::about(this, tr("About Application"),
                       tr("HAMcast monitoring framework\n"
                          "for more information contact: Sebastian.Zagaria@haw-hamburg.de"
                          ));
}


void MainWindow::scale_view(qreal scaleFactor)
{
    QString t = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

    if(m_graph_list.contains(t)){
        QGraphicsView *graphicsView = m_graph_list.value(t)->get_view();
        if(graphicsView == NULL) return;
        qreal factor =    graphicsView->matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
        if (factor < 0.07 || factor > 3)
            return;

        graphicsView->scale(scaleFactor, scaleFactor);
    }
}

void MainWindow::onmy_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    QString name = item->text();
    QList<QPair<QString,QString>  > edge_list=  m_monitor->get_group_tree(name);
    if(edge_list.length() !=0)
        create_graphics_tree(edge_list,name);
    else return;
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{

    QString t = ui->tabWidget->tabText(index);

    if(m_graph_list.contains(t)){
        DrawGraph *gd = m_graph_list.value(t);
        m_graph_list.remove(t);
        delete gd;
    }
    ui->tabWidget->removeTab(index);
}

void MainWindow::create_node_entry(hamcast_node& n){
    QList<QTreeWidgetItem*> match_list = m_info_widget.getTree()->findItems(n.get_name(),Qt::MatchExactly);
    if(match_list.size() >0){
        m_info_widget.getTree()->removeItemWidget(match_list.first(),0);
        qDeleteAll(match_list.first()->takeChildren());
        delete match_list.first();
    }
    QList<hamcast_interface> interface = n.get_interfaces();
    QTreeWidgetItem* node = new QTreeWidgetItem(m_info_widget.getTree());
    node->setText(0,n.get_name());
    node->setIcon(0,m_node_icon);
    QTreeWidgetItem* img = new QTreeWidgetItem(node);
    QString s = n.is_img()?"true":"false";
    img->setText(1,s);
    img->setText(0,"IMG");
    QTreeWidgetItem* top_interfaces = new QTreeWidgetItem(node);
    top_interfaces->setIcon(0,m_network_icon);
    top_interfaces->setText(0,"Interfaces");
    for(int i =0 ; i < interface.length();i++)
    {
        hamcast::uri u (m_current_group.toStdString());
        hamcast_interface  inter = interface.at(i);
        QTreeWidgetItem* name = new QTreeWidgetItem(top_interfaces);
        name->setText(0,inter.get_name());
        name->setIcon(0,m_network_icon);
        QTreeWidgetItem* tech = new QTreeWidgetItem(name);
        tech->setText(1,inter.get_tech());
        tech->setText(0,"Tech");
        QTreeWidgetItem* addr = new QTreeWidgetItem(name);
        addr->setText(1,QString::fromStdString(inter.get_addr().str()));
        addr->setText(0,"Address");
        QTreeWidgetItem* id = new QTreeWidgetItem(name);
        id->setText(1,QString(QString().setNum(inter.get_id())));
        id->setText(0,"Id");
        QMap<hamcast::uri,hamcast_group> groups = interface.at(i).get_groups();
        if(groups.contains(u)){
            hamcast_group group = groups.value(u);
            QTreeWidgetItem* top_group = new QTreeWidgetItem(name);
            top_group->setText(0,"Groups");
            top_group->setIcon(0,m_group_icon);
            QTreeWidgetItem* gpr = new QTreeWidgetItem(top_group);
            gpr->setText(0,QString::fromStdString(group.get_name().str()));
            gpr->setIcon(0,m_group_icon);
            QTreeWidgetItem* top_parent = new QTreeWidgetItem(gpr);
            top_parent->setText(0,"Parent");
            top_parent->setIcon(0,m_node_icon);
            QTreeWidgetItem* top_children = new QTreeWidgetItem(gpr);
            top_children->setText(0,"Children");
            top_children->setIcon(0,m_node_icon);
            QTreeWidgetItem* parent = new QTreeWidgetItem(top_parent);
            parent->setText(0,QString::fromStdString(group.get_parent().str()));
            QList<hamcast::uri> children  = group.get_children();
            for(int c=0;c < children.length();c++){
                QTreeWidgetItem* child = new QTreeWidgetItem(top_children);
                child->setText(0,QString::fromStdString(children.at(c).str()));
            }
        }
    }
}

void MainWindow:: create_graphics_tree(QList<QPair<QString,QString> > edgeList, QString group){
    QList<Edge*> edge_list;
    QList<ImageNode*> images;
    QMap<QString,ImageNode*> cloud_list;
    QMap<QString,ImageNode*> node_list;
    QMap<QString, hamcast_node> node_map;
    int id = 0;
    if(m_group_nodes.contains(group)){
        node_map = m_group_nodes.value(group);
    }
    else{
        return;
    }
    DrawGraph *gd;
    if(m_graph_list.contains(m_current_group)){
        gd = m_graph_list.value(m_current_group);
        gd->set_scene(new QGraphicsScene());
    }
    else{
        QGraphicsScene * scene  = new QGraphicsScene();
        QGraphicsView * view = new QGraphicsView();
        view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        gd = new DrawGraph(view,scene,ui->tabWidget,m_current_group);
        m_graph_list.insert(m_current_group,gd);
        m_graph_edge_list.insert(m_current_group,edgeList);
    }


    for(int i = 0; i < edgeList.length();i++){
        QPair<QString,QString> edge= edgeList.at(i);

        if(node_map.contains(edge.first)){
            ImageNode *n;
            if(node_list.contains(edge.first)){
                n = node_list.value(edge.first);
            }
            else{
                n = createNode(node_map.value(edge.first),id);
                id++;
                images.append(n);
                node_list.insert(n->get_name(),n);
            }
            if(node_map.contains(edge.second)){
                ImageNode *n2;
                if(node_list.contains(edge.second)){
                    n2 = node_list.value(edge.second);
                }
                else{
                    n2 = createNode(node_map.value(edge.second),id);
                    id++;
                    images.append(n2);
                    node_list.insert(n2->get_name(),n2);
                }
                if(n->is_img() && (n2->get_hamcast_node().get_interfaces().length()==1) &&
                        (n2->get_hamcast_node().get_interfaces().at(0).get_tech().contains("IP")) ){
                    if(cloud_list.contains(n->get_name())){
                        ImageNode* cloud = cloud_list.value(n->get_name());
                        Edge* e1 = new Edge(cloud,n2,"e");
                        edge_list.append(e1);
                    }
                    else{
                        n->set_image(m_img_image);
                        ImageNode* cloud = create_cloud(n,n2);
                        cloud->setScale(0.09);
                        cloud->set_node_id(id);
                        id++;
                        images.append(cloud);
                        Edge* e0 = new Edge(n,cloud,"ec");
                        Edge* e1 = new Edge(cloud,n2,"e");
                        edge_list.append(e0);
                        edge_list.append(e1);
                        cloud_list.insert(n->get_name(),cloud);
                    }
                }
                else{
                    if(compare_to_parent(n2->get_hamcast_node(),group,n->get_hamcast_node())){
                        Edge* e1 = new Edge(n,n2,"e");
                        edge_list.append(e1);
                    }
                }
            }
        }
    }
    //    decompose(images,edge_list);

    if(images.isEmpty()){
        if(!node_map.isEmpty()){
            QMap<QString,hamcast_node> node_map = m_group_nodes.value(m_current_group);
            foreach (hamcast_node tmp, node_map){
                ImageNode* n = new ImageNode(tmp,m_big_node_image,m_network_icon,m_group_icon,m_node_icon);
                int id =0;
                n->set_node_id(id);
                n->setScale(0.12);

                images.append(n);
                node_list.insert(n->get_name(),n);
                if(n->get_hamcast_node().get_interfaces().length()==1){
                    if(n->get_hamcast_node().get_interfaces().at(0).get_tech().contains("IP")){
                        n->set_image(m_ip_image);
                    }
                    else{
                        n->set_image(m_alm_image);
                    }
                }
                else{
                    n->set_image(m_img_image);
                }
            }
        }
        else{
            m_current_group = "";
            m_graph_list.remove(gd->get_group());
            m_graph_edge_list.remove(gd->get_group());
            ui->tabWidget->find(gd->get_tab_index())->close();
        }
    }

    if(!images.isEmpty()){
        gd->drawGraph(&edge_list,&images);

        if(!time){
            gd->get_view()->scale(0.6, 0.6);
        }
        gd->showGraph(false);
        m_current_graph = gd;
        m_current_tab_index = gd->get_tab_index();
    }
}

ImageNode * MainWindow::createNode(hamcast_node node, int id){
    ImageNode *n = new ImageNode(node,m_big_node_image,m_network_icon,m_group_icon,m_node_icon);
    n->set_node_id(id);
    n->setScale(0.12);
    if(n->get_hamcast_node().get_interfaces().length()==1){
        if(n->get_hamcast_node().get_interfaces().at(0).get_tech().contains("IP")){
            n->set_image(m_ip_image);
        }
        else{
            n->set_image(m_alm_image);
        }
    }
    else{
        n->set_image(m_img_image);
    }
    return n;
}

bool MainWindow::compare_to_parent(hamcast_node child, QString group, hamcast_node parent){
    QList<hamcast_interface> l_interface = child.get_interfaces();
    hamcast::uri u(group.toStdString());
    foreach(hamcast_interface p_interface, l_interface){
        hamcast_group c_group = p_interface.findGroup(u);
        QList<hamcast_interface> parent_interfaces = parent.get_interfaces();
        foreach(hamcast_interface parent_interface, parent_interfaces){
            hamcast_group parent_group = parent_interface.findGroup(u);
            if(c_group.get_parent().compare(parent_interface.get_addr()) == 0){
                return true;
            }
            else{
                return false;
            }
        }
    }
    return false;
}

ImageNode* MainWindow::create_cloud(ImageNode* source_node, ImageNode* dest_node){
    hamcast::uri group_name(m_current_group.toStdString());
    QList<hamcast_interface> source_interface;
    QList<hamcast_interface> interface_list = source_node->get_hamcast_node().get_interfaces();
    QList<hamcast::uri> child;
    for(int i =0; i < interface_list.length();i++){
        if(interface_list.at(i).get_tech().contains("IP")){
            source_interface.append(interface_list.at(i));
            break;
        }
    }
    hamcast::uri parent_name(source_interface.at(0).get_addr());
    child.append(hamcast::uri (dest_node->get_hamcast_node().get_interfaces().at(0).get_addr()));
    hamcast_group g(group_name,parent_name,child);
    QString addr = QString::fromStdString(source_interface.at(0).get_addr().str());
    int id = source_interface.at(0).get_id();
    QString name = source_interface.at(0).get_name();
    QString tech = source_interface.at(0).get_tech();
    hamcast_interface in(addr,id,name,tech);
    in.set_group(g);
    QList<hamcast_interface> result_interface;
    result_interface.append(in);
    QString node_name = source_node->get_name()+" cloud";
    hamcast_node h_node(node_name,result_interface,false);

    QString tmp =addr.mid(5);
    int first =tmp.indexOf(".");
    QString tmp2 = tmp.mid(first+1);
    int sec =tmp2.indexOf(".");
    QString sub(tmp.mid(0,first+sec+1));
    sub+=".0.0";
    QString setter =m_name_addr_list.value("ip://"+sub);
    hamcast::uri newAddr(setter.toStdString());
    in.set_addr(newAddr);
    ImageNode * result = new ImageNode(h_node,m_cloud_image,m_network_icon,m_group_icon,m_node_icon);
    result->set_text(setter);
    return result;
}

QList<QPair<QList<ImageNode*>,QList<Edge*> > > MainWindow:: decompose(QList<ImageNode*> nodes, QList<Edge*> edges){
    QList<QPair<QList<ImageNode*>,QList<Edge*> > > result;
    for(int i =0; i < nodes.length();i++){
        QList<ImageNode*> to_go;
        QList<ImageNode*> tree_nodes;
        QList<Edge*> tree_edges;
        to_go.append(nodes.first());
        nodes.removeFirst();
        while(to_go.length()>0){
            ImageNode * node_of_interest = to_go.first();
            to_go.removeFirst();
            tree_nodes.append(node_of_interest);
            nodes.removeAt(nodes.indexOf(node_of_interest));
            Edge* parent = node_of_interest->get_parent_edge();
            QList<Edge*> children = node_of_interest->get_edges();
            if(parent !=0){
                if(!to_go.contains(parent->sourceNode()) && !tree_nodes.contains(parent->sourceNode())){
                    tree_edges.append(parent);
                    to_go.append(parent->sourceNode());
                }
            }
            for(int e = 0; e < children.length();e++){
                if(!to_go.contains(children.at(e)->destNode()) && !tree_nodes.contains(children.at(e)->destNode())){
                    tree_edges.append(children.at(e));
                    to_go.append(children.at(e)->destNode());
                }
            }
        }
        QPair<QList<ImageNode*>,QList<Edge*> > pair(tree_nodes,tree_edges);
        result.append(pair);
    }
    return result;
}

void MainWindow::update_current_data(){
    try{
        QList<QString> group_list = m_monitor->get_group_list();
        if(group_list!= m_group_list){
            m_info_widget.getTree();
            m_group_list = group_list;
            m_current_group = "";
            add_group_entrys(group_list);
            foreach(QString grp, m_current_groups){
                if(!m_group_list.contains(grp)){
                    m_current_groups.removeAt(m_current_groups.indexOf(grp));
                    m_group_nodes.remove(grp);
                }
            }


        }
        qDebug() << "group list updated";
        if(!m_current_groups.isEmpty()){
            QString group_name;
            foreach(group_name, m_current_groups){
                QList<QString> group_data = m_monitor->get_group_data(group_name);
                qDebug() << "group data updated";
                QList<hamcast_node> node_list;
                if(group_data.length() > 0){
                    for(int i =0; i < group_data.length();i++){
                        QString node_name = group_data.at(i);
                        hamcast_node h = m_monitor->get_node_data(node_name);
                        if(!h.get_name().isEmpty())
                            node_list.append(h);
                    }
                    if(m_group_nodes.contains(group_name)){
                        QMap<QString,hamcast_node> node_list_old = m_group_nodes.value(group_name);
                        if(node_list.empty()){
                            m_info_widget.getTree()->clear();
                            m_group_nodes.remove(group_name);
                            m_current_groups.removeAt(m_current_groups.indexOf(group_name));
                        }
                        if(node_list_old.values() != node_list || group_name == m_current_group){
                            if(m_refresh_tree){
                                m_info_widget.getTree()->clear();
                            }
                            QMap<QString, hamcast_node> new_map;
                            for(int n =0; n < node_list.length();n++){
                                hamcast_node h_node = node_list.at(n);
                                new_map.insert(h_node.get_name(),h_node);
                                hamcast_node old_node = node_list_old.value(h_node.get_name());
                                if(old_node == h_node && !m_refresh_tree){
                                    continue;
                                }
                                else{
                                    qDebug() << "node Updated";
                                    create_node_entry(h_node);
                                }
                            }
                            m_refresh_tree = false;
                            m_group_nodes.insert(group_name,new_map);
                        }
                        qDebug() << "node data updated";
                    }
                    else{
                        m_info_widget.getTree()->clear();
                        QMap<QString, hamcast_node> new_map;
                        for(int n =0; n < node_list.length();n++){
                            hamcast_node h_node = node_list.at(n);
                            new_map.insert(h_node.get_name(),h_node);
                            create_node_entry(h_node);
                        }
                        m_group_nodes.insert(group_name,new_map);
                        qDebug() << "node data updated";
                    }
                }
            }
        }
        foreach(DrawGraph*graph, m_graph_list){
            QString graph_group = graph->get_group();
            QList<QPair<QString,QString>  > edge_list=  m_monitor->get_group_tree(graph_group);
            if(m_graph_edge_list.value(graph->get_group())!= edge_list){
                //                qDebug()<< "Old List,";
                //                qDebug()<< m_graph_edge_list.value(graph->get_group());
                //                qDebug()<< "new List";
                //                qDebug()<< edge_list;
                m_graph_edge_list.remove(graph->get_group());
                m_graph_edge_list.insert(graph->get_group(),edge_list);

                create_graphics_tree(edge_list,graph->get_group());
            }
            if(m_group_nodes.value(graph->get_group()).empty()){
                ui->tabWidget->removeTab(graph->get_tab_index());
                m_graph_list.remove(graph->get_group());
                delete graph;

            }
        }
    }
    catch (std::exception &e){
        std::cout << e.what() << std::endl;
    }
}

//void MainWindow::update_current_data(){
//    try{
//        QList<QString> group_list = m_monitor->get_group_list();
//        if(group_list!= m_group_list){
//            m_info_widget.getTree();
//            m_group_list = group_list;
//            m_current_group = "";
//            add_group_entrys(group_list);
//            foreach(QString grp, m_current_groups){
//                if(!m_group_list.contains(grp)){
//                    m_current_groups.removeAt(m_current_groups.indexOf(grp));
//                    m_group_nodes.remove(grp);
//                }
//            }


//        }
//        qDebug() << "group list updated";
//        if(!m_current_groups.isEmpty()){
//            QString group_name;
//            foreach(group_name, m_current_groups){
//                QList<QString> group_data = m_monitor->get_group_data(group_name);
//                qDebug() << "group data updated";
//                QList<hamcast_node> node_list;
//                if(group_data.length() > 0){
//                    for(int i =0; i < group_data.length();i++){
//                        QString node_name = group_data.at(i);
//                        hamcast_node h = m_monitor->get_node_data(node_name);
//                        if(!h.get_name().isEmpty())
//                            node_list.append(h);
//                    }
//                    if(m_group_nodes.contains(group_name)){
//                        QList<hamcast_node> node_list_old = m_group_nodes.value(group_name).values();
//                        if(node_list.empty()){
//                            m_info_widget.getTree()->clear();
//                            m_group_nodes.remove(group_name);
//                            m_current_groups.removeAt(m_current_groups.indexOf(group_name));
//                        }
//                        if(node_list_old != node_list || group_name == m_current_group){
//                            m_info_widget.getTree()->clear();
//                            QMap<QString, hamcast_node> new_map;
//                            for(int n =0; n < node_list.length();n++){
//                                hamcast_node h_node = node_list.at(n);
//                                new_map.insert(h_node.get_name(),h_node);
//                                create_node_entry(h_node);
//                            }
//                            m_group_nodes.insert(group_name,new_map);
//                        }
//                        qDebug() << "node data updated";
//                    }
//                    else{
//                        m_info_widget.getTree()->clear();
//                        QMap<QString, hamcast_node> new_map;
//                        for(int n =0; n < node_list.length();n++){
//                            hamcast_node h_node = node_list.at(n);
//                            new_map.insert(h_node.get_name(),h_node);
//                            create_node_entry(h_node);
//                        }
//                        m_group_nodes.insert(group_name,new_map);
//                        qDebug() << "node data updated";
//                    }
//                }
//            }
//        }
//        foreach(DrawGraph*graph, m_graph_list){
//            QString graph_group = graph->get_group();
//            QList<QPair<QString,QString>  > edge_list=  m_monitor->get_group_tree(graph_group);
//            if(m_graph_edge_list.value(graph->get_group())!= edge_list){
////                qDebug()<< "Old List,";
////                qDebug()<< m_graph_edge_list.value(graph->get_group());
////                qDebug()<< "new List";
////                qDebug()<< edge_list;
//                m_graph_edge_list.remove(graph->get_group());
//                m_graph_edge_list.insert(graph->get_group(),edge_list);

//                create_graphics_tree(edge_list,graph->get_group());
//            }
//            if(m_group_nodes.value(graph->get_group()).empty()){
//                ui->tabWidget->removeTab(graph->get_tab_index());
//                m_graph_list.remove(graph->get_group());
//                delete graph;

//            }
//        }
//    }
//    catch (std::exception &e){
//        std::cout << e.what() << std::endl;
//    }
//}
