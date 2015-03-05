/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "videoplayer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QtDebug>
#include <QtCore/QThread>


#include "openglpainter.h"

namespace
{

enum GstPlayFlags {
    GST_PLAY_FLAG_VIDEO         = 0x00000001,
    GST_PLAY_FLAG_AUDIO         = 0x00000002,
    GST_PLAY_FLAG_TEXT          = 0x00000004,
    GST_PLAY_FLAG_VIS           = 0x00000008,
    GST_PLAY_FLAG_SOFT_VOLUME   = 0x00000010,
    GST_PLAY_FLAG_NATIVE_AUDIO  = 0x00000020,
    GST_PLAY_FLAG_NATIVE_VIDEO  = 0x00000040,
    GST_PLAY_FLAG_DOWNLOAD      = 0x00000080,
    GST_PLAY_FLAG_BUFFERING     = 0x000000100,
    GST_PLAY_FLAG_DEINTERLACE   =     0x000000200,
    GST_PLAY_FLAG_SOFT_COLORBALANCE = 0x000000400,
    GST_PLAY_FLAG_FORCE_FILTERS = 0x000000800
};

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
        if (stepSize > 5) {
            qDebug() << "step size is too big:" << stepSize << "current frame number:" << _currentFrameNumber;
        }
        if (stepSize > 0) {
            gst_element_send_event(_pipeline, gst_event_new_step(GST_FORMAT_BUFFERS, stepSize, 1.0, true, false));
        }
        updateCurrentFrameNumber(frameNumber);
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
    GstAppSinkCallbacks callbacks = { handleAppSinkEndOfStream, handleAppSinkNewPreRoll, handleAppSinkNewSample, { nullptr }};
    gst_app_sink_set_callbacks(GST_APP_SINK(_appSink), &callbacks, this, nullptr);

    if (_pipeline) {
        GstPlayFlags flags;
        g_object_get(_playbin, "flags", &flags, nullptr);
        qDebug() << "flags" << flags;
        flags = static_cast<GstPlayFlags>(flags | GST_PLAY_FLAG_VIDEO);
        flags = static_cast<GstPlayFlags>(flags & ~GST_PLAY_FLAG_AUDIO & ~GST_PLAY_FLAG_SOFT_COLORBALANCE & ~GST_PLAY_FLAG_SOFT_VOLUME & ~GST_PLAY_FLAG_TEXT) ;
        qDebug() << "flags" << flags;
        g_object_set(_playbin, "flags", flags, nullptr);

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
    _painter->reset();

    createPipeline();

    if (_pipeline) {
        qDebug() << "really setting uri" << uri;
        g_object_set(_playbin, "uri", uri.toStdString().c_str(), NULL);
    }
    gst_element_set_state(_pipeline, GST_STATE_PAUSED);

    updateLoadState(VIDEO_LOADING);
}

void VideoPlayer::handleAsyncDone()
{
    if (_loadState == VIDEO_LOADING) {
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_playbin), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline.dot");
        qDebug()  << "video loading done";
        gint64 nanoSeconds;
        gst_element_query_duration(_pipeline, GST_FORMAT_TIME, &nanoSeconds);
        updateLoadState(VIDEO_LOADED);
        qDebug() << "video length" << nanoSeconds;
        updateCurrentFrameNumber(0u);
        emit videoLoaded(nanoSeconds);
    } else if (_loadState == SEEKING) {
        qDebug() << "seek done";
        updateLoadState(DONE);
        emit seekDone();
    }
}

bool VideoPlayer::seekToFrame(quint32 frameNumber, float frameRate)
{
    if (_loadState == VIDEO_LOADED || _loadState == DONE) {
        if (frameNumber > _currentFrameNumber && frameNumber - _currentFrameNumber < 100) {
            qDebug() << "will seek" << frameNumber - _currentFrameNumber << "frames to start";
            updateLoadState(DONE);
            stepToFrame(frameNumber);
            emit seekDone();
        } else {
            gst_element_set_state(_pipeline, GST_STATE_PAUSED);
            float seconds = frameNumber / frameRate;
            qDebug() << "need to seek to seconds: " << seconds;
            quint64 milliseconds = static_cast<quint64>(seconds * 1000);
            gst_element_seek_simple(_pipeline, GST_FORMAT_TIME, static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), milliseconds * GST_MSECOND);
            updateCurrentFrameNumber(frameNumber);

            updateLoadState(SEEKING);
        }
        return true;
    }
    return false;
}

void VideoPlayer::displayCurrentFrame(QPainter *painter, QRectF rect, Qt::AspectRatioMode aspectRatioMode)
{
    _painter->paint(painter, rect, aspectRatioMode);
}


bool VideoPlayer::event(QEvent *event)
{
    if (event->type() == NEW_SAMPLE_EVENT_TYPE) {
        _nrOfFramesWaiting += 1;
        _painter->setCurrentSample(gst_app_sink_pull_preroll(GST_APP_SINK(_appSink)));

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

void VideoPlayer::updateCurrentFrameNumber(const quint32 frameNumber)
{
    _currentFrameNumber = frameNumber;
}

void VideoPlayer::updateLoadState(const VideoPlayer::LoadState loadState)
{
    _loadState = loadState;
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

