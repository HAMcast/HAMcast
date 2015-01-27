#include "gui.h"
#include <iostream>

GUI::GUI(QWidget* window)
{
    this->window=window;
}


void GUI::ShowError(string error){
    //QMessageBox::about(window, "Error",error.c_str());
    std::cout<<"ERROR: "<<error<<std::endl;

}
