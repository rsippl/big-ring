#include <QtCore/QCoreApplication>
#include <QtCore/QtDebug>
#include "antcontroller.h"
int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    qDebug() << "Starting";

    ANTController controller;
    a.exec();
}
