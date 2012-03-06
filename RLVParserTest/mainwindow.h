#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>

#include "reallivevideo.h"

class RealLiveVideoImporter;
class RlvListWidget;
class QListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const RealLiveVideoImporter& parser, QWidget *parent = 0);

signals:
    void importFinished(RealLiveVideoList rlvs);

private slots:
    void rlvSelected(RealLiveVideo rlv);

private:
    QLayout* setupSideBar();
    QListWidget* courseListWidget;
    RlvListWidget* rlvListWidget;
};

#endif // MAINWINDOW_H
