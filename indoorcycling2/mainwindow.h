#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QScopedPointer>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStackedWidget>

#include "antcontroller.h"
#include "reallifevideo.h"
#include "reallifevideoimporter.h"

class Cyclist;
class VideoTileView;
class NewVideoWidget;
class Run;
class Simulation;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QString dir, QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *);
private slots:
    void importFinished(RealLifeVideoList rlvs);
private:
    void step();
    void startRun(RealLifeVideo rlv);

//    Ui::MainWindow *_ui;
    RealLifeVideoImporter *_importer;

    ANTController* _antController;
    Cyclist* _cyclist;
    Simulation* _simulation;
    QScopedPointer<Run,QScopedPointerDeleteLater> _run;

    QStackedWidget* const _stackedWidget;
    VideoTileView* const _tileView;
    NewVideoWidget* const _videoWidget;
    RealLifeVideoList _rlvList;
};

#endif // MAINWINDOW_H
