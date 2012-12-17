#include "videocontroller.h"

#include <QtDebug>
#include <QDateTime>
#include <QTimer>

namespace {
const float SPEED = 130.0f / 3.6f;
const float videoUpdateInterval = 1000; // ms
}

VideoController::VideoController(VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_videoWidget(videoWidget),
	_updateTimer(new QTimer(this)),
	_currentDistance(0.0f),
	_running(false)
{
	_updateTimer->setInterval(videoUpdateInterval);
	connect(_updateTimer, SIGNAL(timeout()), SLOT(updateVideo()));
}

void VideoController::realLiveVideoSelected(RealLiveVideo rlv)
{
	_updateTimer->stop();
	_videoWidget->stop();
	_currentRlv = rlv;
	_videoWidget->loadVideo(_currentRlv.videoInformation().videoFilename());
	setDistance(0.0f);
	_lastTime = QDateTime::currentMSecsSinceEpoch();
}

void VideoController::courseSelected(int courseNr)
{
	_updateTimer->stop();
	_videoWidget->stop();
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

	_videoWidget->setPosition(frame);
//	_videoWidget->playVideo();
	//	_updateTimer->start();
}

void VideoController::play(bool doPlay)
{
	if (doPlay) {
		_videoWidget->playVideo();
		_updateTimer->start();
	} else {
		_videoWidget->stop();
		_updateTimer->stop();
	}
}


void VideoController::updateVideo()
{
	if (!_currentRlv.isValid())
		return;

	// update distance
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
	qint64 elapsed = currentTime - _lastTime;

	float distanceTravelled = (SPEED * elapsed) * 0.001;

	qDebug() << elapsed << "  distance travelled = " << distanceTravelled;
	setDistance(_currentDistance + distanceTravelled);

	_lastTime = currentTime;

	float metersPerFrame = _currentRlv.metersPerFrame(_currentDistance);
	float slope = _currentRlv.slopeForDistance(_currentDistance);
	emit slopeChanged(slope);
	emit altitudeChanged(_currentRlv.altitudeForDistance(_currentDistance));

	// speed is 30 km/h -> 8.3333 m/s
	float framesPerSecond = SPEED / metersPerFrame;
	_videoWidget->setRate(framesPerSecond);
}

void VideoController::setDistance(float distance)
{
	_currentDistance = distance;
	emit distanceChanged(distance);
}
