
#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H
class Core;
//#include "core.h"

#include <QWidget>
#include <QLabel>


class DrawVideo : public QLabel
{
    Q_OBJECT

public:
    DrawVideo(QWidget *parent);
    ~DrawVideo();
    void setFloatBased(bool floatBased);
    void setAntialiased(bool antialiased);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void setCore(Core* core);
public slots:
    void nextAnimationFrame();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Core* core;
    unsigned char* rgb;
    bool floatBased;
    bool antialiased;
    int frameNo;

};


#endif
