#include "videocontroller.h"

#include <QTimer>

namespace {
const float SPEED = 15.0f / 3.6f;

const float videoUpdateInterval = 1000; // ms
}
VideoController::VideoController(VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_videoWidget(videoWidget),
	_playDelayTimer(new QTimer(this)),
	_updateTimer(new QTimer(this)),
	_currentDistance(0.0f)
{
	connect(_videoWidget, SIGNAL(videoDurationAvailable(qint64)),
			this, SLOT(videoDurationAvailable(qint64)));

	_playDelayTimer->setSingleShot(true);
	_playDelayTimer->setInterval(250);
	connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));

	_updateTimer->setInterval(videoUpdateInterval);
	connect(_updateTimer, SIGNAL(timeout()), SLOT(updateVideo()));
	_updateTimer->start();
}

void VideoController::realLiveVideoSelected(RealLiveVideo rlv)
{
	if (_playDelayTimer->isActive())
		_playDelayTimer->stop();
	_currentRlv = rlv;

	_playDelayTimer->start();

	_currentDistance = 0.0f;
}

void VideoController::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		return;
	}

	if (!_currentRlv.isValid())
		return;

	const Course& course = _currentRlv.courses()[courseNr];
	_currentDistance = course.start();
	quint32 frame = _currentRlv.frameForDistance(_currentDistance);
	qDebug() << "slope at start = " << _currentRlv.slopeForDistance(_currentDistance);

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

void VideoController::updateVideo()
{
	if (!_currentRlv.isValid())
		return;

	float metersPerFrame = _currentRlv.metersPerFrame(_currentDistance);

	// speed is 30 km/h -> 8.3333 m/s
	float framesPerSecond = SPEED / metersPerFrame;

	float rate = framesPerSecond / _currentRlv.videoInformation().frameRate();
	_videoWidget->setRate(rate);
}

void VideoController::videoDurationAvailable(qint64)
{
	_videoWidget->playVideo();
}
