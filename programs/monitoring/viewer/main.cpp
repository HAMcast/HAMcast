#include <QtGui/QApplication>
#include "mainwindow.h"
#include "QDebug"
#include <mysplash.h>
#include "hamcast_group.hpp"
#include <hamcast/hamcast.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(":/LOGO");
    MySplash *splash = new MySplash(pixmap);
    splash->showMessage("HAMcast Monitoring Tool");
    splash->show();
    MainWindow w(splash);

    return a.exec();


}












