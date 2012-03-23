#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
class QTimer;

#include "reallivevideo.h"

class RealLiveVideoImporter;
class RlvListWidget;
class QListWidget;
class QBoxLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const RealLiveVideoImporter& parser, QWidget *parent = 0);

signals:
    void importFinished(RealLiveVideoList rlvs);

protected:
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void rlvSelected(RealLiveVideo rlv);
    void doFullscreen();
private:
    QLayout* setupSideBar();
    void removeMargins();
    void restoreMargins();

    QListWidget* courseListWidget;
    RlvListWidget* rlvListWidget;
    QMargins _margins;
    QBoxLayout* _layout;
};

#endif // MAINWINDOW_H
