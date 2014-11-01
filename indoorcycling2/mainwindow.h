#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QItemSelection>

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
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    Ui::MainWindow *_ui;
    RealLifeVideoImporter *_importer;

    PreviewVideoWidget* _preview;
    RealLifeVideoList _rlvList;
};

#endif // MAINWINDOW_H
