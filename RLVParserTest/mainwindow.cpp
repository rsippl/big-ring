#include "mainwindow.h"

#include "rlvlistwidget.h"
#include "videowidget.h"

#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(const RealLiveVideoParser& parser, QWidget *parent) :
    QMainWindow(parent)
{
    connect(&parser, SIGNAL(importFinished(RealLiveVideoList)), SIGNAL(importFinished(RealLiveVideoList)));

    setGeometry(0, 0, 1024, 768);

    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    RlvListWidget* listWidget = new RlvListWidget(centralWidget);
    listWidget->setFixedWidth(300);
    layout->addWidget(listWidget);

    VideoWidget* videoWidget = new VideoWidget(centralWidget);
    videoWidget->setMinimumWidth(800);
    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(videoWidget);

    setCentralWidget(centralWidget);

    QObject::connect(listWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), videoWidget, SLOT(realLiveVideoSelected(RealLiveVideo)));
    QObject::connect(this, SIGNAL(importFinished(RealLiveVideoList)), listWidget, SLOT(setRealLiveVideos(RealLiveVideoList)));
}
