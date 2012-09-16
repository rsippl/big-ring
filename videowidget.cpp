#include "videowidget.h"

#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QGridLayout>
#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QWidget(parent),
	_playDelayTimer(new QTimer(this))
{
	_playDelayTimer->setSingleShot(true);
	_playDelayTimer->setInterval(250);
	connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));

	setAutoFillBackground(false);
//	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	QPalette palette = this->palette();
	palette.setColor(QPalette::Background, Qt::black);
	setPalette(palette);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


	QGridLayout *grid = new QGridLayout(this);
	grid->setMargin(0);

	_mediaObject = new Phonon::MediaObject(this);
	_mediaObject->setTickInterval(1000);
	_videoWidget = new Phonon::VideoWidget(this);
	Phonon::createPath(_mediaObject, _videoWidget);

	connect(_mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
	grid->addWidget(_videoWidget, 0, 0, 3, 1);
}

VideoWidget::~VideoWidget()
{

}

void VideoWidget::playVideo()
{
	_mediaObject->setCurrentSource(_currentRealLiveVideo.videoInformation().videoFilename());
	_mediaObject->play();
}

void VideoWidget::tick(qint64)
{
	qDebug() << "tick";
}


void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
	if (_playDelayTimer->isActive())
		_playDelayTimer->stop();
	_currentRealLiveVideo = rlv;

	_playDelayTimer->start();
}

void VideoWidget::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		return;
	}

	if (!_currentRealLiveVideo.isValid())
		return;

	const Course& course = _currentRealLiveVideo.courses()[courseNr];
	course.start();
}

