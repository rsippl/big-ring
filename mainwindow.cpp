#include "mainwindow.h"

#include "antcontroller.h"
#include "reallivevideoimporter.h"
#include "rlvlistwidget.h"
#include "videocontroller.h"
#include "videowidget.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QWidget>
#include <cmath>

MainWindow::MainWindow(const RealLiveVideoImporter& parser, const ANTController&, QWidget *parent) :
	QMainWindow(parent), _simulation(_cyclist), videoWidget(new VideoWidget(this)),
	videoController(new VideoController(_cyclist, videoWidget, this))
{
	connect(&parser, SIGNAL(importFinished(RealLiveVideoList)), SIGNAL(importFinished(RealLiveVideoList)));

	setGeometry(0, 0, 1024, 768);

	QWidget* centralWidget = new QWidget(this);
	_layout = new QHBoxLayout(centralWidget);
	_layout->addLayout(setupSideBar(centralWidget));

	_layout->addLayout(setUpMain(centralWidget));

	setCentralWidget(centralWidget);

	QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), &_simulation, SLOT(rlvSelected(RealLiveVideo)));
	QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), videoController, SLOT(realLiveVideoSelected(RealLiveVideo)));
	QObject::connect(this, SIGNAL(importFinished(RealLiveVideoList)), rlvListWidget, SLOT(setRealLiveVideos(RealLiveVideoList)));
	QObject::connect(rlvListWidget, SIGNAL(realLiveVideoSelected(RealLiveVideo)), SLOT(rlvSelected(RealLiveVideo)));
	QObject::connect(courseListWidget, SIGNAL(currentRowChanged(int)),
					 &_simulation, SLOT(courseSelected(int)));
	QObject::connect(courseListWidget, SIGNAL(currentRowChanged(int)),
					 videoController, SLOT(courseSelected(int)));

	connect(playButton, SIGNAL(clicked(bool)), &_simulation, SLOT(play(bool)));

	connect(&_cyclist, SIGNAL(distanceChanged(float)), SLOT(distanceChanged(float)));
	connect(&_simulation, SIGNAL(slopeChanged(float)), SLOT(slopeChanged(float)));
	connect(&_simulation, SIGNAL(runTimeChanged(QTime)), SLOT(runTimeChanged(QTime)));
	connect(&_simulation, SIGNAL(playing(bool)), videoController, SLOT(play(bool)));

	connect(&_cyclist, SIGNAL(speedChanged(float)), SLOT(speedChanged(float)));

	grabKeyboard();
}

QLayout* MainWindow::setUpMain(QWidget* centralWidget)
{
	QVBoxLayout* layout = new QVBoxLayout();

	videoWidget->setMinimumWidth(800);
	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	layout->addWidget(videoWidget);

	QHBoxLayout* dials = new QHBoxLayout();

	_distanceLabel = createLabel(QString("0 m"), Qt::blue, centralWidget);
	_timeLabel = createLabel(QString("--:--:--"), Qt::cyan, centralWidget);
	slopeLabel = createLabel(QString("0 %"), Qt::red, centralWidget);
	_speedLabel = createLabel("-- km/h", Qt::yellow, centralWidget);

	dials->addWidget(_distanceLabel);
	dials->addWidget(_timeLabel);
	dials->addWidget(slopeLabel);
	dials->addWidget(_speedLabel);

	layout->addLayout(dials);



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

	playButton = new QPushButton("Play", centralWidget);
	playButton->setCheckable(true);
	playButton->setEnabled(videoController->isBufferFull());
	connect(videoController, SIGNAL(bufferFull(bool)), playButton, SLOT(setEnabled(bool)));
	connect(videoController, SIGNAL(playing(bool)), playButton, SLOT(setChecked(bool)));
	layout->addWidget(playButton);
	return layout;
}

QLabel *MainWindow::createLabel(const QString& text, Qt::GlobalColor color, QWidget *centralWidget)
{
	QFont font;
	font.setPointSize(32);
	font.setBold(true);
	QPalette palette;
	palette.setColor(QPalette::Window, Qt::black);
	palette.setColor(QPalette::WindowText, color);

	QLabel* label = new QLabel(text, centralWidget);
	label->setAlignment(Qt::AlignCenter);
	label->setFont(font);
	label->setAutoFillBackground(true);
	label->setPalette(palette);

	return label;
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
		_distanceLabel->setText(QString("%1 %2 m").arg(distanceInMeters / 1000)
							   .arg(distanceInMeters % 1000, width, 10, QLatin1Char('0')));
	else
		_distanceLabel->setText(QString("%1 m").arg(distanceInMeters % 1000));
}

void MainWindow::slopeChanged(float slope)
{
	slopeLabel->setText(QString("%1 %").arg(slope, 2, 'f', 1));
}

void MainWindow::runTimeChanged(QTime runTime)
{
	_timeLabel->setText(runTime.toString(Qt::ISODate));
}

void MainWindow::speedChanged(float speed)
{
	float speedKmPH = speed * 3.6;
	_speedLabel->setText(QString("%1 km/h").arg(speedKmPH, 1, 'f', 1));
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
		playButton->hide();
		removeMargins();
		showFullScreen();
	} else if (event->key() == Qt::Key_Escape) {
		showNormal();
		restoreMargins();
		rlvListWidget->show();
		courseListWidget->show();
		playButton->show();
		rlvListWidget->setEnabled(true);
		courseListWidget->setEnabled(true);
	} else if (event->key() == Qt::Key_Space) {
		if (playButton->isChecked()) {
			_simulation.play(false);
		} else if (videoController->isBufferFull()) {
			_simulation.play(true);
		}
	}
}
