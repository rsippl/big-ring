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

	_videoPlayer = new Phonon::VideoPlayer(Phonon::VideoCategory, this);

	grid->addWidget(_videoPlayer, 1, 0, 3, 1);
}

VideoWidget::~VideoWidget()
{
	_videoPlayer->stop();
}

void VideoWidget::playVideo()
{
	_videoPlayer->play(Phonon::MediaSource(_currentRealLiveVideo.videoInformation().videoFilename()));
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

