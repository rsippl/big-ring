#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrl>


#include "cyclist.h"
#include "run.h"
#include "videotileview.h"
#include "newvideowidget.h"
#include "simulation.h"

MainWindow::MainWindow(QString dir, QWidget *parent) :
    QWidget(parent, Qt::Window),
    _importer(new RealLifeVideoImporter(this)),
    _antController(new ANTController(this)),
    _cyclist(new Cyclist(this)),
    _simulation(new Simulation(*_cyclist, this)),
    _stackedWidget(new QStackedWidget),
    _tileView(new VideoTileView),
    _videoWidget(new NewVideoWidget(*_simulation))
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    qDebug() << "starting from " << dir;
    connect(_importer, &RealLifeVideoImporter::importFinished, this, &MainWindow::importFinished);
    _importer->parseRealLiveVideoFilesFromDir(dir);

    _tileView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _stackedWidget->addWidget(_tileView);
    _stackedWidget->addWidget(_videoWidget);

    layout->addWidget(_stackedWidget);

    connect(_tileView, &VideoTileView::startRlv, _tileView, [=](RealLifeVideo& rlv) {
        qDebug() << "main window:" << rlv.name();
        startRun(rlv);
    });

    _tileView->show();
}

MainWindow::~MainWindow()
{
    // empty
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "received key" << event->key() << event->text();
    switch(event->key()) {
    case Qt::Key_F:
        showFullScreen();
        break;
    case Qt::Key_M:
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
        break;
    case Qt::Key_Escape:
        if (_run) {
            _run->stop();
        }
    default:
        QWidget::keyPressEvent(event);
        return;
        break;
    }
    event->accept();
}

void MainWindow::importFinished(RealLifeVideoList rlvs)
{
    _tileView->rlvsLoaded(rlvs);
    qDebug() << "import finished";
    _rlvList = rlvs;
}

void MainWindow::startRun(RealLifeVideo rlv)
{
    Course course = rlv.courses()[0];
    _run.reset(new Run(*_antController, _simulation, rlv, course, _videoWidget));

    _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_videoWidget));
    connect(_run.data(), &Run::stopped, _run.data(), [this]() {
        qDebug() << "run finished";
        _stackedWidget->setCurrentIndex(_stackedWidget->indexOf(_tileView));
        _run.reset();
    });
    _run->start();

}
