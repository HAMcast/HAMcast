#ifndef COREEXCEPTION_H
#define COREEXCEPTION_H

#include <string>
using namespace std;


class CoreException
{
public:
    CoreException(string msg);

    void PrintError();
private:
    string msg;
};

#endif // COREEXCEPTION_H
