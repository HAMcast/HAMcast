#include "googlemaps.h"
#include <QVBoxLayout>
#include <QFile>

GoogleMaps::GoogleMaps(QWidget *parent) :
    QWidget(parent)
{
    web = new QWebView(this);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(this->web);
    this->setLayout(layout);
    initWeb();
}

void GoogleMaps::initWeb(){
    frame = web->page()->mainFrame();
    QFile file("data/gmap.html");
    file.open(QIODevice::ReadOnly | QIODevice::Text); // Als Text-Datei nur zum Lesen öffnen
    QString content = file.readAll(); // Datei komplett auslesen und in das TextEdit setzen
    file.close(); // Datei wieder schließen
    QString setter = "width:"+QString().setNum(web->width())+"px; height:"+QString().setNum(web->height())+"px;";
    content.replace("width:100%; height:100%;", setter);
    frame->setHtml(content);
}

QWebView* GoogleMaps::getWebView(){
    return web;
}

void GoogleMaps::addMaker(double a, double b){
    markerList.append(QPair<double, double>(a,b));
    QString code = "addMarker("+QString().setNum(a)+","+QString().setNum(b)+ ");";
    web->page()->mainFrame()->evaluateJavaScript(code);
}

void GoogleMaps::resizeEvent(QResizeEvent *){
    initWeb();
    for(int i =0 ; i < markerList.length();i++){
        QString code = "addMarker("+QString().setNum(markerList.at(i).first)+","+QString().setNum(markerList.at(i).second)+ ");";
        web->page()->mainFrame()->evaluateJavaScript(code);
    }
}
