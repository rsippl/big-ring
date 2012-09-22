#include "videocontroller.h"

#include <QTimer>

VideoController::VideoController(VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_videoWidget(videoWidget),
	_playDelayTimer(new QTimer(this))
{
	connect(_videoWidget, SIGNAL(videoDurationAvailable(qint64)),
			this, SLOT(videoDurationAvailable(qint64)));

	_playDelayTimer->setSingleShot(true);
	_playDelayTimer->setInterval(250);
	connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));
}

void VideoController::realLiveVideoSelected(RealLiveVideo rlv)
{
	if (_playDelayTimer->isActive())
		_playDelayTimer->stop();
	_currentRlv = rlv;

	_playDelayTimer->start();
}

void VideoController::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		return;
	}

	if (!_currentRlv.isValid())
		return;

	const Course& course = _currentRlv.courses()[courseNr];
	float startDistance = course.start();
	quint32 frame = _currentRlv.frameForDistance(startDistance);
	qDebug() << "slope at start = " << _currentRlv.slopeForDistance(startDistance);

	double videoDuration = _videoWidget->videoDuration();

	double totalFrames = (videoDuration / 1000.0) * _currentRlv.videoInformation().frameRate();
	qDebug() << "duration = " << videoDuration << " total frames " << totalFrames;
	qDebug() << "Going to frame " << frame;
	double b = frame / totalFrames;

	_videoWidget->setPosition(videoDuration * b);
}

void VideoController::playVideo()
{
	_videoWidget->loadVideo(_currentRlv.videoInformation().videoFilename());
}

void VideoController::videoDurationAvailable(qint64)
{
	_videoWidget->playVideo();
}
