#include "mysplash.h"


   MySplash::MySplash(QPixmap pix,QWidget *parent)
   :QSplashScreen(parent)

   {
    this->installEventFilter(this);
    this->setPixmap(pix);
   }

   bool MySplash::eventFilter(QObject *target, QEvent *event){
   return true;
   }
