#ifndef IMAGENODE_H
#define IMAGENODE_H


#include <QImage>
#include <QPixmap>
#include <QGraphicsRectItem>
#include <QTreeWidget>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <edge.h>
#include <QParallelAnimationGroup>
#include <QObject>
#include <QDialog>
#include "drawgraph.h"
#include <QGraphicsTextItem>
#include <hamcast_interface.hpp>
#include <QVBoxLayout>
#include "hamcast_node.hpp"
#include "node_details.hpp"
#include "hamcast_interface.hpp"


class ImageNode :public QObject, public QGraphicsRectItem
{
   Q_OBJECT

private:
     QImage                  m_image;
     QImage                  m_multicast_image;
     QImage                  m_overlay_image;
     QImage                  m_tunnel_image;
     QPixmap                 m_pixmap;
     QPixmap                 m_multicast_pix;
     QPixmap                 m_overlay_pix;
     QPixmap                 m_tunnel_pix;
     QParallelAnimationGroup m_group_animation ;
     int                     m_node_id;
     hamcast_node            m_hamcast_node;
     QList<Edge*>            m_child_edges;
     Edge*                   m_parent_edge;
     QList<Edge*>            m_all_edges;
     bool                    m_draw_rect;
     ImageNode*              m_parent;
     QList<ImageNode*>       m_children;
     QIcon&                  m_network_icon;
     QIcon&                  m_group_icon;
     QIcon&                  m_node_icon;
     void create_tree();
     QGraphicsTextItem*      m_text;
     node_details&           m_node_detail;

     bool                   m_ip;
     bool                   m_alm;
     bool                   m_tunnel;
     bool                   m_cloud;

     int                    m_y;
     int                    m_x;

     void check_techs();

public:
     ImageNode(hamcast_node& node,QImage& image, QIcon& m_network_icon, QIcon& group_icon, QIcon& node_icon, node_details& node_detail, QImage& multicast,
               QImage& overlay, QImage& tunnel);
     ~ImageNode();
     int rtti () ;
     static const int imageRTTI = 984376;

     void set_image(QImage& image);

     void add_edge(Edge *edge,bool is_child);

     void start_animations(bool);
     QVariant itemChange(GraphicsItemChange change, const QVariant &value);
     void adjust_edges();

     inline  ImageNode* get_parent(){return m_parent;}
     inline Edge* get_parent_edge(){return m_parent_edge;}
     inline QList<Edge*> & get_children_edges(){return m_child_edges;}
     inline const QList<ImageNode*> & get_children(){return m_children;}
     inline const bool& is_img(){ return m_hamcast_node.is_img();}
     inline void set_node_id(int & node_id){m_node_id = node_id;}
     inline const int& get_node_id(){return m_node_id;}
     inline const QString& get_name(){return m_hamcast_node.get_name();}
     inline const QList<Edge*> & get_edges(){return m_all_edges;}
     inline hamcast_node& get_hamcast_node(){ return m_hamcast_node;}
     inline void set_text(QString text){m_text->setPlainText(text);}
     inline QGraphicsTextItem* get_text_item(){return m_text;}
     inline void set_cloud(){m_cloud=true;}
     void update_text_pos();
protected:
    void paint( QPainter *, const QStyleOptionGraphicsItem *option, QWidget *widget );
public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void start_next_animation();
};

#endif // IMAGENODE_H
