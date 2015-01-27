#include "table_widget.hpp"
#include "ui_table_widget.h"

table_widget::table_widget(QIcon &icon, QWidget *parent) :
    QWidget(parent), m_icon(icon),
    ui(new Ui::table_widget)
{
    ui->setupUi(this);
}

table_widget::~table_widget()
{
    delete ui;
}

void table_widget::addList(const QList<QString>& list)
{
    m_list = list;

    ui->tableWidget->setRowCount(m_list.length());
    ui->tableWidget->setColumnCount(m_list.length());
    int x =0;
    int y =0;
    foreach(QString name, m_list){
        QTableWidgetItem * item = new QTableWidgetItem();
        item->setText(name);
        item->setIcon(m_icon);
        ui->tableWidget->setItem(x,y,item);
        x++;
        if(x == 6){
            x =0;
            y++;
        }
    }
    ui->tableWidget->resizeColumnsToContents();
}

void table_widget::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    emit(item_list_double_clicked(item->text()));
}
