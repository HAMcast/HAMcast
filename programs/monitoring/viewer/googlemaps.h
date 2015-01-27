#ifndef GOOGLEMAPS_H
#define GOOGLEMAPS_H

#include <QWidget>
#include <QWebView>
#include <QWebFrame>

class GoogleMaps : public QWidget
{
    Q_OBJECT
public:
    explicit GoogleMaps(QWidget *parent = 0);
    QWebView* getWebView();

    /**
      * @brief to add a marker on the map
      * @param a,b = geo coordinate
      */
    void addMaker(double a, double b);

    /**
      *@brief to resize the window
      */
    void setSize(double w,double h);

    /**
      * @brief to initiate the object
      */
    void initWeb();

private:
    QWebView *web;
    QWebFrame *frame;
    QList<QPair<double,double> > markerList;

public slots:
   void resizeEvent(QResizeEvent *);
};

#endif // GOOGLEMAPS_H
