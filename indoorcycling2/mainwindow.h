#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "antcontroller.h"
#include "previewvideowidget.h"
#include "reallifevideo.h"
#include "reallifevideoimporter.h"
#include "videotileview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString dir, QWidget *parent = 0);
    ~MainWindow();
private slots:
    void importFinished(RealLifeVideoList rlvs);
private:
    void step();
    void startRun(RealLifeVideo rlv);

    Ui::MainWindow *_ui;
    RealLifeVideoImporter *_importer;

    ANTController* _antController;

    VideoTileView* _tileView;
    RealLifeVideoList _rlvList;
};

#endif // MAINWINDOW_H
