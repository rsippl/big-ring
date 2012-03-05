#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "reallivevideo.h"
#include "reallivevideoparser.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const RealLiveVideoParser& parser, QWidget *parent = 0);

signals:
    void importFinished(RealLiveVideoList rlvs);
};

#endif // MAINWINDOW_H
