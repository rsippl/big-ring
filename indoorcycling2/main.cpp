#include "mainwindow.h"
#include <QApplication>
#include <gst/gst.h>
int main(int argc, char *argv[])
{
    qputenv("GST_PLUGIN_PATH", "/home/ibooij/homedev/build/qtgstreamer/elements/gstqtvideosink/");
    QApplication a(argc, argv);
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
