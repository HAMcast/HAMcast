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


class ImageNode :public QObject, public QGraphicsRectItem
{
   Q_OBJECT

private:
     QImage                  m_image;
     QPixmap                 m_pixmap;
     QParallelAnimationGroup m_group_animation ;
     QTreeWidget             m_tree;
     QDialog                 m_property_window;
     int                     m_node_id;
     QVBoxLayout             m_layout;
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


public:
     ImageNode(hamcast_node& node,QImage& image, QIcon& m_network_icon, QIcon& group_icon, QIcon& node_icon);
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

     void update_text_pos();
protected:
    void paint( QPainter *, const QStyleOptionGraphicsItem *option, QWidget *widget );
public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void start_next_animation();
};

#endif // IMAGENODE_H
