#include "videoplayer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QtDebug>
#include <QtCore/QThread>


#include "openglpainter.h"

namespace
{
const QEvent::Type NEW_SAMPLE_EVENT_TYPE = static_cast<QEvent::Type>(QEvent::User + 100);
class NewSampleEvent: public QEvent
{
public:
    NewSampleEvent() : QEvent(NEW_SAMPLE_EVENT_TYPE) {}
};
}
VideoPlayer::VideoPlayer(QGLWidget *paintWidget, QObject *parent) :
    QObject(parent), _pipeline(nullptr), _pipelineBus(nullptr), _busTimer(new QTimer(this)), _loadState(NONE), _currentFrameNumber(0u),
    _nrOfFramesWaiting(0)
{
    _painter = new OpenGLPainter(paintWidget, this);
    _busTimer->setInterval(50);
    connect(_busTimer, &QTimer::timeout, this, &VideoPlayer::pollBus);
}

VideoPlayer::~VideoPlayer()
{
    _busTimer->stop();
    if (_pipeline) {
        cleanupCurrentPipeline();
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

void VideoPlayer::handleAppSinkEndOfStream(GstAppSink *, gpointer user_data)
{
    Q_UNUSED(user_data);
    qDebug() << "eos";
}

GstFlowReturn VideoPlayer::handleAppSinkNewPreRoll(GstAppSink*, gpointer user_data)
{
    VideoPlayer* player = static_cast<VideoPlayer*>(user_data);
    QCoreApplication::postEvent(player, new NewSampleEvent);
    return GST_FLOW_OK;
}

GstFlowReturn VideoPlayer::handleAppSinkNewSample(GstAppSink*, gpointer user_data)
{
    VideoPlayer* player = static_cast<VideoPlayer*>(user_data);
    QCoreApplication::postEvent(player, new NewSampleEvent);
    return GST_FLOW_OK;
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
            gst_element_send_event(_pipeline, gst_event_new_step(GST_FORMAT_BUFFERS, stepSize, 1.0, true, false));
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
    _pipeline = gst_pipeline_new("myplayer");
    _playbin = gst_element_factory_make("playbin", "playbin");
    _appSink = gst_element_factory_make("appsink", "appsink");

    gst_bin_add(GST_BIN(_pipeline), _playbin);
    GstAppSinkCallbacks callbacks = { handleAppSinkEndOfStream, handleAppSinkNewSample, handleAppSinkNewSample, { nullptr }};
    gst_app_sink_set_callbacks(GST_APP_SINK(_appSink), &callbacks, this, nullptr);

    if (_pipeline) {
        GstCaps* caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", nullptr);
        gst_app_sink_set_caps(GST_APP_SINK(_appSink), caps);
        gst_caps_unref(caps);
        g_object_set(_playbin, "video-sink", _appSink, NULL);
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
        g_object_set(_playbin, "uri", uri.toStdString().c_str(), NULL);
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
    _painter->paint(painter, rect);
}


bool VideoPlayer::event(QEvent *event)
{
    if (event->type() == NEW_SAMPLE_EVENT_TYPE) {
        _nrOfFramesWaiting += 1;
        _painter->setCurrentSample(gst_app_sink_pull_preroll(GST_APP_SINK(_appSink)));

        qDebug() << "New sample received!";

        emit updateVideo();
        return true;
    }
    return QObject::event(event);
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
    context->sendVideoUpdated();
}

void VideoPlayer::sendVideoUpdated()
{
    emit updateVideo();
}

