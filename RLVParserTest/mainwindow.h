#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "reallivevideo.h"

class RealLiveVideoImporter;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const RealLiveVideoImporter& parser, QWidget *parent = 0);

signals:
    void importFinished(RealLiveVideoList rlvs);
};

#endif // MAINWINDOW_H
