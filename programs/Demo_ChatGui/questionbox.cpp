#include "questionbox.h"
#include "ui_questionbox.h"

QuestionBox::QuestionBox(Chat * chat,QListWidget* qlist,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuestionBox)
{
    ui->setupUi(this);
    this->chat = chat;
    this->qlist = qlist;
}

QuestionBox::~QuestionBox()
{
    delete ui;
}
QTextBrowser* QuestionBox:: getQ(){
    return ui->textBrowser;
}

void QuestionBox::on_pushButton_2_clicked()
{
    close();
}

void QuestionBox::on_pushButton_clicked()
{
    ui->label_3->clear();
    if(ui->textEdit->toPlainText().size() > 0){
//        chat->sendMsg("<br> <font color='red'>Q : </font></br>"+ui->textBrowser->toPlainText().toStdString()+"\n"+"<br><font color='red'>A : </font></br>"+ui->textEdit->toPlainText().toStdString());
          chat->sendMsg(" Q : "+ui->textBrowser->toPlainText().toStdString()+"\n"+" A : "+ui->textEdit->toPlainText().toStdString());
        close();
        QListWidgetItem* current = qlist->currentItem();
        qlist->removeItemWidget(current);
        delete current;

    }
    else{
        ui->label_3->setText("<font color='red'>U have to fill out A Field</font>");
    }

}
