#ifndef MYSPLASH_H
#define MYSPLASH_H

#include <QSplashScreen>
#include <QWidget>

class MySplash : public QSplashScreen
{
    Q_OBJECT
public:
    explicit MySplash(QPixmap pix,QWidget *parent = 0);

signals:

public slots:
 bool eventFilter(QObject *target, QEvent *event);
};

#endif // MYSPLASH_H
