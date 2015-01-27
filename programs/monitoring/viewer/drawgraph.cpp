#include "drawgraph.h"
#include <igraph/igraph.h>
#include "imagenode.h"
#include <QAbstractGraphicsShapeItem>
#include <QImage>
#include <QVBoxLayout>
#include "ledgend.h"

DrawGraph::DrawGraph(QGraphicsView* view,QGraphicsScene* scene, QTabWidget * tab,QString group)
{
    this->m_view = view;
    this->m_scene = scene;
    this->m_tab = tab;
    this->m_group= group;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(makeAni()));
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
        return;
        igraph_empty(&g,images->length()+1,false);
    }
    else{
        igraph_empty(&g,images->length()*2+1,false);
    }
    igraph_matrix_init(&coords, 0, 0);
    ImageNode * root = findRoot(images);
    for(int i =0; i < images->length(); i++){
        ImageNode* imag = images->at(i);
        if(imag->is_img()){
            if(imag->get_parent() == 0){
                Edge* ed = new Edge(root,imag,"e1");
                edgeList->append(ed);
            }
            if(imag->get_children().length() == 0){
                QImage * node = new QImage();
                node->load(":/NODE_OM");
                imag->set_image(*node);
            }
        }


    }
    if(images->length()!=1){
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
            if(n->get_edges().length()==0){
                images->removeAt(i);
            }

        }
    }
    for(int i =0; i< images->length();i++){
        if(images->at(i)->get_parent() == 0 && images->at(i)->get_children().length()==0 ){
            images->removeAt(i);
        }
    }
    if(edgeList->length()>0){
#ifdef __APPLE__ // actually libigraph >= 0.6
        igraph_vector_t igv;
        int tmp = root->get_node_id();
        int ret = igraph_vector_init_copy (&igv, (igraph_real_t*)&tmp, 1);
        if (ret < 0) {
            cerr << "failed to init igraph vector" << endl;
            return;
        }
        igraph_layout_reingold_tilford(&g, &coords, IGRAPH_ALL, &igv, 0);
        igraph_vector_destroy(&igv);
#else // that is most likely __linux__ and libigraph < 0.6
        igraph_layout_reingold_tilford(&g, &coords, root->get_node_id());
#endif
    }

    for(int i =0; i < edgeList->length();i++){
        m_scene->addItem(edgeList->at(i));
        edgeList->at(i)->adjust();
        m_scene->addItem(&edgeList->at(i)->get_animation());
    }

    for(int t = 0; t< images->length();t++){
        ImageNode *img = images->at(t);
        img->setPos(MATRIX(coords, t, 0)*100,MATRIX(coords, t, 1)*100);
        img->update_text_pos();
        m_scene->addItem(img->get_text_item());
        m_scene->addItem(img);
    }
    m_aniroot = root;
    igraph_matrix_destroy(&coords);
    igraph_destroy(&g);
}



void DrawGraph::showGraph(bool time){
    m_view->setScene(m_scene);
    if( m_group.compare(m_tab->tabText(m_tabindex)) != 0){
        Ledgend * l = new Ledgend((QWidget*)this->parent());
        QVBoxLayout *layout = new QVBoxLayout;
        QWidget *wid = new QWidget;
        wid->setLayout(layout);
        layout->addWidget(m_view);
        layout->addWidget(l);
        m_tabindex = m_tab->addTab(wid,m_group);
    }
    if(!m_timer->isActive()){
        m_timer->start(6000);
        m_aniroot->start_animations(true);
    }
    m_tab->setCurrentIndex(m_tabindex);
    m_view->show();
}
void DrawGraph:: makeAni(){   
    m_aniroot->start_animations(true);
}
