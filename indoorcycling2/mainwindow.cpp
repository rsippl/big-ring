#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore/QtDebug>
#include <QtCore/QUrl>
#include <random>

#include "rlvtablemodel.h"
MainWindow::MainWindow(QString dir, QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _importer(new RealLifeVideoImporter(this)),
    _preview(new PreviewVideoWidget(this))
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
        QString realUri = rlvs[0].videoInformation().videoFilename();
        _preview->setUri(realUri);
        _preview->play();
    }
}

void MainWindow::selectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    qDebug() << "selection changed";
    QModelIndexList indexes = selected.indexes();
    if (!indexes.isEmpty()) {
        int rlvIndex = indexes[0].row();
        QString realUri = _rlvList[rlvIndex].videoInformation().videoFilename();
        _preview->setUri(realUri);
        _preview->play();
    }
}
