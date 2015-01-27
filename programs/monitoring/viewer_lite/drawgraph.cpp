#include "drawgraph.h"
#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <igraph/igraph.h>
#include <QTimer>

#include "edge.h"
#include "imagenode.h"


DrawGraph::DrawGraph(QGraphicsView* view,QGraphicsScene* scene, QString group)
    : m_view(view),m_scene(scene),m_group(group)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(makeAni()));
}

ImageNode * DrawGraph::findRoot(QList<ImageNode*> *list){

    ImageNode * root = NULL;
    for(int i = 0; i < list->length(); i++){
        ImageNode* n = list->at(i);
        if(n->get_parent() == 0 && n->get_children().length() > 0){
            root = n;
            return root;
        }
    }
    if(root==0)return list->first();
    else return root;

}

void DrawGraph::drawGraph(QList<Edge*> *edgeList,QList<ImageNode*> *images){
    igraph_t g;
    igraph_matrix_t coords;

    if(images->length() == 1){
        m_scene->addItem(images->at(0));
        images->at(0)->setPos(0,0);
        igraph_matrix_destroy(&coords);
        igraph_destroy(&g);
        return;
    }
    else{
        igraph_empty(&g,images->length()+1,false);
    }

    igraph_matrix_init(&coords, 0, 0);
    ImageNode * root = findRoot(images);

    for(int i =0; i < images->length();i++){
        ImageNode* imag = images->at(i);
        if(imag->is_img()){
            if(imag->get_parent() == 0){
                Edge* ed = new Edge(root,imag,2);
                edgeList->append(ed);
            }
            if(imag->get_children().length() == 0){
                QImage * node = new QImage();
                node->load(":/NODE");
                imag->set_image(*node);
                imag->setScale(0.12);
            }
        }
        if(imag->get_edges().length()==0){
            images->removeAt(i);
        }
    }

    for(int i = 0; i < images->length();i++){
        ImageNode * n = images->at(i);
        for(int f = 0; f < n->get_children().length();f++){
            ImageNode * child = n->get_children().at(f);
            int idfrom =n->get_node_id();
            int idto = child->get_node_id();
            if(idfrom != idto && idfrom <=edgeList->length()*2+1 && idto <=edgeList->length()*2+1){
                try{
                    igraph_add_edge(&g,idfrom,idto);
                }
                catch (invalid_argument &e){
                    std::cout << e.what() << std::endl;
                    std::cout << "from id : "<< idfrom << "  to  " << idto << std::endl;
                }
            }
        }
    }

    if(edgeList->length()>0){
        int err;
#ifdef __APPLE__ // actually libigraph >= 0.6
        igraph_vector_t igv;
        int tmp = root->get_node_id();
        int ret = igraph_vector_init_copy (&igv, (igraph_real_t*)&tmp, 1);
        if (ret < 0) {
            cerr << "failed to init igraph vector" << endl;
            return;
        }
        err = igraph_layout_reingold_tilford(&g, &coords, IGRAPH_ALL, &igv, 0);
        igraph_vector_destroy(&igv);
#else // that is most likely __linux__ and libigraph < 0.6
        err =igraph_layout_reingold_tilford(&g, &coords, root->get_node_id());
#endif
        if(err == -1){
            qDeleteAll(*images);
            qDeleteAll(*edgeList);
        }
    }

    for(int i =0; i < edgeList->length();i++){
        m_scene->addItem(edgeList->at(i));
        edgeList->at(i)->adjust();
        m_scene->addItem(&edgeList->at(i)->get_animation());
    }

    for(int t = 0; t< images->length();t++){
        ImageNode *img = images->at(t);
        int posx = (MATRIX(coords, t, 0)*100);
        int posy = (MATRIX(coords, t, 1)*100);
        QRectF rec =  img->mapRectToScene(img->boundingRect());
        posx -= rec.width()/2;
        posy -= rec.height()/2;
        img->setPos(posx,posy);
        img->update_text_pos();
        m_scene->addItem(img->get_text_item());
        m_scene->addItem(img);
    }
    m_aniroot = root;
    igraph_matrix_destroy(&coords);
    igraph_destroy(&g);
}



void DrawGraph::showGraph(){
    m_view->setScene(m_scene);
    if(!m_timer.isActive()){
        m_timer.start(6000);
        m_aniroot->start_animations(true);
    }
    m_view->show();
}
void DrawGraph:: makeAni(){
    m_aniroot->start_animations(true);
}
