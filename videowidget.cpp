#include "videowidget.h"

#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QThread>

#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QGLWidget(parent), _videoDecoder(new VideoDecoder),
	_playDelayTimer(new QTimer(this)),
	_playTimer(new QTimer(this)),
	_playThread(new QThread(this))
{
    _playDelayTimer->setSingleShot(true);
    _playDelayTimer->setInterval(250);
    connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));
	_playTimer->setInterval(40);
	connect(_playTimer, SIGNAL(timeout()), _videoDecoder, SLOT(nextFrame()));

	_playThread->start();
	_videoDecoder->moveToThread(_playThread);

	connect(_videoDecoder, SIGNAL(frameReady(quint32)), SLOT(frameReady(quint32)));
}

VideoWidget::~VideoWidget()
{
	_playThread->quit();
	_playThread->wait();

	delete _videoDecoder;
}

void VideoWidget::playVideo()
{
	QMetaObject::invokeMethod(_videoDecoder, "openFile",
							  Q_ARG(QString, _currentRealLiveVideo.videoInformation().videoFilename()));
}


void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
	if (_playTimer->isActive())
		_playTimer->stop();
    if (_playDelayTimer->isActive())
        _playDelayTimer->stop();
    _currentRealLiveVideo = rlv;

    _playDelayTimer->start();
}

void VideoWidget::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		_playTimer->stop();
		return;
	}

	if (!_currentRealLiveVideo.isValid())
		return;

	const Course& course = _currentRealLiveVideo.courses()[courseNr];
	course.start();
	_playTimer->start();
}

void VideoWidget::frameReady(quint32 frameNr)
{
	qDebug() << "frame is ready " << frameNr;
	repaint();
}

void VideoWidget::paintEvent(QPaintEvent *)
{
	_videoDecoder->lock();
	qDebug() << "painting";
	const QImage& image = _videoDecoder->currentImage();

	QPainter p(this);
	//Set the painter to use a smooth scaling algorithm.
	//p.setRenderHint(QPainter::SmoothPixmapTransform, 1);


	p.drawImage(this->rect(), image);

	_videoDecoder->unlock();
}
