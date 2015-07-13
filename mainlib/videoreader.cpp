#include "videoreader.h"

#include <cstring>

#include <QtCore/QCoreApplication>
#include <QtCore/QSize>
#include <QtCore/QtDebug>
#include <QtGui/QImage>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "libswscale/swscale.h"
}

#include "reallifevideo.h"

namespace {
const int ERROR_STR_BUF_SIZE = 128;
QEvent::Type CreateImageForFrameEventType = static_cast<QEvent::Type>(QEvent::User + 102);

class CreateImageForFrameEvent: public QEvent
{
public:
    CreateImageForFrameEvent(const RealLifeVideo& rlv, qreal distance):
        QEvent(CreateImageForFrameEventType), _rlv(rlv), _distance(distance)
    {
        // empty
    }

    RealLifeVideo _rlv;
    qreal _distance;
};
}

VideoReader::VideoReader(QObject *parent) :
    QObject(parent), _initialized(false), _codec(nullptr), _codecContext(nullptr),
    _formatContext(nullptr), _frameYuv(nullptr), _frameRgb(nullptr)
{
    // empty
}

VideoReader::~VideoReader()
{
    qDebug() << "closing Videoreader";
    close();
}

void VideoReader::createImageForFrame(const RealLifeVideo& rlv, const qreal distance)
{
    QCoreApplication::postEvent(this, new CreateImageForFrameEvent(rlv, distance));
}

void VideoReader::openVideoFile(const QString &videoFilename)
{
    initialize();
    if (_formatContext && QString(_formatContext->filename) == videoFilename) {
        return;
    }
    close();
    int errorNr = avformat_open_input(&_formatContext, videoFilename.toStdString().c_str(),
                                                                              NULL, NULL);
    if (errorNr != 0) {
        printError(errorNr, QString("Unable to open %1").arg(videoFilename));
    }
    errorNr = avformat_find_stream_info(_formatContext, NULL);
    if (errorNr < 0) {
        printError(errorNr, QString("Unable to find video stream"));
    }
    av_dump_format(_formatContext, 0, videoFilename.toStdString().c_str(), 0);

    _currentVideoStream = findVideoStream(_formatContext);
    if (_currentVideoStream < 0) {
        printError("Unable to find video stream");
    }

    _codecContext = _formatContext->streams[_currentVideoStream]->codec;
    if (!_codecContext) {
        printError("Unable to open codec context");
    }

    _codec = avcodec_find_decoder(_codecContext->codec_id);
    if (!_codec) {
        printError("Unable to find codec");
    }

    errorNr = avcodec_open2(_codecContext, _codec, NULL);
    if (errorNr < 0) {
        printError(errorNr, "Unable to open codec");
    }

    _frameYuv.reset(new AVFrameWrapper);
    _frameRgb.reset(new AVFrameWrapper);
    int numBytes= avpicture_get_size(PIX_FMT_RGB24,
          _codecContext->width, _codecContext->height);
    _imageBuffer.resize(numBytes);
    avpicture_fill(_frameRgb->asPicture(), reinterpret_cast<uint8_t*>(_imageBuffer.data()), PIX_FMT_RGB24,
                   _codecContext->width, _codecContext->height);

    _swsContext = sws_getContext(_codecContext->width, _codecContext->height, AV_PIX_FMT_YUV420P,
                                 _codecContext->width, _codecContext->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR,
                                 nullptr, nullptr, nullptr);

    emit videoOpened(videoFilename, QSize(_codecContext->width, _codecContext->height));
}

void VideoReader::performSeek(qint64 targetFrameNumber)
{
    qDebug() << "seeking to" << targetFrameNumber;
    av_seek_frame(_formatContext, _currentVideoStream, targetFrameNumber,
                  AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codecContext);
}

qint64 VideoReader::loadNextFrame()
{
    AVPacket packet;
    int frameFinished = 0;
    while (!frameFinished) {
        if (av_read_frame(_formatContext, &packet) < 0) {
            qDebug() << "end of file reached";
            return -1;
        }
        if (packet.stream_index == _currentVideoStream) {
            avcodec_decode_video2(_codecContext, _frameYuv->frame,
                                  &frameFinished, &packet);
        }
    }
    qint64 currentFrameNumber = packet.dts;
    av_free_packet(&packet);
    qDebug() << QString("frame read, frame nr #%1").arg(packet.dts);
    return currentFrameNumber;
}

void VideoReader::createImageForFrameNumber(RealLifeVideo& rlv, const qreal distance)
{
    // the first few frames are sometimes black, so when requested to take a "screenshot" of the first frames, just
    // skip to a few frames after the start.
    qint64 frameNumber = qMax(20, static_cast<int>(rlv.frameForDistance(distance)));
    qDebug() << "creating image for" << rlv.name() << "distance" << distance << "frame nr" << frameNumber;
    performSeek(frameNumber);
    loadFramesUntilTargetFrame(frameNumber);
    emit newFrameReady(rlv, distance, createImage());
}

bool VideoReader::event(QEvent *event)
{
    if (event->type() == CreateImageForFrameEventType) {
        CreateImageForFrameEvent* createImageForFrameEvent = dynamic_cast<CreateImageForFrameEvent*>(event);

        RealLifeVideo& rlv = createImageForFrameEvent->_rlv;
        const qreal distance = createImageForFrameEvent->_distance;
        qDebug() << "creating thumbnail for rlv" << rlv.name();
        openVideoFile(rlv.videoInformation().videoFilename());
        AVStream* videoStream = _formatContext->streams[_currentVideoStream];
        qreal frameRate = av_q2d(videoStream->avg_frame_rate);
        qint64 totalNumberOfFrames = frameRate * (_formatContext->duration / AV_TIME_BASE);
        rlv.setNumberOfFrames(totalNumberOfFrames);
        createImageForFrameNumber(rlv, distance);
        return true;
    }
    return QObject::event(event);
}

/**
 * This will load all frame until we encounter the frame with the target frame number.
 * @param targetFrameNumber the number of the target frame
 */
void VideoReader::loadFramesUntilTargetFrame(qint64 targetFrameNumber)
{
    bool seekAgain;
    bool extraSeekDone = false;
    qint64 currentFrameNumber = -1;
    do {
        seekAgain = false;
        currentFrameNumber = loadNextFrame();

        // special case: Some videos, probably those that do not start with a keyframe,
        // will exhibit faulty seek behaviour. After a seek, they end up *after* the
        // targetted frame. In that case, we do another seek to a frame number
        // some frames before the targetted frame to hopefully get a keyframe there.
        // We only do this once to prevent us from causing an endless loop here.
        if (!extraSeekDone && currentFrameNumber > 100 && currentFrameNumber > targetFrameNumber) {
            performSeek(targetFrameNumber - 500);
            seekAgain = true;
            extraSeekDone = true;
        }

    } while (seekAgain || (currentFrameNumber >= 0 && currentFrameNumber < targetFrameNumber));
    emit seekReady(currentFrameNumber);
}

void VideoReader::initialize()
{
    if (!_initialized) {
        avcodec_register_all();
        av_register_all();
        _initialized = true;
    }
}

void VideoReader::close()
{
    if (_codecContext) {
        avcodec_close(_codecContext);
    }
    if (_formatContext) {
        avformat_close_input(&_formatContext);
    }
}

void VideoReader::printError(int errorNumber, const QString &message)
{
    char errorstr[ERROR_STR_BUF_SIZE];
    av_strerror(errorNumber, errorstr, ERROR_STR_BUF_SIZE);

    QString completeMessage = QString("%1 : %2").arg(message).arg(errorstr);
    printError(completeMessage);
}

void VideoReader::printError(const QString &message)
{
    qWarning("%s", qPrintable(message));
    emit error(message);
}

QImage VideoReader::createImage()
{
    sws_scale(_swsContext, _frameYuv->frame->data, _frameYuv->frame->linesize, 0, _codecContext->height, _frameRgb->frame->data,
              _frameRgb->frame->linesize);
    QImage image(_frameYuv->frame->width, _frameYuv->frame->height, QImage::Format_RGB888);
    std::memcpy(image.bits(), _frameRgb->frame->data[0], _imageBuffer.size());
    return image;
}

/**
 * @brief VideoReader::findVideoStream find the video stream amongst a number of streams in a format context
 * @param formatContext the format context for a video file
 * @return the number of the video stream or -1 or no video stream was found.
 */
int VideoReader::findVideoStream(AVFormatContext *formatContext) const
{
    for (quint32 i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            return static_cast<int>(i);
        }
    }
    return -1;
}



AVFrameWrapper::AVFrameWrapper()
{
    frame = av_frame_alloc();
}

AVFrameWrapper::~AVFrameWrapper()
{
    av_frame_free(&frame);
}

AVPicture *AVFrameWrapper::asPicture()
{
    return reinterpret_cast<AVPicture*>(frame);
}
