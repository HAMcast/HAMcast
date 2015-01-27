#ifndef LEDGEND_H
#define LEDGEND_H

#include <QWidget>

namespace Ui {
    class Ledgend;
}

class Ledgend : public QWidget
{
    Q_OBJECT

public:
    explicit Ledgend(QWidget *parent);
    ~Ledgend();

private:
    Ui::Ledgend *ui;
};

#endif // LEDGEND_H
