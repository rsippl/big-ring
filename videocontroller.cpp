#include "videocontroller.h"

#include <QtDebug>
#include <QDateTime>
#include <QTimer>

namespace {
const float SPEED = 100.0f / 3.6f;
const float videoUpdateInterval = 1000; // ms
}

VideoController::VideoController(VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_videoWidget(videoWidget),
	_playDelayTimer(new QTimer(this)),
	_updateTimer(new QTimer(this)),
	_currentDistance(0.0f)
{
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

	setDistance(0.0f);
	_lastTime = QDateTime::currentMSecsSinceEpoch();
}

void VideoController::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		return;
	}

	if (!_currentRlv.isValid())
		return;

	const Course& course = _currentRlv.courses()[courseNr];
	setDistance(course.start());
	_lastTime = QDateTime::currentMSecsSinceEpoch();
	quint32 frame = _currentRlv.frameForDistance(_currentDistance);
	qDebug() << "slope at start = " << _currentRlv.slopeForDistance(_currentDistance);

	_videoWidget->setPosition(frame, _currentRlv.videoInformation().frameRate());
	_videoWidget->playVideo();

}

void VideoController::playVideo()
{
	_videoWidget->loadVideo(_currentRlv.videoInformation().videoFilename());
	_videoWidget->playVideo();
}

void VideoController::updateVideo()
{
	if (!_currentRlv.isValid())
		return;

	// update distance
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
	qint64 elapsed = currentTime - _lastTime;

	float distanceTravelled = (SPEED * elapsed) * 0.001;

	setDistance(_currentDistance + distanceTravelled);

	_lastTime = currentTime;

	float metersPerFrame = _currentRlv.metersPerFrame(_currentDistance);
	float slope = _currentRlv.slopeForDistance(_currentDistance);
	emit slopeChanged(slope);

	// speed is 30 km/h -> 8.3333 m/s
	float framesPerSecond = SPEED / metersPerFrame;
	_videoWidget->setRate(framesPerSecond);
}

void VideoController::setDistance(float distance)
{
	_currentDistance = distance;
	emit distanceChanged(distance);
}
