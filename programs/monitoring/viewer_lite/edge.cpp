#include <QObject>
#include <QPainter>

#include "edge.h"
#include "imagenode.h"

#include <math.h>
#include <QTimer>
#include <QPixmap>


Edge::Edge(ImageNode * sourceNode, ImageNode *destNode,int color)
    : m_color(color)
{
    setAcceptedMouseButtons(0);
    source = sourceNode;
    dest = destNode;
    setZValue(-10);
    ani.setPen(QPen(Qt::black, 0));
    QColor c ;
    c.setRgb(30,30,30);
    ani.setBrush(QBrush(c));
    ani.setRect(8,8,8,8);
    animation = new QPropertyAnimation(&ani, "pos");
    animation->setDuration(1000);
    animation->setEasingCurve(QEasingCurve::Linear);
    source->add_edge(this,true);
    QPointF point(source->boundingRect().width(),source->boundingRect().height());
    QPointF newcoords =   source->mapToScene(point);
    ani.setPos(source->x()+newcoords.x()/2,source->y()+newcoords.y()/2);

    dest->add_edge(this,false);
    adjust();
}

Edge::~Edge()
{
}

void Edge::adjust()
{
    prepareGeometryChange();
    QLineF line(mapFromItem(source, source->boundingRect().width()/2, source->boundingRect().height()/2),mapFromItem(dest,dest->boundingRect().width()/2, dest->boundingRect().height()/2));
    sourcePoint = line.p1();
    destPoint = line.p2();
    animation->setStartValue(QPointF(line.p1().x()-12,line.p1().y()-12));
    animation->setEndValue(QPointF(line.p2().x()-12,line.p2().y()-12));
    prepareGeometryChange();
}

QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    qreal penWidth = 3;
    qreal extra = (penWidth)  / 2.0+10;

    QRectF rectF = QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                              destPoint.y() - sourcePoint.y()))
            .normalized()
            .adjusted(-extra, -extra, extra, extra);
    return rectF;
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget *)
{
    QColor c ;
    if (!source || !dest)
        return;
    painter->setClipRect( option->exposedRect );
    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.)))
        return;
    if(m_color == 1){
        c.setRgb(170,20,50); //rot
        painter->setPen(QPen(c,5,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(line);
    }
    if(m_color == 2){
        c.setRgb(16,54,131); // blau
        painter->setPen(QPen(c,5,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(line);
    }
    if(m_color == 3){
        c.setRgb(22,133,19); // grün
        painter->setPen(QPen(c,5,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(line);
    }
    if(m_color ==0){
        painter->setPen(QPen(Qt::black,5,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawLine(line);
    }

}
