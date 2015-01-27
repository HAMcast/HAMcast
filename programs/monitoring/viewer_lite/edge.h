#ifndef EDGE_H
#define EDGE_H

#include <QGraphicsItem>
#include <QPropertyAnimation>
#include "animationellipse.h"

class ImageNode;

class Edge :public QGraphicsItem
{

private:

    QPointF sourcePoint;
    QPointF destPoint;
    QPropertyAnimation* animation;
    AnimationEllipse  ani;
    int               m_color;


public:

    /**
      * @brief to create a edge object
      * @param sourceNode = source of the edge
      * qparam destNode = destination of the edge
      */
    Edge(ImageNode *sourceNode, ImageNode *destNode,int color);
    ~Edge();

    void adjust();

    enum { Type = UserType + 2 };
    int type() const { return Type; }

   inline void startChildAnmations(){
       animation->start();
   }
   inline QPropertyAnimation * getAni(){
        return animation;
    }


   inline ImageNode* sourceNode() const {return source;}

   inline void setSourceNode(ImageNode *node)
   {
       source = node;
       adjust();
   }

   inline ImageNode* destNode() const {return dest;}

   inline void setDestNode(ImageNode *node)
   {
       dest = node;
       adjust();
   }


   ImageNode *source;
   ImageNode *dest;

    inline AnimationEllipse& get_animation() { return ani;}

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
