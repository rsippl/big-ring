#include "anttestappmainwindow.h"
#include "ui_anttestappmainwindow.h"

namespace indoorcycling
{

AntTestAppMainWindow::AntTestAppMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AntTestAppMainWindow)
{
    ui->setupUi(this);
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

void AntTestAppMainWindow::searchTimedOut(AntChannelType channelType)
{
    if (channelType == AntChannelType::CHANNEL_TYPE_HR) {
        ui->hrSearchLabel->setText("Search Timed Out");
        ui->pushButton->setEnabled(true);
    }
}

void AntTestAppMainWindow::setSensor(AntChannelType channelType, int deviceNumber)
{
    if (channelType == AntChannelType::CHANNEL_TYPE_HR) {
        ui->hrSearchLabel->setText("Sensor Found");
        ui->hrSensorId->setText(QString::number(deviceNumber));
    }
}

void AntTestAppMainWindow::searchStarted(AntChannelType channelType, int deviceNumber)
{
    if (channelType == AntChannelType::CHANNEL_TYPE_HR) {
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
    emit startSearch(AntChannelType::CHANNEL_TYPE_HR);
}
