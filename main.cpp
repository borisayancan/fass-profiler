#include "fmain.h"
#include <QApplication>


// Git init, may 3th 2020
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FMain w;
    w.show();

    return a.exec();
}
