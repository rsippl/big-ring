#include "videoreader2.h"

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
const int SEEK_TIMEOUT = 10; // ms
QEvent::Type CreateImageForFrameEventType = static_cast<QEvent::Type>(QEvent::User + 102);
QEvent::Type OpenVideoFileEventType = static_cast<QEvent::Type>(QEvent::User + 103);
QEvent::Type CopyNextFrameEventType = static_cast<QEvent::Type>(QEvent::User + 104);

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

class OpenVideoFileEvent: public QEvent
{
public:
    OpenVideoFileEvent(const QString& videoFilename):
        QEvent(OpenVideoFileEventType), _videoFilename(videoFilename)
    {
        // empty
    }

    QString _videoFilename;
};

class CopyNextFrameEvent: public QEvent
{
public:
    CopyNextFrameEvent(const FrameBuffer& frameBuffer):
        QEvent(CopyNextFrameEventType), _frameBuffer(frameBuffer)
    {
        // empty
    }

    FrameBuffer _frameBuffer;
};
}

VideoReader2::VideoReader2(QObject *parent) :
    QObject(parent), _initialized(false), _codec(nullptr), _codecContext(nullptr),
    _formatContext(nullptr), _frame(nullptr), _frameRgb(nullptr)
{
    // empty
}

VideoReader2::~VideoReader2()
{
    qDebug() << "closing VideoReader2";
    close();
}

void VideoReader2::createImageForFrame(const RealLifeVideo& rlv, const qreal distance)
{
    QCoreApplication::postEvent(this, new CreateImageForFrameEvent(rlv, distance));
}

void VideoReader2::copyNextFrame(const FrameBuffer &buffer)
{
    QCoreApplication::postEvent(this, new CopyNextFrameEvent(buffer));
}

void VideoReader2::openVideoFile(const QString &videoFilename)
{
    QCoreApplication::postEvent(this, new OpenVideoFileEvent(videoFilename));
}

void VideoReader2::openVideoFileInternal(const QString &videoFilename)
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

    _frame = av_frame_alloc();
    _frameRgb = av_frame_alloc();
    int numBytes= avpicture_get_size(PIX_FMT_RGB24,
          _codecContext->width, _codecContext->height);
    _imageBuffer.resize(numBytes);
    avpicture_fill((AVPicture*) _frameRgb, reinterpret_cast<uint8_t*>(_imageBuffer.data()), PIX_FMT_RGB24,
                   _codecContext->width, _codecContext->height);

    _swsContext = sws_getContext(_codecContext->width, _codecContext->height, AV_PIX_FMT_YUV420P,
                                 _codecContext->width, _codecContext->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR,
                                 nullptr, nullptr, nullptr);

    AVStream* videoStream = _formatContext->streams[_currentVideoStream];
    qreal frameRate = av_q2d(videoStream->avg_frame_rate);
    qint64 totalNumberOfFrames = frameRate * (_formatContext->duration / AV_TIME_BASE);
    emit videoOpened(videoFilename, QSize(_codecContext->width, _codecContext->height), totalNumberOfFrames);
}

void VideoReader2::copyNextFrameInternal(const FrameBuffer &buffer)
{
    if (buffer.ptr) {
        quint8* bufferPointer = reinterpret_cast<quint8*>(buffer.ptr);
        qDebug() << "buffer = " << bufferPointer;
        quint32 offset = 0;
        quint32 ysize = _frame->linesize[0] * _frame->height;
        std::memcpy(bufferPointer, _frame->data[0], ysize);
        offset += ysize;
        quint32 usize = _frame->linesize[1] * _frame->height / 2;
        std::memcpy(bufferPointer + offset, _frame->data[1], usize);
        offset += usize;
        quint32 vsize = usize;
        std::memcpy(bufferPointer + offset, _frame->data[2], vsize);

        qDebug() << "copied" << offset + vsize << "bytes to buffer";

        emit frameCopied(buffer.index, buffer.frameSize);
        loadNextFrame();
    }
}

void VideoReader2::performSeek(qint64 targetFrameNumber)
{
    qDebug() << "seeking to" << targetFrameNumber;
    av_seek_frame(_formatContext, _currentVideoStream, targetFrameNumber,
                  AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codecContext);
}

qint64 VideoReader2::loadNextFrame()
{
    AVPacket packet;
    int frameFinished = 0;
    while (!frameFinished) {
        if (av_read_frame(_formatContext, &packet) < 0) {
            qDebug() << "end of file reached";
            return -1;
        }
        if (packet.stream_index == _currentVideoStream) {
            avcodec_decode_video2(_codecContext, _frame,
                                  &frameFinished, &packet);
        }
    }
    qint64 currentFrameNumber = packet.dts;
    av_free_packet(&packet);
    qDebug() << QString("frame read, frame nr #%1").arg(packet.dts);
    return currentFrameNumber;
}

void VideoReader2::createImageForFrameNumber(RealLifeVideo& rlv, const qreal distance)
{
    // the first few frames are sometimes black, so when requested to take a "screenshot" of the first frames, just
    // skip to a few frames after the start.
    qint64 frameNumber = qMax(20, static_cast<int>(rlv.frameForDistance(distance)));
    qDebug() << "creating image for" << rlv.name() << "distance" << distance << "frame nr" << frameNumber;
    performSeek(frameNumber);
    loadFramesUntilTargetFrame(frameNumber);
    emit newFrameReady(rlv, distance, createImage());
}

bool VideoReader2::event(QEvent *event)
{
    if (event->type() == CreateImageForFrameEventType) {
        CreateImageForFrameEvent* createImageForFrameEvent = dynamic_cast<CreateImageForFrameEvent*>(event);

        RealLifeVideo& rlv = createImageForFrameEvent->_rlv;
        const qreal distance = createImageForFrameEvent->_distance;
        qDebug() << "creating thumbnail for rlv" << rlv.name();
        openVideoFile(rlv.videoInformation().videoFilename());
        rlv.setDuration(_formatContext->duration);
        createImageForFrameNumber(rlv, distance);
        return true;
    } else if (event->type() == OpenVideoFileEventType) {
        OpenVideoFileEvent* openVideoFileEvent = dynamic_cast<OpenVideoFileEvent*>(event);
        openVideoFileInternal(openVideoFileEvent->_videoFilename);
        loadNextFrame();
        return true;
    } else if (event->type() == CopyNextFrameEventType) {
        CopyNextFrameEvent* copyNextFrameEvent = dynamic_cast<CopyNextFrameEvent*>(event);
        copyNextFrameInternal(copyNextFrameEvent->_frameBuffer);
    }
    return QObject::event(event);
}

/**
 * This will load all frame until we encounter the frame with the target frame number.
 * @param targetFrameNumber the number of the target frame
 */
void VideoReader2::loadFramesUntilTargetFrame(qint64 targetFrameNumber)
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

void VideoReader2::initialize()
{
    if (!_initialized) {
        avcodec_register_all();
        av_register_all();
        _initialized = true;
    }
}

void VideoReader2::close()
{
    if (_frameRgb) {
        av_frame_free(&_frameRgb);
    }
    if (_frame) {
        av_frame_free(&_frameRgb);
    }
    if (_codecContext) {
        avcodec_close(_codecContext);
    }
    if (_formatContext) {
        avformat_close_input(&_formatContext);
    }
}

void VideoReader2::printError(int errorNumber, const QString &message)
{
    char errorstr[ERROR_STR_BUF_SIZE];
    av_strerror(errorNumber, errorstr, ERROR_STR_BUF_SIZE);

    QString completeMessage = QString("%1 : %2").arg(message).arg(errorstr);
    printError(completeMessage);
}

void VideoReader2::printError(const QString &message)
{
    qWarning("%s", qPrintable(message));
    emit error(message);
}

QImage VideoReader2::createImage()
{
    sws_scale(_swsContext, _frame->data, _frame->linesize, 0, _codecContext->height, _frameRgb->data, _frameRgb->linesize);
    QImage image(_frame->width, _frame->height, QImage::Format_RGB888);
    std::memcpy(image.bits(), _frameRgb->data[0], _imageBuffer.size());
    return image;
}

/**
 * @brief VideoReader2::findVideoStream find the video stream amongst a number of streams in a format context
 * @param formatContext the format context for a video file
 * @return the number of the video stream or -1 or no video stream was found.
 */
int VideoReader2::findVideoStream(AVFormatContext *formatContext) const
{
    for (quint32 i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

