#include "ledgend.h"
#include "ui_ledgend.h"

Ledgend::Ledgend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Ledgend)
{
    ui->setupUi(this);
}

Ledgend::~Ledgend()
{
    delete ui;
}
