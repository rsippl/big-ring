#include "mainwindow.h"

#include "reallivevideoimporter.h"
#include "rlvlistwidget.h"
#include "videowidget.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QWidget>

MainWindow::MainWindow(const RealLiveVideoImporter& parser, QWidget *parent) :
    QMainWindow(parent)
{
    connect(&parser, SIGNAL(importFinished(RealLiveVideoList)), SIGNAL(importFinished(RealLiveVideoList)));

    setGeometry(0, 0, 1024, 768);

    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(centralWidget);
    layout->addLayout(setupSideBar());
//    RlvListWidget* listWidget = new RlvListWidget(centralWidget);
//    listWidget->setFixedWidth(300);
//    layout->addWidget(listWidget);

    VideoWidget* videoWidget = new VideoWidget(centralWidget);
    videoWidget->setMinimumWidth(800);
    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(videoWidget);

    setCentralWidget(centralWidget);

    QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), videoWidget, SLOT(realLiveVideoSelected(RealLiveVideo)));
    QObject::connect(this, SIGNAL(importFinished(RealLiveVideoList)), rlvListWidget, SLOT(setRealLiveVideos(RealLiveVideoList)));
    QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), SLOT(rlvSelected(RealLiveVideo)));
	QObject::connect(courseListWidget, SIGNAL(currentRowChanged(int)),
					 videoWidget, SLOT(courseSelected(int)));
}

QLayout* MainWindow::setupSideBar()
{
    QVBoxLayout* layout = new QVBoxLayout;
    rlvListWidget = new RlvListWidget();
    rlvListWidget->setFixedWidth(300);
    layout->addWidget(rlvListWidget);

    courseListWidget = new QListWidget;
    layout->addWidget(courseListWidget);



    return layout;
}

void MainWindow::rlvSelected(RealLiveVideo rlv)
{
    courseListWidget->clear();
    foreach(const Course& course, rlv.courses()) {
        new QListWidgetItem(course.name(), courseListWidget);
    }
}
