#include "anttestappmainwindow.h"
#include "ui_anttestappmainwindow.h"

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
    if (success) {
        ui->InitializingLabel->setText("Successfully initialized ANT+ USB Stick");
    } else {
        ui->InitializingLabel->setText("Could not Successfully initialize ANT+ USB Stick");
    }
}
