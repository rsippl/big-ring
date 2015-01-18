#include "mainwindow.h"
#include <QApplication>
#include <gst/gst.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationDomain("org.github.ibooij");
    a.setOrganizationName("Ilja Booij");
    a.setApplicationName("Big Ring Video Indoor Cycling");
    gst_init(&argc, &argv);
    QString dir;
    if (a.arguments().length() > 1) {
        dir = a.arguments()[1];
    } else {
        dir = "/media/ibooij/SamsungTacx/Tacx/";
    }
    MainWindow w(dir);
    w.showMaximized();

    return a.exec();

    gst_deinit();
}
