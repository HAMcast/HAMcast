#ifndef DRAWGRAPH_H
#define DRAWGRAPH_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <igraph/igraph.h>
#include <QTimer>

#include "edge.h"
#include "imagenode.h"


using namespace std;

class DrawGraph : public QObject
{
Q_OBJECT
private:
     QString m_group;
     QTimer m_timer;
     ImageNode *m_aniroot;
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
     DrawGraph(QGraphicsView* view =0,QGraphicsScene* scene =0, QString group=0);

     /**
       * to display the graph
       */
     void showGraph();

     /**
       *@brief to calculate node positions
       *@param edgeList = list of edges between nodes
       *@param imageList = list of imageNodes between the edges
       */
     void drawGraph(QList<Edge*> *edgeList,QList<ImageNode*> *imageList);

     inline void clear_graph(){
        m_timer.stop();
        m_scene->clear();
     }

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

     inline QString get_group(){
         return m_group;
     }

     ImageNode * findRoot(QList<ImageNode*> *list);
public slots:
    void makeAni();
};

#endif // DRAWGRAPH_H
