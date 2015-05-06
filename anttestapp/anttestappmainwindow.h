#ifndef ANTTESTAPPMAINWINDOW_H
#define ANTTESTAPPMAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include "antchanneltype.h"
namespace Ui {
class AntTestAppMainWindow;
}

namespace indoorcycling {
class AntTestAppMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AntTestAppMainWindow(QWidget *parent = 0);
    ~AntTestAppMainWindow();
public slots:
    void antUsbStickFound(bool found);
    void initializationFinished(bool success);
    void searchTimedOut(AntChannelType channelType);
    void setSensor(AntChannelType channelType, int deviceNumber);

    void setHeartRate(int bpm);
signals:
    void startSearch(AntChannelType channelType);
private slots:
    void on_pushButton_clicked();

private:
    Ui::AntTestAppMainWindow *ui;
};
}
#endif // ANTTESTAPPMAINWINDOW_H
