#include <QtWidgets/QApplication>
#include <QtCore/QtDebug>
#include "antchanneltype.h"
#include "antcentraldispatch.h"
#include "anttestappmainwindow.h"

using indoorcycling::AntCentralDispatch;
using indoorcycling::AntTestAppMainWindow;
int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    AntTestAppMainWindow mainWindow;
    AntCentralDispatch acd;

    QObject::connect(&acd, &AntCentralDispatch::antUsbStickScanningFinished, &mainWindow,
                     &AntTestAppMainWindow::antUsbStickFound);
    QObject::connect(&acd, &AntCentralDispatch::initializationFinished, &mainWindow,
                     &AntTestAppMainWindow::initializationFinished);
    QObject::connect(&acd, &AntCentralDispatch::initializationFinished, &acd, [&acd](bool success) {
        if (success) {
            acd.searchForSensor(indoorcycling::CHANNEL_TYPE_HR, 27717);
        }
    });
    QObject::connect(&acd, &AntCentralDispatch::sensorNotFound, &mainWindow,
                     &AntTestAppMainWindow::searchTimedOut);
    QObject::connect(&acd, &AntCentralDispatch::sensorFound, &mainWindow,
                     &AntTestAppMainWindow::setSensor);
    QObject::connect(&acd, &AntCentralDispatch::heartRateMeasured, &mainWindow,
                     &AntTestAppMainWindow::setHeartRate);


    acd.initialize();
    qDebug() << "Starting";

//    ANTController* controller = new ANTController;
//    QObject::connect(controller, &ANTController::destroyed, &a, &QCoreApplication::quit);

    mainWindow.show();
    a.exec();
}
