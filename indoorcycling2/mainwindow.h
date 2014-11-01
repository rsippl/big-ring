#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "previewvideowidget.h"
#include "reallifevideo.h"
#include "reallifevideoimporter.h"


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
    Ui::MainWindow *_ui;
    RealLifeVideoImporter *_importer;

    PreviewVideoWidget* _preview;
};

#endif // MAINWINDOW_H
