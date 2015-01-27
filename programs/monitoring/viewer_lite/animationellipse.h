#ifndef ANIMATIONELLIPSE_H
#define ANIMATIONELLIPSE_H

#include <QObject>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>

class AnimationEllipse : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    explicit AnimationEllipse(QObject *parent = 0)
     : QObject(parent)
    {

    }

signals:

public slots:

};

#endif // ANIMATIONELLIPSE_H
