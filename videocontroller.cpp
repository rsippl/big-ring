#include "videocontroller.h"

#include <QtDebug>
#include <QDateTime>
#include <QTimer>

namespace {
const float SPEED = 50.0f / 3.6f;
const float videoUpdateInterval = 100; // ms
const quint32 NR_FRAMES_PER_REQUEST = 200;
const int NR_FRAMES_BUFFER_LOW = 150;
}

VideoController::VideoController(VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_videoWidget(videoWidget),
	_currentDistance(0.0f),
	_running(false),
	_newFramesRequested(false),
	_frameRequestId(0)
{
	_decoderThread.start();
	_videoDecoder.moveToThread(&_decoderThread);

	// set up timers
	_playTimer.setInterval(40);
	connect(&_playTimer, SIGNAL(timeout()), SLOT(displayFrame()));
	_updateTimer.setInterval(videoUpdateInterval);
	connect(&_updateTimer, SIGNAL(timeout()), SLOT(updateVideo()));

	// set up video decoder
	connect(&_videoDecoder, SIGNAL(videoLoaded()), SLOT(videoLoaded()));
	connect(&_videoDecoder, SIGNAL(framesReady(FrameList, quint32)), SLOT(framesReady(FrameList, quint32)));
	connect(&_videoDecoder, SIGNAL(seekFinished()), SLOT(seekFinished()));
}

VideoController::~VideoController()
{
	_decoderThread.quit();
	_decoderThread.wait(1000); // wait for a maximum of 1 second.
}

bool VideoController::isBufferFull()
{
	return !_imageQueue.empty();
}

void VideoController::realLiveVideoSelected(RealLiveVideo rlv)
{
	reset();
	_currentRlv = rlv;
	if (!_currentRlv.videoInformation().videoFilename().isEmpty())
		loadVideo(_currentRlv.videoInformation().videoFilename());
	setDistance(0.0f);
	_lastTime = QDateTime::currentMSecsSinceEpoch();
}

void VideoController::courseSelected(int courseNr)
{
	reset();

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

	setPosition(frame);
}

void VideoController::play(bool doPlay)
{
	if (doPlay) {
		_lastTime = QDateTime::currentMSecsSinceEpoch();
		_playTimer.start();
		_updateTimer.start();
	} else {
		_updateTimer.stop();
		_playTimer.stop();
	}
	emit playing(_playTimer.isActive());
}

void VideoController::videoLoaded()
{
	qDebug() << "video loaded, loading frames";
	requestNewFrames(1);
}


void VideoController::updateVideo()
{
	float slope = _currentRlv.slopeForDistance(_currentDistance);
	emit slopeChanged(slope);
	emit altitudeChanged(_currentRlv.altitudeForDistance(_currentDistance));
}

void VideoController::displayFrame()
{
	updateDistance();
	quint32 frameToShow = _currentRlv.frameForDistance(_currentDistance);
	Frame frame;

	if (frameToShow == _currentFrameNumber)
		return; // no need to display again.
	if (frameToShow < _currentFrameNumber && _currentFrameNumber != UNKNOWN_FRAME_NR) {
		qDebug() << "frame to show" << frameToShow << "current" << _currentFrameNumber;
		return; // wait until playing catches up.
	}

	if (_imageQueue.empty()) {
		qDebug() << "image queue empty, doing nothing";
		return;
	}

	if (_currentFrameNumber == UNKNOWN_FRAME_NR) {
		qDebug() << "current frame is null, taking first one";
		frame = takeFrame();
		_currentFrameNumber = frame.first;
		_videoWidget->displayFrame(frame.first, frame.second);
		return;
	}

	bool displayed = false;
	while(!displayed && _currentFrameNumber != UNKNOWN_FRAME_NR && !_imageQueue.empty()) {
		frame = takeFrame();
		_currentFrameNumber = frame.first;
		if (_currentFrameNumber == frameToShow) {
			_videoWidget->displayFrame(frame.first, frame.second);
			displayed = true;
		}
	}
}

void VideoController::framesReady(FrameList frames, quint32 requestId)
{
	if (_newFramesRequested && requestId == _frameRequestId) {
		_imageQueue.append(frames);
		_newFramesRequested = false;
		if (_currentFrameNumber == UNKNOWN_FRAME_NR) {
			displayFrame();
		}
		if (_imageQueue.size() >= NR_FRAMES_BUFFER_LOW)
			emit bufferFull(true);
	}
}

void VideoController::seekFinished()
{
	requestNewFrames(1);
}

void VideoController::updateDistance()
{
	if (!_currentRlv.isValid())
		return;

	// update distance
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
	qint64 elapsed = currentTime - _lastTime;
	_lastTime = currentTime;

	float distanceTravelled = (SPEED * elapsed) * 0.001;

	setDistance(_currentDistance + distanceTravelled);
}

void VideoController::setDistance(float distance)
{
	_currentDistance = distance;
	emit distanceChanged(distance);
}

void VideoController::loadVideo(const QString &filename)
{
	QMetaObject::invokeMethod(&_videoDecoder, "openFile",
							  Q_ARG(QString, filename));
}

void VideoController::setPosition(quint32 frameNr)
{
	QMetaObject::invokeMethod(&_videoDecoder, "seekFrame",
							  Q_ARG(quint32, frameNr));
}

void VideoController::reset()
{
	play(false);
	emit bufferFull(false);
	_playTimer.stop();
	_updateTimer.stop();
	_currentFrameNumber = UNKNOWN_FRAME_NR;
	_imageQueue.clear();
	_newFramesRequested = false;
	_frameRequestId = 0;
}

Frame VideoController::takeFrame()
{
	if (!_newFramesRequested && _imageQueue.size() <= NR_FRAMES_BUFFER_LOW) {
		requestNewFrames(NR_FRAMES_PER_REQUEST);
	}

	if (_imageQueue.empty())
		return qMakePair(UNKNOWN_FRAME_NR, QImage());
	else
		return _imageQueue.dequeue();
}

void VideoController::requestNewFrames(quint32 numberOfFrames)
{
	_newFramesRequested = true;
	_frameRequestId++;
	QMetaObject::invokeMethod(&_videoDecoder, "loadFrames", Q_ARG(quint32, numberOfFrames), Q_ARG(quint32, _frameRequestId));
}
