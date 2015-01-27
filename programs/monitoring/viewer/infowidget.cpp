#include "infowidget.h"
#include "ui_infowidget.h"

InfoWidget::InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWidget)
{
    ui->setupUi(this);
//    ui->treeWidget->setVisible(false);
}

InfoWidget::~InfoWidget()
{
    delete ui;
}

QListWidget *InfoWidget::getGrouList(){
    return ui->GroupList;
}



QTreeWidget *InfoWidget::getTree(){
    return ui->treeWidget;
}

