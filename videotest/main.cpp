#include "videotestmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoTestMainWindow w;
    w.show();

    return a.exec();
}
