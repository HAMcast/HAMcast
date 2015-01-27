#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QTreeWidget>

namespace Ui {
    class InfoWidget;
}

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWidget(QWidget *parent = 0);
    ~InfoWidget();
    QListWidget *getGrouList();
    QTreeWidget *getTree();
private:
     Ui::InfoWidget *ui;
};

#endif // INFOWIDGET_H
