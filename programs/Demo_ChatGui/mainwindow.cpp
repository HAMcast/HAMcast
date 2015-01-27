#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <hamcast/hamcast.hpp>
#include "chat.h"
#include "questionbox.h"

using namespace hamcast;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    vport = 7580;
    ui->setupUi(this);
    wizard.addPage(new Intro);
    configPage = new Config;
    wizard.addPage(configPage);
    videoPage = new VideoCooser;
    wizard.addPage(videoPage);
    wizard.addPage(new Finished);
    wizard.setWindowTitle("Hamcast Demo Chat");
    connect(&wizard,SIGNAL(finished(int)),this,SLOT(wizardFin(int)));
    connect(ui->actionExit,SIGNAL(triggered()),this,SLOT(close()));
    wizard.resize(QSize(200,300));
    wizard.show();
ui->pushButton_8->setVisible(false);
    delAct = new QAction(tr("delete"), this);
//    delAct->setShortcuts(QKeySequence::Cut);
    delAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
//    connect(delAct, SIGNAL(triggered()), this, SLOT(cut()));
    ui->listWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::wizardFin(int arg){
    if(arg == QWizard::Rejected){
        this->close();
    }
    else{

        this->show();
        ui->nicknameLabel->setText(configPage->nameLineEdit->text());
        ui->groupLabel->setText(configPage->groupLineEdit->text());
        if(configPage->send->isChecked()){
           chat = new Chat(configPage->getName().toStdString(),configPage->getGroup()->c_str(),true,ui->listWidget,ui->textBrowser,ui->listWidget_2,ui->usercount,ui->questioncount);
           if(videoPage->cam->isChecked()){
                //mach irgendwas
           }
           else{
               if(!videoPage->filename.isEmpty()){
                    p = new Player(ui->horizontalSlider,ui->widget);
                    ui->widget->layout()->addWidget(p);
                    udp = new UdpReciever(QString::fromStdString(configPage->getGroup()->c_str())+ ":" + QString().setNum(vport),true);
               }
           }
        }
        else{

            ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tab_2));
//            ui->tab_2->setDisabled(true);
            chat = new Chat(configPage->getName().toStdString(),configPage->getGroup()->c_str(),false,ui->listWidget,ui->textBrowser,ui->listWidget_2,ui->usercount,ui->questioncount);
            p = new Player(ui->horizontalSlider,ui->widget);
            ui->widget->layout()->addWidget(p);
            udp = new UdpReciever(QString::fromStdString(configPage->getGroup()->c_str())+ ":" + QString().setNum(vport),false);
        }
//        init(configPage->getGroup()->c_str(),configPage->getName().toStdString());
    questionBox = new QuestionBox(chat,ui->listWidget_2);
    }
}

Intro::Intro(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Welcome");
    QLabel *label2 = new QLabel;

    label = new QLabel("to the Hamcast video chat Demo.\n"
                               "To continue click next.");
    label->setWordWrap(true);
    label2->setPixmap(QPixmap("logo.png"));

    layout = new QVBoxLayout;
     layout->addWidget(label2);
    layout->addWidget(label);



    setLayout(layout);
}




int Intro::nextId() const
 {
    return MainWindow::Config_Page;
 }





Config::Config(QWidget *parent)
    :  QWizardPage(parent)
{

    setTitle("Configuration");
    setSubTitle("Please fill out all fields.");

    nameLabel = new QLabel("Nickname:");
   nameLineEdit = new QLineEdit;

    groupLabel = new QLabel("Group:");
    groupLineEdit = new QLineEdit;
    send = new QRadioButton("Send");
    recieve = new QRadioButton("Receive");
    send->setChecked(true);
    vbox = new QVBoxLayout;
    groupBox = new QGroupBox("Send or Receive video stream");
    vbox->addWidget(send);
    vbox->addWidget(recieve);
    groupBox->setLayout(vbox);
    vbox2 = new QGridLayout();
    vbox2->addWidget(nameLabel,0,0);
    vbox2->addWidget(nameLineEdit,0,1);
    vbox2->addWidget(groupLabel,1,0);
    vbox2->addWidget(groupLineEdit,1,1);
    layout = new QGridLayout;


    layout->addWidget(groupBox,1,0);
    layout->addLayout(vbox2,0,0);
    error = new QLabel;
    layout->addWidget(error);
    setLayout(layout);
    registerField("Nickname*", nameLineEdit);
//    registerField("Group*", groupLineEdit);
    groupLineEdit->setText("ip://239.201.108.1");

}

int Config::nextId() const
 {
    if(send->isChecked()){
        return MainWindow::Video_Page;
    }
    else{
        return MainWindow::Finished_Page;
    }
 }


bool Config:: validatePage(){
    QString group = groupLineEdit->text();
    u = new uri(group.toStdString());
    if(u->empty()){
        error->setText("<font color='red'>Invalid Group uri</font>");
        return false;
    }
    error->clear();
    nickname = nameLineEdit->text();
    return true;
}
QString Config::getName() {return nickname;}
uri* Config::getGroup(){return u;}

VideoCooser::VideoCooser(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Video Device");
    setSubTitle("Choose your Video Divice");
    cam = new QRadioButton;
    cam->setChecked(true);
    cam->setText("Webcam");
    video = new QRadioButton;
    video->setText("Filestreaming");
    layout = new QVBoxLayout;
    layout->addWidget(cam);
    layout->addWidget(video);
    setLayout(layout);
    fileDialog = new QFileDialog;

}

bool VideoCooser:: validatePage(){
    if(cam->isChecked()){
        return true;
    }
    if(video->isChecked()){
//        fileDialog->show();
        filename = QFileDialog::getOpenFileName(this,tr("Open Video"), "/home/", tr("Video Files (*.mp4)"));
        if(!filename.isEmpty()){
            return true;
        }
        else{
            return false;
        }
    }

}

int VideoCooser::nextId() const
{

    return MainWindow::Finished_Page;
}

Finished::Finished(QWidget *parent)
    :QWizardPage(parent)
{
    setTitle("Configuration Completed");
   lable = new QLabel;
   lable->setText("Configuration sucsessfully completed.\n"
                 "Click finish to run the programm ");
    layout = new QVBoxLayout;
    layout->addWidget(lable);
    setLayout(layout);

}

int Finished::nextId() const{
    return -1;
}


void MainWindow::on_pushButton_clicked()
{
    chat->sendMsg(ui->lineEdit->text().toStdString());
//    ui->textBrowser->append(ui->lineEdit->text());
    if(configPage->send->isChecked()){

        ui->textBrowser->append("<strong>"+configPage->nameLineEdit->text()+" ( "+QTime::currentTime().toString()+" )"+"</strong>"+" : "+ui->lineEdit->text());
    }
    ui->lineEdit->clear();
}

void MainWindow::on_lineEdit_returnPressed()
{
    chat->sendMsg(ui->lineEdit->text().toStdString());
//    ui->textBrowser->append(ui->lineEdit->text());
    if(configPage->send->isChecked()){

        ui->textBrowser->append("<strong>"+configPage->nameLineEdit->text()+" ( "+QTime::currentTime().toString()+" )"+"</strong>"+" : "+ui->lineEdit->text());
    }
    ui->lineEdit->clear();
}

void MainWindow::on_listWidget_2_customContextMenuRequested(QPoint pos)
{
    // right click menue dosent work yet
//        QMenu menu(this);
//        menu.addAction(delAct);

//        menu.exec(mapToParent(pos));


}

void MainWindow::on_pushButton_2_clicked()
{
    QListWidgetItem *currentItem =ui->listWidget_2->currentItem();
    ui->listWidget_2->removeItemWidget(ui->listWidget->currentItem());
       ui->listWidget_2->clearSelection();
       delete currentItem;
    ui->questioncount->setText(QString().setNum(ui->listWidget_2->count()));

}

void MainWindow::on_pushButton_3_clicked()
{
    questionBox->close();
    QListWidgetItem *currentItem =ui->listWidget_2->currentItem();
    if(currentItem!=0){
        questionBox->getQ()->setText(currentItem->text());
        questionBox->show();
    }

}

void MainWindow::on_pushButton_5_clicked()
{
    if(videoPage->video->isChecked()){
        udp->start();
        p->broadcastFile(videoPage->filename);
        p->playFile(videoPage->filename);
    }
    else{
        udp->start();
        p->playFile("rtp://@127.0.0.1:5003");
    }

}

void MainWindow::on_pushButton_4_clicked()
{
    udp->deleteLater();
    this->close();

}

void MainWindow::on_pushButton_6_clicked()
{
    p->stopPlaying();
}
