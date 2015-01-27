#ifndef NODE_DETAILS_HPP
#define NODE_DETAILS_HPP

#include <QWidget>
#include <QTreeWidget>
#include "hamcast_node.hpp"
#include "hamcast_interface.hpp"
#include "hamcast_group.hpp"

namespace Ui {
class node_details;
}



class node_details : public QWidget
{
    Q_OBJECT
private:
    Ui::node_details*        ui;
    QIcon&                   m_network_icon;
    QIcon&                   m_group_icon;
    QIcon&                   m_node_icon;
    hamcast_node             m_hamcast_node;
    QString                  m_interface;

    void fill_interface_list();
    void fill_interface_detail(const QString& interface_name);
    void fill_group_list();

public:
    explicit node_details(QIcon& m_network_icon, QIcon& group_icon, QIcon& node_icon,QWidget *parent = 0);
    ~node_details();
    
    void clear_and_close();

    void fill_trees(const hamcast_node &node);

private slots:

    void on_node_detail_tree_itemClicked(QTreeWidgetItem *item, int column);
    void on_interface_list_itemClicked(QTreeWidgetItem *item, int column);
    void on_interface_detail_itemClicked(QTreeWidgetItem *item, int column);
    void on_group_list_itemClicked(QTreeWidgetItem *item, int column);
};

#endif // NODE_DETAILS_HPP
