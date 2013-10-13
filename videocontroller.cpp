#include "videocontroller.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>

#include "videowidget.h"

namespace {
const quint32 NR_FRAMES_PER_REQUEST = 11;
const int NR_FRAMES_BUFFER_LOW = 10;
const int FRAME_INTERVAL = 1000/30;
}

VideoController::VideoController(Cyclist &cyclist, VideoWidget* videoWidget, QObject *parent) :
	QObject(parent),
	_cyclist(cyclist),
	_videoDecoder(new VideoDecoder(this)),
	_videoWidget(videoWidget),
	_lastFrameRateSample(QDateTime::currentDateTime()),
	_currentFrameRate(0u)
{
	// set up timers
	_playTimer.setInterval(FRAME_INTERVAL);
	connect(&_playTimer, SIGNAL(timeout()), SLOT(playNextFrame()));

	// set up video decoder
	connect(_videoDecoder, SIGNAL(videoLoaded()), SLOT(videoLoaded()));
}

VideoController::~VideoController()
{
	delete _videoDecoder;
}

bool VideoController::isBufferFull()
{
	return _videoWidget->buffersFull();
}

void VideoController::offerFrame(Frame& frame)
{
	loadFrame(frame);
	emit bufferFull(true);
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
		qDebug() << "framerate = " << _currentFrameRate;
		emit(currentFrameRate(_currentFrameRate));
	}

	quint32 frameToShow = _currentRlv.frameForDistance(_cyclist.distance());
	displayFrame(frameToShow);
}

void VideoController::displayFrame(quint32 frameToShow)
{
	_videoWidget->displayNextFrame(frameToShow);
	_framesThisSecond += frameToShow - _currentFrameNumber;
	_currentFrameNumber = frameToShow;
	fillFrameBuffers();
}

void VideoController::seekFinished(Frame& frame)
{
	_currentFrameNumber = frame.frameNr;
	loadFrame(frame);
	displayFrame(frame.frameNr);
}

void VideoController::loadFrame(Frame &frame)
{
	_loadedFrameNumber = frame.frameNr;
	_videoWidget->loadFrame(frame);
}

void VideoController::loadVideo(const QString &filename)
{
	_videoDecoder->openFile(filename);
}

void VideoController::setPosition(quint32 frameNr)
{
	_videoDecoder->seekFrame(frameNr);
}

void VideoController::reset()
{
	qDebug() << "reset";
	_videoWidget->clearOpenGLBuffers();
	play(false);
	emit bufferFull(false);
	_playTimer.stop();
	_currentFrameNumber = UNKNOWN_FRAME_NR;
	_loadedFrameNumber = 0;
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

void VideoController::fillFrameBuffers()
{
	int skip = determineFramesToSkip();
	while (!_videoWidget->buffersFull()) {
		_videoDecoder->loadFrames(skip);
	}
}
