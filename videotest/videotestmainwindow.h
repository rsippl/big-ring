#ifndef VIDEOTESTMAINWINDOW_H
#define VIDEOTESTMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class VideoTestMainWindow;
}

class VideoTestMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoTestMainWindow(QWidget *parent = 0);
    ~VideoTestMainWindow();

private:
    Ui::VideoTestMainWindow *ui;
};

#endif // VIDEOTESTMAINWINDOW_H
