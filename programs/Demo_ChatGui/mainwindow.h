#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <hamcast.hpp>
#include "chat.h"
#include <QAction>
#include <questionbox.h>
#include "vlcStreamer/vlc_on_qt.h"
#include "vlcStreamer/udpreciever.h"
using namespace hamcast;
namespace Ui {
    class MainWindow;
}

class Config : public QWizardPage
 {
     Q_OBJECT

 public:
     Config(QWidget *parent = 0);

     int nextId() const;
     bool validatePage();
     QString getName();
     uri* getGroup();
     QRadioButton *send ;
     QLineEdit *nameLineEdit ;
     QLineEdit *groupLineEdit ;
 private:
     QLabel *nameLabel;
     QLabel *groupLabel ;
     QRadioButton *recieve ;
     QVBoxLayout *vbox ;
     QGridLayout *vbox2 ;
     QGroupBox *groupBox ;
     QGridLayout *layout ;
     QLabel * error;
     uri *u;
     QString nickname;

 };




class Intro : public QWizardPage
 {
     Q_OBJECT

 public:
     Intro(QWidget *parent = 0);

     int nextId() const;

 private:
     QLabel *label;
     QVBoxLayout *layout;

 };


class VideoCooser : public QWizardPage
 {
     Q_OBJECT

 public:
     VideoCooser(QWidget *parent = 0);

     int nextId() const;
     bool validatePage();
     QString filename;
     QRadioButton *cam ;
     QRadioButton *video;
 private:
     QLabel *label;

     QFileDialog * fileDialog;
     QVBoxLayout *layout;

 };

class Finished : public QWizardPage
 {
     Q_OBJECT

 public:
     Finished(QWidget *parent = 0);

     int nextId() const;

 private:
    QLabel *lable;
    QVBoxLayout *layout;
 };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    enum { Intro_Page, Config_Page,Video_Page,Finished_Page};

public slots:
    void wizardFin(int);
private:
    Ui::MainWindow *ui;
    Config *configPage;
    QWizard wizard;
    Chat * chat;
    VideoCooser* videoPage;
    QAction *delAct;
    QuestionBox * questionBox;
    Player * p;
    UdpReciever * udp;
    int vport;
private slots:
    void on_pushButton_6_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void on_listWidget_2_customContextMenuRequested(QPoint pos);
    void on_lineEdit_returnPressed();
    void on_pushButton_clicked();
};



#endif // MAINWINDOW_H
