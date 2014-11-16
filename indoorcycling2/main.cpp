#include "mainwindow.h"
#include <QApplication>
#include <QGst/Init>

int main(int argc, char *argv[])
{
    qputenv("GST_PLUGIN_PATH", "/home/ibooij/homedev/build/qtgstreamer/elements/gstqtvideosink/");
    QApplication a(argc, argv);
    QGst::init(&argc, &argv);
    QString dir;
    if (a.arguments().length() > 1) {
        dir = a.arguments()[1];
    } else {
        dir = "/media/ibooij/SamsungTacx/Tacx/";
    }
    MainWindow w(dir);
    w.show();

    return a.exec();
}
