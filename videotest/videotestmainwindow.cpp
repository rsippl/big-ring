#include "videotestmainwindow.h"
#include "ui_videotestmainwindow.h"

VideoTestMainWindow::VideoTestMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoTestMainWindow)
{
    ui->setupUi(this);
}

VideoTestMainWindow::~VideoTestMainWindow()
{
    delete ui;
}
