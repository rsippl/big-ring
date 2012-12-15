#include "mainwindow.h"

#include "antcontroller.h"
#include "reallivevideoimporter.h"
#include "rlvlistwidget.h"
#include "videocontroller.h"
#include "videowidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QWidget>
#include <cmath>

MainWindow::MainWindow(const RealLiveVideoImporter& parser, const ANTController& controller, QWidget *parent) :
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
	connect(&controller, SIGNAL(heartRateMeasured(quint8)), SLOT(hrChanged(quint8)));

    grabKeyboard();
}

QLayout* MainWindow::setUpMain(QWidget* centralWidget)
{
	QVBoxLayout* layout = new QVBoxLayout();

	videoWidget = new VideoWidget(centralWidget);
	videoWidget->setMinimumWidth(800);
	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	layout->addWidget(videoWidget);

	QHBoxLayout* dials = new QHBoxLayout();
	QFont font;
	font.setPointSize(32);
	font.setBold(true);
	QPalette palette;
	palette.setColor(QPalette::Window, Qt::black);
	palette.setColor(QPalette::WindowText, Qt::blue);

	distanceLabel = new QLabel("0 m", centralWidget);
	distanceLabel->setAlignment(Qt::AlignCenter);
	distanceLabel->setFont(font);
	distanceLabel->setAutoFillBackground(true);
	distanceLabel->setPalette(palette);

	palette.setColor(QPalette::WindowText, Qt::red);
	dials->addWidget(distanceLabel);
	slopeLabel = new QLabel("0 %", centralWidget);
	slopeLabel->setAlignment(Qt::AlignCenter);
	slopeLabel->setFont(font);
	slopeLabel->setAutoFillBackground(true);
	slopeLabel->setPalette(palette);
	dials->addWidget(slopeLabel);
	hrLabel = new QLabel("--", centralWidget);
	dials->addWidget(hrLabel);

	layout->addLayout(dials);

	videoController = new VideoController(videoWidget, this);

	return layout;
}

QLayout* MainWindow::setupSideBar(QWidget* centralWidget)
{
	QVBoxLayout* layout = new QVBoxLayout();
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

void MainWindow::distanceChanged(float distance)
{
	double exponent = exp10(distance);
	int width = (exponent > 3) ? 3 : (int) exponent;
	qint32 distanceInMeters = (qint32) distance;

	if (distanceInMeters > 1000)
		distanceLabel->setText(QString("%1 %2 m").arg(distanceInMeters / 1000)
							   .arg(distanceInMeters % 1000, width, 10, QLatin1Char('0')));
	else
		distanceLabel->setText(QString("%1 m").arg(distanceInMeters % 1000));
}

void MainWindow::slopeChanged(float slope)
{
	slopeLabel->setText(QString("%1 %").arg(slope, 2, 'f', 1));
}

void MainWindow::hrChanged(quint8 hr)
{
	hrLabel->setText(QString("%1 BPM").arg(hr));
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
		rlvListWidget->setDisabled(true);
		courseListWidget->setDisabled(true);
		rlvListWidget->hide();
		courseListWidget->hide();
		removeMargins();
		showFullScreen();
	} else if (event->key() == Qt::Key_Escape) {
		showNormal();
		restoreMargins();
		rlvListWidget->show();
		courseListWidget->show();
		rlvListWidget->setEnabled(true);
		courseListWidget->setEnabled(true);
	}
}
