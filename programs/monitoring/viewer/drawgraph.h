#ifndef DRAWGRAPH_H
#define DRAWGRAPH_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "edge.h"
#include <QTableWidget>
#include <cmath>
#include <QTime>
#include <QDebug>
#include <igraph/igraph.h>
#include <imagenode.h>
#include <QTimer>


using namespace std;

class DrawGraph : public QObject
{
Q_OBJECT
private:
     QTimer *m_timer;
     QString m_group;
     int m_tabindex;
     ImageNode *m_aniroot;
     QTabWidget *m_tab;
     QGraphicsView *m_view;
     QGraphicsScene *m_scene;
public:

     /**
       *@brief creates a DrawGraph object
       *@param view = the view to display the graph
       *@param scene = the scene to display the graph
       *@param tab = the tab widget where the graph will be painted
       *@param group = name for the tabWidget
       */
     DrawGraph(QGraphicsView* view =0,QGraphicsScene* scene =0,QTabWidget * tab =0, QString group=0);

     /**
       * to display the graph
       */
     void showGraph(bool time);

     /**
       *@brief to calculate node positions
       *@param edgeList = list of edges between nodes
       *@param imageList = list of imageNodes between the edges
       */
     void drawGraph(QList<Edge*> *edgeList,QList<ImageNode*> *imageList);

     inline QGraphicsView* get_view(){
         return m_view;
     }

     inline QGraphicsScene* get_scene(){
         return m_scene;
     }

     inline void set_scene(QGraphicsScene* scene){
        m_scene = scene;
     }

     inline void set_view(QGraphicsView* view){
        m_view = view;
     }

     inline int get_tab_index(){
         return m_tabindex;
     }

     inline QString get_group(){
         return m_group;
     }

     ImageNode * findRoot(QList<ImageNode*> *list);
public slots:
    void makeAni();
};

#endif // DRAWGRAPH_H
