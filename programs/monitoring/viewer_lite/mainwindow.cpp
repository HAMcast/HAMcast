#include "mainwindow.h"

#include <QMainWindow>
#include <QObject>
#include <QSplashScreen>
#include <QShortcut>
#include <QGLWidget>
#include <boost/property_tree/ptree.hpp>
#include <QFile>

/** Own Classes*/
#include "ui_mainwindow.h"
#include "monitor.hpp"
#include "drawgraph.h"
#include "connect_dialog.hpp"
#include "node_details.hpp"
#include "table_widget.hpp"



void igraph_error_hnd (const char * reason, const char * file,
                       int line, int igraph_errno){
    std::cout << reason << std::endl;
}

MainWindow::MainWindow(QSplashScreen* splash)
    : m_splash(splash),
      ui(new Ui::MainWindow), m_network_image(":/MIN_NETWORK"),m_node_image(":/MIN_NODE"),
      m_group_image(":/MIN_GROUP"),m_big_node_image(":/NODE"),m_cloud_image(":/CLOUD"),m_img_image(":/IMG")
    ,m_ip_image(":/NODE_IP"),m_alm_image(":/NODE_OM"), m_connect_dialog(this),m_fullscreen_zoom_in(this),
      m_fullscreen_zoom_out(this), m_arrow_up(":/UP"), m_arrow_down(":/DOWN"), m_node_details(m_network_icon,m_group_icon,m_node_icon,this),
      m_multicast_image(":/MULTICAST"), m_overlay_image(":/OVERLAY"),m_tunnel_image(":/TUNNEL"),m_current_graph(0), m_node_list_widget(m_node_icon)
{
    ui->setupUi(this);
    m_update_thread = new update_thread();
    igraph_set_error_handler(igraph_error_hnd);
    create_widgets();
    connect_slots();
    create_icons();
//    m_connection_error.setIcon(QMessageBox::Critical);

    /** some magic that does nothing*/
    ui->frame_6->hide();
    ui->window_button->setIcon(m_arrow_up);
    ui->frame_6->raise();
    ui->window_button->raise();
    ui->graphicsView->lower();

    ui->info_layout->addWidget(&m_node_details);
    ui->info_layout->addWidget(&m_node_list_widget);
    m_node_list_widget.hide();
    m_node_details.hide();
    read_file();
}

MainWindow::~MainWindow()
{
    if(m_update_thread != NULL){
        m_update_thread->stop();
         m_update_thread->exit();
        m_update_thread->wait();
        m_update_thread->deleteLater();
    }
}

void MainWindow::read_file(){
    QFile file("glab_sites.txt");
    file.open(QIODevice:: ReadWrite | QIODevice::Text);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line =in.readLine();
        create_node_content(line);
    }
}

void MainWindow::create_node_content(QString& arg){
    QStringList strList = arg.split(" ");
    m_display_name.insert(strList.at(0),strList.at(1));
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
    ui->graphicsView->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    ui->graphicsView->setScene(new QGraphicsScene());
    ui->graphicsView->show();
    m_fullscreen_zoom_in.setKey(Qt::Key_Plus); //Or any other non modifier key
    m_fullscreen_zoom_in.setContext(Qt::ApplicationShortcut);
    m_fullscreen_zoom_out.setKey(Qt::Key_Minus); //Or any other non modifier key
    m_fullscreen_zoom_out.setContext(Qt::ApplicationShortcut);

    connect(&m_fullscreen_zoom_in,SIGNAL(activated()),this, SLOT(zoom_in()));
    connect(&m_fullscreen_zoom_out,SIGNAL(activated()),this, SLOT(zoom_out()));
}

int MainWindow::tech_to_int(const QString &tech)
{
    if(tech.contains("IP")) return 1;
    if(tech.contains("ALM")) return 2;
    if(tech.contains("tunnel")) return 3;
    return 0;
}

void MainWindow::update_current_data(){

    try{
        qDebug() << "Update started";
//        update_group_list();
//        update_group_data();
//        update_node_list();
//        update_node_data();
//        update_group_tree();
        qDebug() << "Update ended";
    }
    catch(exception & e){
        std::cout << e.what() << std::endl;
    }
}

void MainWindow:: connect_slots(){

    qRegisterMetaType<QList<QString> >("my string list");
    connect(m_update_thread,SIGNAL(group_list_update(QList<QString>)),this,SLOT(update_group_list(QList<QString>)),Qt::QueuedConnection);
    connect(m_update_thread,SIGNAL(group_data_update(QString,QList<QString>)),this,SLOT(update_group_data(QString,QList<QString> )),Qt::QueuedConnection);
    connect(m_update_thread,SIGNAL(node_list_update(QList<QString>)),this,SLOT(update_node_list(QList<QString>)),Qt::QueuedConnection);
    connect(m_update_thread,SIGNAL(node_data_update(QString)),this,SLOT(convert_node(QString)),Qt::QueuedConnection);
    connect(m_update_thread,SIGNAL(group_tree_update(QString,QString)),this,SLOT(convert_edges(QString,QString)),Qt::QueuedConnection);
//    connect(m_update_thread,SIGNAL(update_finished()),this,SLOT(close_splash()));
    QTimer::singleShot(0,this,SLOT(close_splash()));
    connect(&m_connect_dialog,SIGNAL(exit()),this,SLOT(on_close_button_clicked()));
    connect(&m_connect_dialog,SIGNAL(connect()),this,SLOT(connect_to()));
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(update_current_data()));
    connect(&m_node_list_widget,SIGNAL(item_list_double_clicked(QString)),this, SLOT(show_node_detail(QString)));
}

void MainWindow::update_group_list(QList<QString> group_list)
{
    qDebug() << "called";
    if(group_list!= m_group_list){
        m_group_list = group_list;
        m_current_group = "";
        add_group_entrys(group_list);
    }
}

void MainWindow::update_group_data(QString group_name ,QList<QString> group_data)
{
    m_group_data.insert(group_name,group_data);
}

void MainWindow::update_node_list(QList<QString> node_list)
{  
    if(node_list != m_node_list){
        m_node_list = node_list;
    }
}

void MainWindow::update_node_data(hamcast_node new_node)
{
        if(m_node_map.contains(new_node.get_name())){
            hamcast_node old_node = m_node_map.value(new_node.get_name());
            if(new_node == old_node){
                return;
            }
            else{
                m_node_map.insert(new_node.get_name(),new_node);
            }
        }
        else{
            m_node_map.insert(new_node.get_name(),new_node);
        }
}

void MainWindow::update_group_tree(QString group, QList<QPair<QPair<QString,QString>, QString > > edge_list )
{
    if(!m_current_group.isEmpty()){
        if(m_current_group == group){
            QList<QPair<QPair<QString,QString>, QString > > old_list = m_graph_edge_list.value(group);
        if(old_list!= edge_list){
            m_graph_edge_list.insert(group,edge_list);
            create_graphics_tree(edge_list);
            return;
        }
       }
    }
    m_graph_edge_list.insert(group,edge_list);

}

int MainWindow::get_ip_interface(hamcast_node &n)
{
    for(int i =0; i < n.get_interfaces().length();i++){
        hamcast_interface inter = n.get_interfaces().at(i);
        if(inter.get_tech().contains("IP")){
            return i;
        }
    }
    return -1;
}

void MainWindow::convert_node(QString node){
    std::string b = node.toStdString();
    qDebug() << node;
    hamcast_node n(b);
    update_node_data(n);
}

void MainWindow::convert_edges(QString group, QString edges){
    boost::property_tree::ptree pt;
    std::stringstream input(edges.toStdString());
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);

        QList<QPair<QPair<QString,QString>, QString >  > edge_list;
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("edges")){
            std::string key = v.first.data();
            boost::property_tree::ptree tmp = pt.get_child("edges."+key);
            QPair<QString,QString> edge;
            edge.first = QString::fromStdString(tmp.get("from",""));
            edge.second = QString::fromStdString(tmp.get("to",""));
            QPair<QPair<QString,QString>, QString > full;
            full.first = edge;
            full.second = QString::fromStdString(tmp.get("tech",""));
            edge_list.append(full);
        }
        update_group_tree(group,edge_list);
    }
    catch(std::exception & e){

    }

}

void MainWindow::zoom_in(){
    ui->graphicsView->scale(scaleFactor, scaleFactor);
}

void MainWindow::zoom_out(){
    ui->graphicsView->scale(1.0/scaleFactor, 1.0/scaleFactor);
}

void MainWindow::show_node_detail(QString name)
{
    m_node_list_widget.hide();
    m_node_details.show();
    ui->window_button->setIcon(m_arrow_down);
    ui->frame_6->show();
    m_node_details.fill_trees(m_node_map.value(name));
    ui->comboBox_2->setCurrentIndex(NODE_DETAILS);
}


void MainWindow::config_action(){
    m_connect_dialog.show();
}

void MainWindow::connect_to(){
    QString ip = m_connect_dialog.get_ip();
    int port = m_connect_dialog.get_port().toInt();
    int up = m_connect_dialog.get_updaterate();
    if(!ip.isEmpty() || port !=0 || up > 0){
        m_update_rate = up*1000;
        m_connect_dialog.close();
        m_update_thread->init(up,ip,port);
        m_update_thread->moveToThread(m_update_thread);
        m_update_thread->start();
//        this->showFullScreen();
        this->show();
    }
}

void MainWindow::add_group_entrys(QList<QString> & groupList){
    ui->comboBox->clear();
 ui->comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    foreach(QString group, groupList){
        ui->comboBox->addItem(group);
    }

}

void MainWindow:: create_graphics_tree(QList<QPair<QPair<QString, QString>, QString> > edgeList){
    QList<Edge*> edge_list;
    QList<ImageNode*> images;
    QMap<QString,ImageNode*> cloud_list;
    QMap<QString,ImageNode*> node_list;
    QMap<QString, hamcast_node> node_map = m_node_map;
    int id = 0;
    if(m_current_graph != NULL){
        m_current_graph->clear_graph();
    }
    else{
        QGraphicsScene * scene  = new QGraphicsScene();
        m_current_graph = new DrawGraph(ui->graphicsView,scene,m_current_group);
    }

    for(int i = 0; i < edgeList.length();i++){
        QPair<QString,QString> edge= edgeList.at(i).first;
        QString techno = edgeList.at(i).second;
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
//                if(n->is_img() && (n2->get_hamcast_node().get_interfaces().length()==1) &&
//                        (n2->get_hamcast_node().get_interfaces().at(0).get_tech().contains("IP")) ){
//                    if(cloud_list.contains(n->get_name())){
//                        ImageNode* cloud = cloud_list.value(n->get_name());
//                        Edge* e1 = new Edge(cloud,n2,tech_to_int(techno));
//                        edge_list.append(e1);
//                    }
//                    else{
//                        n->set_image(m_img_image);
//                        ImageNode* cloud = create_cloud(n,n2);
//                        cloud->setScale(0.12);
//                        cloud->set_node_id(id);
//                        id++;
//                        images.append(cloud);
//                        Edge* e0 = new Edge(n,cloud,tech_to_int(techno));
//                        Edge* e1 = new Edge(cloud,n2,tech_to_int(techno));
//                        edge_list.append(e0);
//                        edge_list.append(e1);
//                        cloud_list.insert(n->get_name(),cloud);
//                    }
//                }
                if(n->is_img() && techno.contains("IP")){
                    if(cloud_list.contains(n->get_name())){
                        ImageNode* cloud = cloud_list.value(n->get_name());
                        Edge* e1 = new Edge(cloud,n2,tech_to_int(techno));
                        edge_list.append(e1);
                    }
                    else{
                        n->set_image(m_img_image);
                        ImageNode* cloud = create_cloud(n,n2);
                        cloud->setScale(0.12);
                        cloud->set_node_id(id);
                        id++;
                        images.append(cloud);
                        Edge* e0 = new Edge(n,cloud,tech_to_int(techno));
                        Edge* e1 = new Edge(cloud,n2,tech_to_int(techno));
                        edge_list.append(e0);
                        edge_list.append(e1);
                        cloud_list.insert(n->get_name(),cloud);
                    }
                }
                else{
                    if(compare_to_parent(n2->get_hamcast_node(),m_current_group,n->get_hamcast_node())){
                        Edge* e1 = new Edge(n,n2,tech_to_int(techno));
                        edge_list.append(e1);
                    }
                }
            }
        }
    }
    if(!images.isEmpty()){
        m_current_graph->drawGraph(&edge_list,&images);

        if(!time){
            //            m_current_graph->get_view()->scale(0.6, 0.6);
        }
        m_current_graph->showGraph();
    }
}

ImageNode * MainWindow::createNode(hamcast_node node, int id){
    ImageNode *n = new ImageNode(node,m_big_node_image,m_network_icon,m_group_icon,m_node_icon, m_node_details,m_multicast_image,m_overlay_image,m_tunnel_image);
    n->set_node_id(id);

    if(n->get_hamcast_node().is_img()) {
        n->set_image(m_img_image);
    }
    n->setScale(0.12);
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
                continue;
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

    //    QString tmp =addr.mid(5);
    //    int first =tmp.indexOf(".");
    //    QString tmp2 = tmp.mid(first+1);
    //    int sec =tmp2.indexOf(".");
    //    QString sub(tmp.mid(0,first+sec+1));
    //    sub+=".0.0";
    //    QString setter =m_name_addr_list.value("ip://"+sub);
    //    hamcast::uri newAddr(setter.toStdString());
    in.set_addr(source_interface.at(0).get_addr());
    ImageNode * result = new ImageNode(h_node,m_cloud_image,m_network_icon,m_group_icon,m_node_icon,m_node_details,m_multicast_image,m_overlay_image,m_tunnel_image);
    //    result->set_text(QString::fromStdString(source_interface.at(0).get_addr().str()));
    result->set_cloud();
    QString finder = QString::fromStdString(source_interface.at(0).get_addr().str());
    QString thisname = m_display_name.value(finder);
    result->set_text(thisname);
    result->update_text_pos();

    //    result->set_text(source_node->get_name());
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



void MainWindow::on_close_button_clicked()
{
    this->close();
}

void MainWindow::on_fullscreen_button_clicked()
{
    if(!this->isFullScreen())
        this->showFullScreen();
    else{
        this->showNormal();
    }
}


void MainWindow::on_window_button_clicked()
{
    if(ui->frame_6->isHidden())
    {

        ui->frame_6->show();
        ui->frame_6->raise();
        ui->window_button->raise();
        ui->window_button->setIcon(m_arrow_down);
    }
    else{
        ui->window_button->setIcon(m_arrow_up);
        ui->frame_6->hide();
    }
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    QString name = arg1;
    m_current_group = name;
    QList<QPair<QPair<QString,QString>, QString >  > edge_list=  m_graph_edge_list.value(name);
    if(edge_list.length() !=0)
        create_graphics_tree(edge_list);
    else return;
}

void MainWindow::on_comboBox_2_activated(int index)
{
    qDebug() << index;
    switch(index){
    case GROUP_DETAIL: {
        m_node_list_widget.hide();
        m_node_details.hide();
        break;
    }
    case NODE_LIST: {
        m_node_list_widget.addList(m_node_list);
        m_node_list_widget.show();
        m_node_details.hide();
        ui->frame_6->show();
        break;
    }
    case GROUP_LIST: {
        m_node_list_widget.hide();
        m_node_details.hide();
        break;
    }
    case NODE_DETAILS:{
        m_node_list_widget.hide();
        m_node_details.show();
        ui->window_button->setIcon(m_arrow_down);
        ui->frame_6->show();
        break;
    }
    default: break;
    }
}
