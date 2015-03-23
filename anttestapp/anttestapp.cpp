#include <QtCore/QCoreApplication>

#include "antcontroller.h"
int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    ANTController controller;
    a.exec();
}
