#ifndef CONNECT_DIALOG_HPP
#define CONNECT_DIALOG_HPP

#include <QDialog>

namespace Ui {
class connect_dialog;
}

class connect_dialog : public QDialog
{
    Q_OBJECT

    
public:
    explicit connect_dialog(QWidget *parent = 0);
    ~connect_dialog();
public slots:

     QString get_ip();
     QString get_port();
     int get_updaterate();

signals:
   void exit();
   void connect();

private slots:
   void on_pushButton_2_clicked();

   void on_pushButton_clicked();

private:
    Ui::connect_dialog *ui;
};

#endif // CONNECT_DIALOG_HPP
