#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QItemSelection>

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
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    void step();

    Ui::MainWindow *_ui;
    RealLifeVideoImporter *_importer;

    VideoTileView* _tileView;
    RealLifeVideoList _rlvList;

    QTimer* _playTimer;
    QTime _time;
    float _currentDistance;
};

#endif // MAINWINDOW_H
