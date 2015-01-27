#include "imagenode.h"
#include <QImage>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsRectItem>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QParallelAnimationGroup>
#include <QVBoxLayout>
#include "drawgraph.h"
ImageNode::ImageNode(hamcast_node& node,QImage& image, QIcon& network_icon, QIcon& group_icon, QIcon& node_icon)
    : m_hamcast_node(node), m_image(image),m_draw_rect(false), m_parent_edge(0),m_network_icon(network_icon) ,m_group_icon(group_icon),m_node_icon(node_icon),m_parent(0)
{
    // Setup Rectangle and set some flags
    setRect(0, 0, image.width(), image.height());
    setFlag(ItemSendsScenePositionChanges,true);
    setFlag(ItemSendsGeometryChanges,true);

    m_pixmap.convertFromImage(image, Qt::OrderedAlphaDither);
    m_layout.addWidget(&m_tree);
    m_property_window.setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint| Qt::WindowStaysOnTopHint |Qt::Tool);
    m_property_window.setWindowTitle(m_hamcast_node.get_name());
    m_property_window.setLayout(&m_layout);
    m_property_window.setGeometry(QRect(100,100,350,400));
    m_text = new QGraphicsTextItem();
    connect(&m_group_animation,SIGNAL(finished()),this,SLOT(start_next_animation()));
}

ImageNode::~ImageNode(){
    m_tree.clear();
}



void ImageNode::set_image(QImage& image){
    m_image = image;
    setRect(0, 0, m_image.width(), m_image.height());
#if !defined(Q_WS_QWS)
    m_pixmap.convertFromImage(m_image, Qt::OrderedAlphaDither);
#endif
    update();
}

int ImageNode::rtti(){return imageRTTI;}

void ImageNode::update_text_pos(){
    QPolygonF p =m_text->mapToScene(pos().x(),pos().y(),boundingRect().width(),boundingRect().height());
    m_text->setPos(p.boundingRect().x()+15,p.boundingRect().y()+10);
    m_text->setZValue(1000);
}

void ImageNode::add_edge(Edge *edge,bool isChild)
{
    if(isChild){

        m_child_edges << edge;
        m_children << edge->dest;
    }
    else{
        m_parent = edge->source;
        m_parent_edge = edge;

    }
    m_all_edges << edge;
    edge->adjust();
}

QVariant ImageNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        foreach (Edge *edge, m_all_edges)
            edge->adjust();
        break;
    default:
        break;
    };
    return QGraphicsItem::itemChange(change, value);
}




void ImageNode::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget * )
{  
    setZValue(20);
    if(m_draw_rect){
        p->drawRect(boundingRect());
    }
    p->drawPixmap(0,0,boundingRect().width(),boundingRect().height(),m_pixmap);
}

void ImageNode::mousePressEvent(QGraphicsSceneMouseEvent *event){
    m_tree.clear();
    m_draw_rect = true;
    create_tree();
    m_property_window.show();
}

void ImageNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    m_draw_rect = false;
}

void ImageNode::create_tree(){
    QTreeWidgetItem *qtreewidgetitem = m_tree.headerItem();
    qtreewidgetitem->setText(0, "data");
    qtreewidgetitem->setText(1, "value");
    m_tree.setColumnWidth(0,250);

    QString tech_str;
    QList<hamcast_interface> interface = m_hamcast_node.get_interfaces();
    QTreeWidgetItem* img = new QTreeWidgetItem(&m_tree);
    QString s = m_hamcast_node.is_img()?"true":"false";
    img->setText(1,s);
    img->setText(0,"IMG");
    for(int j =0 ; j < interface.length(); ++j) {

        hamcast_interface  inter = interface.at(j);

        QTreeWidgetItem* top_interfaces = new QTreeWidgetItem(&m_tree);
        top_interfaces->setIcon(0,m_network_icon);
        top_interfaces->setText(0,"Interfaces");

        QTreeWidgetItem* name = new QTreeWidgetItem(top_interfaces);
        name->setText(0,inter.get_name());
        name->setIcon(0,m_network_icon);

        QTreeWidgetItem* tech = new QTreeWidgetItem(name);
        tech->setText(1,inter.get_tech());
        tech->setText(0,"Tech");
        tech_str+= inter.get_tech();

        QTreeWidgetItem* addr = new QTreeWidgetItem(name);
        addr->setText(1,QString::fromStdString(inter.get_addr().str()));
        addr->setText(0,"Address");

        QTreeWidgetItem* id = new QTreeWidgetItem(name);
        id->setText(1,QString(QString().setNum(inter.get_id())));
        id->setText(0,"Id");

        QTreeWidgetItem* top_group = new QTreeWidgetItem(name);
        top_group->setText(0,"Groups");
        top_group->setIcon(0,m_group_icon);

        QMap<hamcast::uri,hamcast_group> groups = interface.at(j).get_groups();
        QMapIterator<hamcast::uri, hamcast_group> i(groups);
        while (i.hasNext()) {
            i.next();
            hamcast_group group = i.value();
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

    int parent = 0;
    if(m_parent !=0){
        parent = 1;
    }
    if(m_hamcast_node.is_img()){
        setToolTip("<p>Name = "+m_hamcast_node.get_name()+"</p>"
            "<ul>"
                "<li> <font color='#22aaff'>IMG</font></li>"
                "<li><img src=':/MIN_NETWORK'> Technology = "+tech_str+"</li>"
                "<li><img src=':/MIN_NODE'> Parent = "+QString().setNum(parent)+ "</li>"
                "<li><img src=':/MIN_NODE'> Children = "+QString().setNum(m_children.length())+ "</li>"
            "</ul>");

    }
    else{
        setToolTip("<p>Name = "+m_hamcast_node.get_name()+"</p>"
            "<ul>"
                "<li><img src=':/MIN_NETWORK'> Technology = "+tech_str+"</li>"
                "<li><img src=':/MIN_NODE'> Parent = "+QString().setNum(parent)+ "</li>"
                "<li><img src=':/MIN_NODE'> Children = "+QString().setNum(m_children.length())+ "</li>"
            "</ul>");
    }
}


void ImageNode::start_animations(bool first){
    if(first && m_parent_edge != 0)
        m_group_animation.addAnimation(m_parent_edge->getAni());
    if(!m_child_edges.isEmpty()){
        for(int i =0; i < m_child_edges.length();i++){

            m_group_animation.addAnimation(m_child_edges.at(i)->getAni());
        }
        m_group_animation.start();

    }
}

void ImageNode::start_next_animation(){
    for(int i =0; i < m_child_edges.length();i++){
        if(m_child_edges.at(i)->dest != this){
            m_child_edges.at(i)->dest->start_animations(false);
        }
    }
}
