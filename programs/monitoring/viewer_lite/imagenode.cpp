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
#include "hamcast_interface.hpp"

ImageNode::ImageNode(hamcast_node& node,QImage& image, QIcon& network_icon, QIcon& group_icon, QIcon& node_icon, node_details &node_detail, QImage& multicast, QImage& overlay, QImage& tunnel)
    : m_hamcast_node(node), m_image(image),m_draw_rect(false), m_parent_edge(0),m_network_icon(network_icon) ,m_group_icon(group_icon),m_node_icon(node_icon),m_parent(0)
    , m_node_detail(node_detail), m_multicast_image(multicast), m_overlay_image(overlay), m_tunnel_image(tunnel),m_ip(false),m_alm(false),m_tunnel(false)
    ,m_cloud(false),m_text(0)
{
    setRect(0, 0, image.width()+50, image.height());
    setFlag(ItemSendsScenePositionChanges,true);
    setFlag(ItemSendsGeometryChanges,true);

    m_pixmap.convertFromImage(image, Qt::OrderedAlphaDither);
    m_multicast_pix.convertFromImage(m_multicast_image, Qt::OrderedAlphaDither);
    m_overlay_pix.convertFromImage(m_overlay_image, Qt::OrderedAlphaDither);
    m_tunnel_pix.convertFromImage(m_tunnel_image, Qt::OrderedAlphaDither);
    m_y = 0;
    m_x = 150;
    m_text = new QGraphicsTextItem();
    connect(&m_group_animation,SIGNAL(finished()),this,SLOT(start_next_animation()));
    check_techs();
}

ImageNode::~ImageNode(){
}

void ImageNode::set_image(QImage& image){
    m_image = image;
    setRect(0, 0, m_image.width()+50, m_image.height());
#if !defined(Q_WS_QWS)
    m_pixmap.convertFromImage(m_image, Qt::OrderedAlphaDither);
#endif
    update();
}

int ImageNode::rtti(){return imageRTTI;}

void ImageNode::update_text_pos(){
    QPolygonF p =m_text->mapToScene(pos().x(),pos().y(),boundingRect().width(),boundingRect().height());
    m_text->setPos(p.boundingRect().x()+10,p.boundingRect().y()+10);
    m_text->setZValue(1000);
    m_text->setTextWidth(80);
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
    m_y = boundingRect().height()-150;
    setZValue(20);
    if(m_draw_rect){
        p->drawRect(boundingRect());
    }
    p->drawPixmap(0,0,boundingRect().width()-50,boundingRect().height(),m_pixmap);
    if(!m_cloud){
        if(m_ip) {
            p->drawPixmap(boundingRect().width()-m_x,m_y,150,150,m_multicast_pix);
            m_y-=175;
        }
        if(m_alm) {
            p->drawPixmap(boundingRect().width()-m_x,m_y,150,150,m_overlay_pix);
            m_y-=175;
        }
        if(m_tunnel) {
            p->drawPixmap(boundingRect().width()-m_x,m_y,150,150,m_tunnel_pix);
        }
    }
}

void ImageNode::mousePressEvent(QGraphicsSceneMouseEvent *event){
    m_draw_rect = true;
    m_node_detail.fill_trees(m_hamcast_node);
    update();
}

void ImageNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    m_draw_rect = false;
    update();
}
    //    if(m_hamcast_node.is_img()){
    //        setToolTip("<p>Name = "+m_hamcast_node.get_name()+"</p>"
    //            "<ul>"
    //                "<li> <font color='#22aaff'>IMG</font></li>"
    //                "<li><img src=':/MIN_NETWORK'> Technology = "+tech_str+"</li>"
    //                "<li><img src=':/MIN_NODE'> Parent = "+QString().setNum(parent)+ "</li>"
    //                "<li><img src=':/MIN_NODE'> Children = "+QString().setNum(m_children.length())+ "</li>"
    //            "</ul>");

    //    }
    //    else{
    //        setToolTip("<p>Name = "+m_hamcast_node.get_name()+"</p>"
    //            "<ul>"
    //                "<li><img src=':/MIN_NETWORK'> Technology = "+tech_str+"</li>"
    //                "<li><img src=':/MIN_NODE'> Parent = "+QString().setNum(parent)+ "</li>"
    //                "<li><img src=':/MIN_NODE'> Children = "+QString().setNum(m_children.length())+ "</li>"
    //            "</ul>");
    //    }

void ImageNode::check_techs()
{
    QList<hamcast_interface> inter_list =  m_hamcast_node.get_interfaces();
    foreach(hamcast_interface interface, inter_list){
        QString tech = interface.get_tech();
        if(tech.contains("IP")) {
            m_ip = true;
            continue;
        }
        if(tech.contains("ALM")) {
            m_alm = true;
            continue;
        }
        if(tech.contains("tunnel")) {
            m_tunnel = true;
            continue;
        }
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
