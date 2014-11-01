#include "mainwindow.h"
#include <QApplication>
#include <QGst/Init>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGst::init(&argc, &argv);

    MainWindow w(a.arguments()[1]);
    w.show();

    return a.exec();
}
