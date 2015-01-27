#include "node_details.hpp"
#include "ui_node_details.h"

node_details::node_details(QIcon &network_icon, QIcon &group_icon, QIcon &node_icon, QWidget *parent) :
    QWidget(parent),m_network_icon(network_icon) ,m_group_icon(group_icon),m_node_icon(node_icon),
    ui(new Ui::node_details)
{
    ui->setupUi(this);
    clear_and_close();
}

node_details::~node_details()
{
    delete ui;
}

void node_details::clear_and_close()
{
    ui->node_detail_tree->clear();
    ui->group_list->clear();
    ui->interface_list->clear();
    ui->interface_detail->clear();

    ui->group_list->hide();
    ui->interface_list->hide();
    ui->interface_detail->hide();
}

void node_details::fill_trees(const hamcast_node &node)
{
    m_hamcast_node = node;
    clear_and_close();
    QTreeWidgetItem* name = new QTreeWidgetItem(ui->node_detail_tree);
    name->setIcon(0,m_node_icon);
    name->setText(0,"Daemon ID");
    name->setText(0,m_hamcast_node.get_name());
    QTreeWidgetItem* groups = new QTreeWidgetItem(ui->node_detail_tree);
    groups->setIcon(0,m_group_icon);
    groups->setText(0,"Groups");
    QTreeWidgetItem* interfaces = new QTreeWidgetItem(ui->node_detail_tree);
    interfaces->setIcon(0,m_network_icon);
    interfaces->setText(0,"Interfaces");
    ui->node_detail_tree->resizeColumnToContents(0);
}

void node_details::on_node_detail_tree_itemClicked(QTreeWidgetItem *item, int column){
    if(item->text(0)== "Interfaces"){
        fill_interface_list();
        ui->interface_list->show();
        m_interface.clear();
        ui->group_list->clear();
        ui->group_list->hide();
    }
    if(item->text(0) == "Groups"){
//        m_interface.clear();
//        ui->group_list->clear();
//        fill_group_list();
//        ui->group_list->show();
    }
}

void node_details::on_interface_list_itemClicked(QTreeWidgetItem *item, int column)
{
    QString interface_name = item->text(0);
    fill_interface_detail(interface_name);
}

void node_details::on_interface_detail_itemClicked(QTreeWidgetItem *item, int column)
{
    if(item->text(0) == "Groups"){
        ui->group_list->clear();
        fill_group_list();
        ui->group_list->show();
    }
}

void node_details::on_group_list_itemClicked(QTreeWidgetItem *item, int column)
{

}

void node_details::fill_interface_list()
{
    ui->interface_list->clear();
    QList<hamcast_interface> interfaces = m_hamcast_node.get_interfaces();

    foreach(hamcast_interface inter , interfaces) {
        QTreeWidgetItem* name = new QTreeWidgetItem(ui->interface_list);
        name->setText(0,inter.get_name());
        name->setIcon(0,m_network_icon);
    }
}

void node_details::fill_interface_detail(const QString &interface_name)
{
    ui->interface_detail->resizeColumnToContents(1);
    QList<hamcast_interface> interfaces = m_hamcast_node.get_interfaces();
    foreach(hamcast_interface inter , interfaces) {
        if(inter.get_name().compare(interface_name)==0){
            m_interface = inter.get_name();
            ui->interface_detail->clear();
            QTreeWidgetItem* tech = new QTreeWidgetItem(ui->interface_detail);
            tech->setText(1,inter.get_tech());
            tech->setText(0,"Tech");
            QTreeWidgetItem* addr = new QTreeWidgetItem(ui->interface_detail);
            addr->setText(1,QString::fromStdString(inter.get_addr().str()));
            addr->setText(0,"Address");
            QTreeWidgetItem* id = new QTreeWidgetItem(ui->interface_detail);
            id->setText(1,QString(QString().setNum(inter.get_id())));
            id->setText(0,"ID");
            QList<hamcast::uri> neighbors = inter.get_neighbors();
            foreach(hamcast::uri neigh, neighbors){
                QTreeWidgetItem* nei = new QTreeWidgetItem(ui->interface_detail);
                nei->setText(1,QString::fromStdString(neigh.str()));
                nei->setText(0,"ID");
                nei->setIcon(0,m_node_icon);
            }
            QTreeWidgetItem* groups = new QTreeWidgetItem(ui->interface_detail);
            groups->setText(0,"Groups");
            groups->setIcon(0,m_group_icon);
            ui->interface_detail->show();
            return;
        }
    }

}

void node_details::fill_group_list()
{    
    QList<hamcast_interface> interfaces = m_hamcast_node.get_interfaces();
    foreach(hamcast_interface inter , interfaces) {
        if(inter.get_name().compare(m_interface)==0){
            foreach(hamcast_group group, inter.get_groups()){
                QTreeWidgetItem* gpr = new QTreeWidgetItem(ui->group_list);
                gpr->setText(0,QString::fromStdString(group.get_name().str()));
                gpr->setIcon(0,m_group_icon);
                QTreeWidgetItem* top_parent = new QTreeWidgetItem(gpr);
                top_parent->setText(0,"Parent");
                top_parent->setIcon(0,m_node_icon);
                QTreeWidgetItem* top_children = new QTreeWidgetItem(gpr);
                top_children->setText(0,"Children");
                top_children->setIcon(0,m_node_icon);
                QTreeWidgetItem* parent = new QTreeWidgetItem(top_parent);
                parent->setText(0,QString::fromStdString(group.get_parent().str()));
                QList<hamcast::uri> children  = group.get_children();
                for(int c=0;c < children.length();c++){
                    QTreeWidgetItem* child = new QTreeWidgetItem(top_children);
                    child->setText(0,QString::fromStdString(children.at(c).str()));
                }
            }
            return;
        }
    }
}
