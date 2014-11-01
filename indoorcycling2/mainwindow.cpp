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

//    connect(_ui->pushButton, &QPushButton::clicked, _preview, &PreviewVideoWidget::showFullScreen);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::importFinished(RealLifeVideoList rlvs)
{
    qDebug() << "import finished";
    _ui->rlvTable->setModel(new RlvTableModel(rlvs, this));

    if (!rlvs.isEmpty()) {
        int i = std::rand() % rlvs.size();
        QString realUri = rlvs[i].videoInformation().videoFilename();
        _preview->setUri(realUri);
        _preview->play();
    }
}
