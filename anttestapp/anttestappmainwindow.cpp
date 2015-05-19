#include "anttestappmainwindow.h"
#include "ui_anttestappmainwindow.h"

#include <QtCore/QTimer>
namespace indoorcycling
{

AntTestAppMainWindow::AntTestAppMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AntTestAppMainWindow), _timer(new QTimer(this))
{
    ui->setupUi(this);
    connect(_timer, &QTimer::timeout, _timer, [this]() {
        quint16 powerWatts = static_cast<quint16>(ui->spinBox->value());
        emit powerGenerated(powerWatts);
    });
    _timer->setInterval(1000);
}

AntTestAppMainWindow::~AntTestAppMainWindow()
{
    delete ui;
}

void AntTestAppMainWindow::antUsbStickFound(bool found)
{
    if (found) {
        ui->searchStickLabel->setText("ANT+ USB Stick found");
        ui->InitializingLabel->setText("Initializing ANT+ Communication");
    } else {
        ui->searchStickLabel->setText("Error: No ANT+ USB Stick found");
    }
}

void AntTestAppMainWindow::initializationFinished(bool success)
{
    ui->pushButton->setEnabled(success);
    if (success) {
        ui->InitializingLabel->setText("Successfully initialized ANT+ USB Stick");
        ui->pushButton->setEnabled(true);
    } else {
        ui->InitializingLabel->setText("Could not Successfully initialize ANT+ USB Stick");
    }
}

void AntTestAppMainWindow::searchTimedOut(AntSensorType channelType)
{
    if (channelType == AntSensorType::SENSOR_TYPE_HR) {
        ui->hrSearchLabel->setText("Search Timed Out");
        ui->pushButton->setEnabled(true);
    }
}

void AntTestAppMainWindow::setSensor(AntSensorType channelType, int deviceNumber)
{
    if (channelType == AntSensorType::SENSOR_TYPE_HR) {
        ui->hrSearchLabel->setText("Sensor Found");
        ui->hrSensorId->setText(QString::number(deviceNumber));
    }
}

void AntTestAppMainWindow::searchStarted(AntSensorType channelType, int)
{
    if (channelType == AntSensorType::SENSOR_TYPE_HR) {
        ui->pushButton->setEnabled(false);
    }
}

void AntTestAppMainWindow::setHeartRate(int bpm)
{
    ui->currentHrLabel->setText(QString("%1 BPM").arg(QString::number(bpm)));
}
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_clicked()
{
    emit startSearch(AntSensorType::SENSOR_TYPE_HR);
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_2_clicked()
{
    emit startSearch(AntSensorType::SENSOR_TYPE_POWER);
}

void indoorcycling::AntTestAppMainWindow::on_pushButton_3_clicked()
{
    emit openPowerTransmissionChannel();
    _timer->start();
}