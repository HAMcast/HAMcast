#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QSplashScreen>




int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("plastique");
    QPixmap pixmap(":/LOGO");
    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->showMessage("HAMcast Monitoring Tool");
    splash->setEnabled(false);
    splash->show();
    QString ip = "141.22.28.249";
    int port = 35000;
//    update_thread u(ip,port);
//    u.start();

    MainWindow w(splash);
    return a.exec();


}












