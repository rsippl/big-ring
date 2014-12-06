#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <random>

#include "rlvtablemodel.h"
MainWindow::MainWindow(QString dir, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _importer(new RealLifeVideoImporter(this)),
    _tileView(new VideoTileView(this)),
    _playTimer(new QTimer(this))
{
    _ui->setupUi(this);

    qDebug() << "starting from " << dir;
    connect(_importer, &RealLifeVideoImporter::importFinished, this, &MainWindow::importFinished);
    _importer->parseRealLiveVideoFilesFromDir(dir);

    _tileView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _tileView->setMinimumWidth(300);
    _tileView->setMinimumHeight(600);
    _ui->centralwidget->layout()->addWidget(_tileView);

    _tileView->show();

    connect(_playTimer, &QTimer::timeout, [this]() {
       step();
    });
    _playTimer->setInterval(40);
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
    _ui->rlvTable->setModel(new RlvTableModel(rlvs, this));
    qDebug() << _ui->rlvTable->selectionModel();
    connect(_ui->rlvTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::selectionChanged);
}

void MainWindow::selectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    qDebug() << "selection changed";
}

void MainWindow::step()
{
    int milliseconds = _time.restart();
    float distance = 10 * milliseconds / 1000.0;
    _currentDistance += distance;
    qDebug() << "current distance" << _currentDistance;

}
