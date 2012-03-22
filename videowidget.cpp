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
    _videoDecoder->doWithImage(*this);
}

void VideoWidget::resizeGL(int w, int h)
{
    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glOrtho(0, w,0,h,-1,1);
    glMatrixMode (GL_MODELVIEW);

    QMetaObject::invokeMethod(_videoDecoder, "targetSizeChanged",
			      Q_ARG(int, w), Q_ARG(int, h));
}

/*! This method should only be called from the video decoder. */
void VideoWidget::handleImage(const QImage &image)
{
    // only paint if image size is equal to widget size.
    if (image.width() == width() || image.height() == height()) {
	_glImage = convertToGLFormat(image);
	glDrawPixels(_glImage.width(), _glImage.height(), GL_RGBA, GL_UNSIGNED_BYTE, _glImage.bits());
    }
}
