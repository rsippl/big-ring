#ifndef ANTTESTAPPMAINWINDOW_H
#define ANTTESTAPPMAINWINDOW_H

#include <QtWidgets/QMainWindow>

namespace Ui {
class AntTestAppMainWindow;
}

class AntTestAppMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AntTestAppMainWindow(QWidget *parent = 0);
    ~AntTestAppMainWindow();
public slots:
    void antUsbStickFound(bool found);
    void initializationFinished(bool success);
private:
    Ui::AntTestAppMainWindow *ui;
};

#endif // ANTTESTAPPMAINWINDOW_H
