#include <QtGui>

#include "DrawVideo.h"
#include "VideoIn.h"
#include "ColorFormat.h"
#include "core.h"

#include <stdlib.h>


DrawVideo::DrawVideo(QWidget *parent)
    : QLabel(parent)
{


    floatBased = false;
    antialiased = false;
    frameNo = 0;

    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initColorLUT();
    rgb= new unsigned char[2000*2000*4]; //
}
DrawVideo::~DrawVideo(){
    delete rgb;
}

void DrawVideo::setCore(Core* core){
    this->core = core;
}

void DrawVideo::setFloatBased(bool floatBased)
{
    this->floatBased = floatBased;
    update();
}

void DrawVideo::setAntialiased(bool antialiased)
{
    this->antialiased = antialiased;
    update();
}

QSize DrawVideo::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize DrawVideo::sizeHint() const
{
    return QSize(640, 480);
}



void DrawVideo::nextAnimationFrame()
{
    ++frameNo;
    update();
}

void DrawVideo::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, antialiased);
    //painter.translate(width() / 2, height() / 2);


    //painter.setPen(QPen(QColor(0, 100 / 2, 127, 255), 3));
    //painter.drawEllipse(QRectF(-100 / 2.0, -100 / 2.0, 100, 100));
    unsigned char* mem=core->GetFrame();
    if(mem==NULL){
       return;
   }
    int w=core->GetWidth();
    int h=core->GetHeight();

    colorConvert[RGB32][YUV420](rgb,w*4,mem,w,h);

    int h_vp=painter.viewport().height();
    Q_UNUSED(h_vp)
    int w_vp=painter.viewport().width();
    Q_UNUSED(w_vp)
    QImage image= QImage(rgb,w,h,QImage::Format_RGB32);
    //painter.drawPixmap(0-(w_vp/2),0-(h_vp/2),w,h,QPixmap::fromImage((image)));
    painter.drawPixmap(0,0,w,h,QPixmap::fromImage((image)));


}

