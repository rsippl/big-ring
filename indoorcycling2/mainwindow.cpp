#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <random>

#include "rlvtablemodel.h"
#include "run.h"

MainWindow::MainWindow(QString dir, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _importer(new RealLifeVideoImporter(this)),
    _antController(new ANTController(this)),
    _tileView(new VideoTileView(this))
{
    _ui->setupUi(this);

    qDebug() << "starting from " << dir;
    connect(_importer, &RealLifeVideoImporter::importFinished, this, &MainWindow::importFinished);
    _importer->parseRealLiveVideoFilesFromDir(dir);

    _tileView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _ui->centralwidget->layout()->addWidget(_tileView);

    connect(_tileView, &VideoTileView::startRlv, _tileView, [=](RealLifeVideo& rlv) {
        qDebug() << "main window:" << rlv.name();
        startRun(rlv);
    });

    _tileView->show();
}

MainWindow::~MainWindow()
{
    delete _ui;
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
    Run* run = new Run(*_antController, rlv, course);
    run->start();
    this->releaseKeyboard();
    connect(run, &Run::stopped, run, [this,run]() {
        qDebug() << "run finished";
        this->show();
        run->deleteLater();
    });
}
