#include "videoplayer.h"
#include <QtCore/QtDebug>
#include <QtCore/QThread>

VideoPlayer::VideoPlayer(QGLWidget *paintWidget, QObject *parent) :
    QObject(parent), _pipeline(nullptr), _videoSink(nullptr), _pipelineBus(nullptr), _busTimer(new QTimer(this)), _loadState(NONE), _currentFrameNumber(0u)
{
    setUpVideoSink(paintWidget);
    _busTimer->setInterval(50);
    connect(_busTimer, &QTimer::timeout, this, &VideoPlayer::pollBus);
}

VideoPlayer::~VideoPlayer()
{
    _busTimer->stop();
    if (_pipeline) {
        cleanupCurrentPipeline();
    } else {
        if (_videoSink) {
            gst_element_set_state(_videoSink, GST_STATE_NULL);
            g_object_unref(_videoSink);
        }
    }
}

void VideoPlayer::pollBus()
{
    if (_pipelineBus) {
        GstMessage* message;
        g_object_ref(_pipelineBus);

        while((message = gst_bus_pop(_pipelineBus))) {
            onBusMessage(_pipelineBus, message, this);
        }

        g_object_unref(_pipelineBus);
    }
}

bool VideoPlayer::isReadyToPlay()
{
    return (_loadState == DONE);
}

void VideoPlayer::stop()
{
    if (_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
    }
}

void VideoPlayer::stepToFrame(quint32 frameNumber)
{
    if (_loadState == DONE) {
        qint32 stepSize = frameNumber - _currentFrameNumber;
        if (stepSize > 0) {
            gst_element_send_event(_videoSink, gst_event_new_step(GST_FORMAT_BUFFERS, stepSize, 1.0, true, false));
        }
        _currentFrameNumber = frameNumber;
    } else {
        qDebug() << "stepping when video not ready. Ignoring.";
    }
}

void VideoPlayer::cleanupCurrentPipeline()
{
    if (_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        g_object_unref(_pipeline);
        _pipeline = nullptr;
    }
}

void VideoPlayer::createPipeline()
{
    _pipeline = gst_element_factory_make("playbin", "playbin");

    if (_pipeline) {
        g_object_set(_pipeline, "video-sink", _videoSink, NULL);
        _pipelineBus = gst_element_get_bus(_pipeline);
        _busTimer->start();
    } else {
        qWarning() << "Failed to create the pipeline";
    }
}

void VideoPlayer::loadVideo(QString uri)
{
    cleanupCurrentPipeline();
    _busTimer->stop();

    createPipeline();

    qDebug() << "setting uri";
    if (_pipeline) {
        qDebug() << "really setting uri" << uri;
        g_object_set(_pipeline, "uri", uri.toStdString().c_str(), NULL);
    }
    gst_element_set_state(_pipeline, GST_STATE_PAUSED);

    _loadState = VIDEO_LOADING;
}

void VideoPlayer::handleAsyncDone()
{
    if (_loadState == VIDEO_LOADING) {
        qDebug() << "video loading done";
        gint64 nanoSeconds;
        gst_element_query_duration(_pipeline, GST_FORMAT_TIME, &nanoSeconds);
        _loadState = VIDEO_LOADED;
        qDebug() << "video length" << nanoSeconds;
        emit videoLoaded(nanoSeconds);
        _currentFrameNumber = 0u;
    } else if (_loadState == SEEKING) {
        qDebug() << "seek done";
        _loadState = DONE;
        emit seekDone();
    }
}

bool VideoPlayer::seekToFrame(quint32 frameNumber, float frameRate)
{
    if (_loadState == VIDEO_LOADED || _loadState == DONE) {
        if (frameNumber > _currentFrameNumber && frameNumber - _currentFrameNumber < 100) {
            stepToFrame(frameNumber);
            emit seekDone();
        } else {
            gst_element_set_state(_pipeline, GST_STATE_PAUSED);
            float seconds = frameNumber / frameRate;
            qDebug() << "need to seek to seconds: " << seconds;
            quint64 milliseconds = static_cast<quint64>(seconds * 1000);
            gst_element_seek_simple(_pipeline, GST_FORMAT_TIME, static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), milliseconds * GST_MSECOND);
            _currentFrameNumber = frameNumber;
            _loadState = SEEKING;
        }
        return true;
    }
    return false;
}

void VideoPlayer::displayCurrentFrame(QPainter *painter, QRectF rect)
{
    qDebug() << "display: current thread =" << QThread::currentThreadId();
    g_signal_emit_by_name(_videoSink, "paint", painter, rect.x(), rect.y(), rect.width(), rect.height(), NULL);
}

void VideoPlayer::handleError(GstMessage *msg)
{
    GError *err;
    gchar *debug;

    gst_message_parse_error (msg, &err, &debug);
    QString errorString(err->message);
    qDebug() << "error" << errorString;
    g_error_free(err);
    g_free(debug);
    stop();
}

void VideoPlayer::onBusMessage(GstBus*, GstMessage *msg, VideoPlayer *context)
{
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS: //End of stream. We reached the end of the file.
        qDebug() << "eos";
        context->stop();
        break;
    case GST_MESSAGE_ERROR:
    {
        context->handleError(msg);
    }
        break;
    case GST_MESSAGE_ASYNC_DONE:
        context->handleAsyncDone();
        break;
    default:
        //        qDebug() << GST_MESSAGE_TYPE_NAME(msg);
        break;
    }
}

void VideoPlayer::onVideoUpdate(GObject *, guint, VideoPlayer *context)
{
    qDebug() << "update: current thread =" << QThread::currentThreadId();
    context->sendVideoUpdated();
}

void VideoPlayer::sendVideoUpdated()
{
    emit updateVideo();
}

/*!
 * setup the video sink. This tries to create a gstreamer "qt5glvideosink".
 * If this succeeds, it will handle the OpenGL context to this element, so
 * it can be used to draw the video.
 *
 * NewVideoWidget::onVideoUpdate() will be called when there is something to draw.
 */
void VideoPlayer::setUpVideoSink(QGLWidget* glWidget)
{
    qDebug() << "setting up video sink";
    _videoSink = gst_element_factory_make("qt5glvideosink", "qt5glvideosink");
    if (_videoSink) {
        glWidget->makeCurrent();
        g_object_set(_videoSink, "glcontext", QGLContext::currentContext(), NULL);
        glWidget->doneCurrent();
        gst_element_set_state(_videoSink, GST_STATE_READY);

        qDebug() << "attaching signal";
        g_signal_connect(_videoSink, "update", G_CALLBACK(VideoPlayer::onVideoUpdate), this);
    } else {
        qWarning("unable to create video sink. Not able to display video.");
    }
}
