#include "videowidget.h"

#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QThread>
#include <QtOpenGL/qgl.h>
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

    _paintTime.start();
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

void VideoWidget::frameReady(quint32)
{
    repaint();
}

void VideoWidget::paintGL()
{
    _paintTime.restart();

    _videoDecoder->lock();
    const QImage* image = _videoDecoder->currentImage();
    if (!image) {
	_videoDecoder->unlock();
	return;
    }
    qDebug() << "painting image of " << QString("(%1x%2)").arg(image->width()).arg(image->height());
    bool doPaint = false;
    if (image->width() == width() && image->height() == height()) {
	_glImage = convertToGLFormat(*image);
	doPaint = true;
    }
    _videoDecoder->unlock();


    if (doPaint) {
	qDebug() << "painting image of " << QString("(%1x%2)").arg(_glImage.width()).arg(_glImage.height());
	glDrawPixels(_glImage.width(), _glImage.height(), GL_RGBA, GL_UNSIGNED_BYTE, _glImage.bits());
	qDebug() << "painting took " << _paintTime.elapsed() << " ms";
    }
}

void VideoWidget::resizeGL(int w, int h)
{
    qDebug() << "resize called with size " << QString("(%1,%2)").arg(w).arg(h);
    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w,0,h,-1,1);
    glMatrixMode (GL_MODELVIEW);
    QMetaObject::invokeMethod(_videoDecoder, "targetSizeChanged",
			      Q_ARG(int, w), Q_ARG(int, h));
}
