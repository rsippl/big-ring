#include "videowidget.h"

#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>
VideoWidget::VideoWidget(QWidget *parent) :
    QGraphicsView(parent), _playTimer(new QTimer(this)), _framesLoaded(0), _currentFrame(0)
{
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    QGLWidget* viewPortWidget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
    _painter = new OpenGLPainter2(viewPortWidget, this);
    setViewport(viewPortWidget);
    setFrameShape(QFrame::NoFrame);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QGraphicsScene* scene = new QGraphicsScene(this);
    setScene(scene);

    setSizeAdjustPolicy(QGraphicsView::AdjustIgnored);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheNone);

    _videoReaderThread = new QThread;
    _videoReader = new VideoReader2;
    _videoReader->moveToThread(_videoReaderThread);
    connect(_videoReaderThread, &QThread::finished, _videoReaderThread, &QThread::deleteLater);
    connect(_videoReaderThread, &QThread::finished, _videoReader, &VideoReader2::deleteLater);
    _videoReaderThread->start();

    connect(_videoReader, &VideoReader2::videoOpened, this, &VideoWidget::setVideoInformation);
    connect(_videoReader, &VideoReader2::seekReady, this, &VideoWidget::setSeekReady);
    connect(_videoReader, &VideoReader2::frameCopied, this, &VideoWidget::setFrameLoaded);
    connect(_painter, &OpenGLPainter2::frameNeeded, this, &VideoWidget::setFrameNeeded);

    QString filename = "/home/ibooij/Videos/rlv/Kaunertal.avi";
    _videoReader->openVideoFile(filename);

    _playTimer->setTimerType(Qt::PreciseTimer);
    _playTimer->setInterval(1000 / 30);
    connect(_playTimer, &QTimer::timeout, this, &VideoWidget::showNextFrame);
}

VideoWidget::~VideoWidget()
{
    _videoReaderThread->quit();
}

void VideoWidget::setVideoInformation(const QString &videoFilename, const QSize &videoSize, const qint64 numberOfFrames)
{
    qDebug() << "video opened" << videoFilename << videoSize << numberOfFrames;
    _painter->setVideoSize(videoSize);
    _videoReader->seekToFrame(110000);
    _videoReader->copyNextFrame(_painter->getNextFrameBuffer());
    _painter->requestNewFrames();
    _time.start();
}

void VideoWidget::showNextFrame()
{
    qDebug() << "time since last frame" << _betweenFramesTime.restart();
    _currentFrame += 2;
    if (_painter->showFrame(_currentFrame)) {
        viewport()->update();
    }
}

void VideoWidget::setSeekReady(qint64 frameNumber)
{
    qDebug() << "seek ready, now at frame number" << frameNumber;
    _currentFrame = frameNumber;
}

void VideoWidget::setFrameLoaded(int index, qint64 frameNumber, const QSize& frameSize)
{
    _framesLoaded += 1;
    qDebug() << "frame loaded" << frameNumber << "showing" << _currentFrame;
    _painter->setFrameLoaded(index, frameNumber, frameSize);

    _playTimer->start();
}

void VideoWidget::setFrameNeeded(const FrameBuffer &frameBuffer)
{
    if (frameBuffer.ptr) {
        _videoReader->copyNextFrame(frameBuffer, 2);
    }
}

void VideoWidget::resizeEvent(QResizeEvent *)
{
    qDebug() << "resize event";
}

/*!
 *
 * \brief Draw background by telling the video sink to update the complete background.
 * \param painter the painter used for drawing.
 */
void VideoWidget::drawBackground(QPainter *painter, const QRectF &)
{
    QPointF topLeft = mapToScene(viewport()->rect().topLeft());
    QPointF bottemRight = mapToScene(viewport()->rect().bottomRight());
    const QRectF r = QRectF(topLeft, bottemRight);

    qDebug() << "painting";
    _painter->paint(painter, r, Qt::KeepAspectRatioByExpanding);
}
