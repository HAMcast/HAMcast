#include "coreexception.h"
#include <iostream>

using namespace std;

CoreException::CoreException(string msg)
{
    this->msg=msg;
}

void CoreException::PrintError(){
    cout<< "Error: "<<msg<<endl;
}
