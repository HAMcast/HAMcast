#ifndef TABLE_WIDGET_HPP
#define TABLE_WIDGET_HPP

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
class table_widget;
}

class table_widget : public QWidget
{
    Q_OBJECT
    QList<QString> m_list;
    QIcon&                   m_icon;
public:
    explicit table_widget( QIcon& icon,QWidget *parent = 0);
    ~table_widget();
    void addList(const QList<QString> &list);
private slots:
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

signals:
    void item_list_double_clicked(QString name);

private:
    Ui::table_widget *ui;
};

#endif // TABLE_WIDGET_HPP
