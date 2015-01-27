#ifndef QUESTIONBOX_H
#define QUESTIONBOX_H

#include <QDialog>
#include <QTextBrowser>
#include <QTextEdit>
#include <chat.h>
#include <QListWidget>
namespace Ui {
    class QuestionBox;
}

class QuestionBox : public QDialog
{
    Q_OBJECT

public:
    explicit QuestionBox(Chat* chat,QListWidget* qlist,QWidget *parent = 0);
    ~QuestionBox();
    QTextBrowser* getQ();
private:
    Ui::QuestionBox *ui;
    Chat * chat;
    QListWidget* qlist;
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};

#endif // QUESTIONBOX_H
