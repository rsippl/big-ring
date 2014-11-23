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
    _preview(new PreviewVideoWidget(this)),
    _playTimer(new QTimer(this))
{
    _ui->setupUi(this);

    qDebug() << "starting from " << dir;
    connect(_importer, &RealLifeVideoImporter::importFinished, this, &MainWindow::importFinished);
    _importer->parseRealLiveVideoFilesFromDir(dir);


    _preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _preview->setMinimumWidth(300);
    _preview->setMinimumHeight(600);
    _ui->centralwidget->layout()->addWidget(_preview);

    _preview->show();

    connect(_preview, &PreviewVideoWidget::videoLoaded, [this]() {
       _playTimer->start();
       _time.start();
    });
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
    qDebug() << "import finished";
    _rlvList = rlvs;
    _ui->rlvTable->setModel(new RlvTableModel(rlvs, this));
    qDebug() << _ui->rlvTable->selectionModel();
    connect(_ui->rlvTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::selectionChanged);

    if (!rlvs.isEmpty()) {
        _preview->setRealLifeVideo(rlvs[0]);
        _preview->setCourse(rlvs[0].courses().last());
        _preview->play();
    }
}

void MainWindow::selectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    qDebug() << "selection changed";
    QModelIndexList indexes = selected.indexes();
    if (!indexes.isEmpty()) {
        int rlvIndex = indexes[0].row();
        RealLifeVideo video = _rlvList[rlvIndex];
        _preview->setRealLifeVideo(_rlvList[rlvIndex]);
        int index = video.courses().size() >= 4 ? 3 : video.courses().size() - 1;
        _preview->setCourse(video.courses()[13]);
        int i = 0;
        _currentDistance = video.courses()[index].start();
        for (Course course: _rlvList[rlvIndex].courses()) {
                    qDebug() << "Course " << i++ << course.name();
                }
//        _preview->setUri(realUri);

        _preview->play();

    }
}

void MainWindow::step()
{
    int milliseconds = _time.restart();
    float distance = 10 * milliseconds / 1000.0;
    _currentDistance += distance;
    qDebug() << "current distance" << _currentDistance;
    _preview->setDistance(_currentDistance);
}
