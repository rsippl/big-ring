#include <QtWidgets/QApplication>
#include <QtCore/QtDebug>
#include "antcentraldispatch.h"
#include "anttestappmainwindow.h"

using indoorcycling::AntCentralDispatch;

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    AntTestAppMainWindow mainWindow;
    AntCentralDispatch acd;

    QObject::connect(&acd, &AntCentralDispatch::antUsbStickScanningFinished, &mainWindow,
                     &AntTestAppMainWindow::antUsbStickFound);
    QObject::connect(&acd, &AntCentralDispatch::initializationFinished, &mainWindow,
                     &AntTestAppMainWindow::initializationFinished);
    acd.initialize();
    qDebug() << "Starting";

//    ANTController* controller = new ANTController;
//    QObject::connect(controller, &ANTController::destroyed, &a, &QCoreApplication::quit);

    mainWindow.show();
    a.exec();
}
