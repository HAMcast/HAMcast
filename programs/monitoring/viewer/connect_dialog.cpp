#include "connect_dialog.hpp"
#include "ui_connect_dialog.h"

connect_dialog::connect_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::connect_dialog)
{
    ui->setupUi(this);
    ui->pushButton->setFocus();

}

connect_dialog::~connect_dialog()
{
    delete ui;
}

QString connect_dialog::get_ip(){
    return ui->lineEdit->text();
}

QString connect_dialog:: get_port(){
    return ui->lineEdit_2->text();
}

QString connect_dialog::get_updaterate(){
    return ui->lineEdit_3->text();
}

void connect_dialog::on_pushButton_2_clicked()
{
    emit(exit());
}

void connect_dialog::on_pushButton_clicked()
{
    emit(connect());
}
