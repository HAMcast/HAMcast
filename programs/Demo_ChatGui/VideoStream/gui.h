#ifndef GUI_H
#define GUI_H

#include <QWidget>
#include <QMessageBox>

using namespace std;

class GUI
{
public:
    GUI(QWidget* window);
    void ShowError(string error);


private:
    QWidget* window;
};

#endif // GUI_H
