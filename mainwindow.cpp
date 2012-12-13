#include "mainwindow.h"

#include "reallivevideoimporter.h"
#include "rlvlistwidget.h"
#include "videocontroller.h"
#include "videowidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QWidget>

MainWindow::MainWindow(const RealLiveVideoImporter& parser, QWidget *parent) :
    QMainWindow(parent)
{
    connect(&parser, SIGNAL(importFinished(RealLiveVideoList)), SIGNAL(importFinished(RealLiveVideoList)));

    setGeometry(0, 0, 1024, 768);

    QWidget* centralWidget = new QWidget(this);
    _layout = new QHBoxLayout(centralWidget);
	_layout->addLayout(setupSideBar(centralWidget));
    //    RlvListWidget* listWidget = new RlvListWidget(centralWidget);
    //    listWidget->setFixedWidth(300);
    //    layout->addWidget(listWidget);

	_layout->addLayout(setUpMain(centralWidget));

    setCentralWidget(centralWidget);

	QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), videoController, SLOT(realLiveVideoSelected(RealLiveVideo)));
    QObject::connect(this, SIGNAL(importFinished(RealLiveVideoList)), rlvListWidget, SLOT(setRealLiveVideos(RealLiveVideoList)));
    QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), SLOT(rlvSelected(RealLiveVideo)));
    QObject::connect(courseListWidget, SIGNAL(currentRowChanged(int)),
			 videoController, SLOT(courseSelected(int)));

	connect(videoController, SIGNAL(distanceChanged(float)), SLOT(distanceChanged(float)));
	connect(videoController, SIGNAL(slopeChanged(float)), SLOT(slopeChanged(float)));
    grabKeyboard();
}

QLayout* MainWindow::setUpMain(QWidget* centralWidget)
{
	QVBoxLayout* layout = new QVBoxLayout(centralWidget);

	videoWidget = new VideoWidget(centralWidget);
	videoWidget->setMinimumWidth(800);
	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	layout->addWidget(videoWidget);

	QHBoxLayout* dials = new QHBoxLayout(centralWidget);

	distanceLabel = new QLabel("0 m", centralWidget);
	dials->addWidget(distanceLabel);
	slopeLabel = new QLabel("0 %", centralWidget);
	dials->addWidget(slopeLabel);

	layout->addLayout(dials);

	videoController = new VideoController(videoWidget, this);

	return layout;
}

QLayout* MainWindow::setupSideBar(QWidget* centralWidget)
{
	QVBoxLayout* layout = new QVBoxLayout(centralWidget);
	rlvListWidget = new RlvListWidget(centralWidget);
    rlvListWidget->setFixedWidth(300);
    layout->addWidget(rlvListWidget);

	courseListWidget = new QListWidget(centralWidget);
	courseListWidget->setFixedWidth(300);
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

void MainWindow::doFullscreen()
{
    rlvListWidget->hide();
    courseListWidget->hide();
	showFullScreen();
}

void MainWindow::distanceChanged(float distance)
{
	qint32 distanceInMeters = (qint32) distance;
	distanceLabel->setText(QString("%1 m").arg(distanceInMeters));
}

void MainWindow::slopeChanged(float slope)
{
	slopeLabel->setText(QString("%1 %%").arg(slope));
}

void MainWindow::removeMargins()
{
    int l,t,r,b;

    _layout->getContentsMargins(&l,&t,&r,&b);
    _margins = QMargins(l,t,r,b);
    _layout->setContentsMargins(0, 0, 0, 0);
}

void MainWindow::restoreMargins()
{
    _layout->setContentsMargins(_margins);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_F) {
		rlvListWidget->hide();
		courseListWidget->hide();
		removeMargins();
		showFullScreen();
	} else if (event->key() == Qt::Key_Escape) {
		showNormal();
		restoreMargins();
		rlvListWidget->show();
		courseListWidget->show();
	}
}
