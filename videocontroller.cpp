#include "videocontroller.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>

namespace {
const quint32 NR_FRAMES_PER_REQUEST = 25;
const int NR_FRAMES_BUFFER_LOW = 40;
const int FRAME_INTERVAL = 1000/30;
}

VideoController::VideoController(Cyclist &cyclist, VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_cyclist(cyclist),
	_videoDecoder(new VideoDecoder),
	_videoWidget(videoWidget),
	_requestBusy(false),
	_lastFrameRateSample(QDateTime::currentDateTime()),
	_currentFrameRate(0u)
{
	_decoderThread.start();
	_videoDecoder->moveToThread(&_decoderThread);

	// set up timers
	_playTimer.setInterval(FRAME_INTERVAL);
	connect(&_playTimer, SIGNAL(timeout()), SLOT(playNextFrame()));

	// set up video decoder
	connect(_videoDecoder, SIGNAL(videoLoaded()), SLOT(videoLoaded()));
	connect(_videoDecoder, SIGNAL(framesReady(FrameList)), SLOT(framesReady(FrameList)));
	connect(_videoDecoder, SIGNAL(seekFinished(Frame)), SLOT(seekFinished(Frame)));
	connect(_videoDecoder, SIGNAL(destroyed()), &_decoderThread, SLOT(quit()));
}

VideoController::~VideoController()
{
	qDebug() << "destroying video controller" << QThread::currentThreadId();
	_videoDecoder->deleteLater();
	while (_decoderThread.isRunning())
		QCoreApplication::processEvents();
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
}

void VideoController::courseSelected(int courseNr)
{
	reset();

	if (courseNr == -1) {
		return;
	}

	if (!_currentRlv.isValid())
		return;

	quint32 frame = _currentRlv.frameForDistance(_cyclist.distance());

	setPosition(frame);
}

void VideoController::play(bool doPlay)
{
	if (doPlay) {
		_playTimer.start();
	} else {
		_playTimer.stop();
	}
	emit playing(_playTimer.isActive());
}

void VideoController::videoLoaded()
{
	qDebug() << "video loaded";
}

void VideoController::playNextFrame()
{
	// keep track of frame rate
	QDateTime now = QDateTime::currentDateTime();
	if (_lastFrameRateSample.addSecs(1) < now) {
		_currentFrameRate = _framesThisSecond;
		_framesThisSecond = 0;
		_lastFrameRateSample = now;
		emit(currentFrameRate(_currentFrameRate));
	}

	quint32 frameToShow = _currentRlv.frameForDistance(_cyclist.distance());
	displayFrame(frameToShow);
}

void VideoController::displayFrame(quint32 frameToShow)
{
	if (frameToShow == _currentFrameNumber)
		return; // no need to display again.
	if (frameToShow < _currentFrameNumber && _currentFrameNumber != UNKNOWN_FRAME_NR) {
		qDebug() << "frame to show" << frameToShow << "current" << _currentFrameNumber;
		return; // wait until playing catches up.
	}

	// if we've loaded a frame that's higher than the currently shown frame, let the widget display it.
	if (_loadedFrameNumber > _currentFrameNumber) {
		_videoWidget->displayNextFrame();
		_currentFrameNumber = _loadedFrameNumber;
	}

	// find new frame to load into widget, but not display yet.
	if (_imageQueue.empty()) {
		qDebug() << "image queue empty, doing nothing";
		return;
	}

	Frame frame;
	if (_currentFrameNumber == UNKNOWN_FRAME_NR) {
		qDebug() << "current frame is null, taking first one";
		frame = takeFrame();
		_currentFrameNumber = frame.frameNr;
		_loadedFrameNumber = frame.frameNr;
		_videoWidget->loadFrame(frame);
		_videoWidget->displayNextFrame();
	}

	/* load next frame, so widget can upload it to graphics card */
	bool loaded = false;
	while(!loaded && _currentFrameNumber != UNKNOWN_FRAME_NR && !_imageQueue.empty()) {
		frame = takeFrame();
		_framesThisSecond += frame.frameNr - _currentFrameNumber;
		_loadedFrameNumber = frame.frameNr;
		if (_loadedFrameNumber > frameToShow) {
			_videoWidget->loadFrame(frame);
			loaded = true;
		}
	}
}

void VideoController::framesReady(FrameList frames)
{
	_requestBusy = false;
	if (_currentFrameNumber == UNKNOWN_FRAME_NR) {
		_currentFrameNumber = 1;
	}

	if (!frames.isEmpty() && frames.first().frameNr >= _currentFrameNumber) {
		_imageQueue += frames;

		if (_imageQueue.size() >= NR_FRAMES_BUFFER_LOW)
			emit bufferFull(true);
	}

	// check if we need some new frames ASAP
	if (_imageQueue.size() <= NR_FRAMES_BUFFER_LOW)
		requestNewFrames(NR_FRAMES_PER_REQUEST);
}

void VideoController::seekFinished(Frame frame)
{
	_imageQueue.clear();
	_imageQueue.append(frame);
	displayFrame(frame.frameNr);
}

void VideoController::loadVideo(const QString &filename)
{
	QMetaObject::invokeMethod(_videoDecoder, "openFile",
							  Q_ARG(QString, filename));
}

void VideoController::setPosition(quint32 frameNr)
{
	QMetaObject::invokeMethod(_videoDecoder, "seekFrame",
							  Q_ARG(quint32, frameNr));
}

void VideoController::reset()
{
	_videoWidget->clearOpenGLBuffers();
	play(false);
	emit bufferFull(false);
	_playTimer.stop();
	_currentFrameNumber = UNKNOWN_FRAME_NR;
	_imageQueue.clear();
	_requestBusy = false;
}

Frame VideoController::takeFrame()
{
	if (!_requestBusy && _imageQueue.size() <= NR_FRAMES_BUFFER_LOW) {
		requestNewFrames(NR_FRAMES_PER_REQUEST);
	}

	if (_imageQueue.empty()) {
		Frame frame;
		frame.frameNr = UNKNOWN_FRAME_NR;
		return frame;
	}

	return _imageQueue.takeFirst();
}

int VideoController::determineFramesToSkip()
{
	if (_currentFrameRate < 30)
		return 0; // skip no frames when (virtual) frame rate below 30 frames/s.
	else if (_currentFrameRate < 40)
		return 1; // skip 1 frame for every decoded frame.
	else
		return 2; // skip 2 frames for every decoded frame
}

void VideoController::requestNewFrames(quint32 numberOfFrames)
{
	_requestBusy = true;

	int skip = determineFramesToSkip();
	QMetaObject::invokeMethod(_videoDecoder, "loadFrames", Q_ARG(quint32, numberOfFrames), Q_ARG(quint32, skip));
}
