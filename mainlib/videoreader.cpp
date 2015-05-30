#include "videoreader.h"

#include <cstring>

#include <QtCore/QCoreApplication>
#include <QtCore/QSize>
#include <QtCore/QtDebug>
#include <QtCore/QTimer>
#include <QtMultimedia/QVideoFrame>

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
    _formatContext(nullptr), _frame(nullptr), _frameRgb(nullptr)
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

    emit videoOpened(videoFilename, QSize(_codecContext->width, _codecContext->height));
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
            avcodec_decode_video2(_codecContext, _frame,
                                  &frameFinished, &packet);
        }
    }
    qint64 currentFrameNumber = packet.dts;
    av_free_packet(&packet);
    qDebug() << QString("frame read, frame nr #%1").arg(packet.dts);
    return currentFrameNumber;
}

void VideoReader::createImageForFrameNumber(const RealLifeVideo& rlv, const qreal distance, int frameNumber)
{
    qDebug() << "creating image for" << rlv.name() << "distance" << distance << "frame nr" << frameNumber;
    av_seek_frame(_formatContext, _currentVideoStream, frameNumber, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_codecContext);

//    QTimer::singleShot(500, this, [this, rlv, distance, frameNumber]() {
        loadFramesUntilTargetFrame(frameNumber);
        emit newFrameReady(rlv, distance, createImage());
//    });

}

bool VideoReader::event(QEvent *event)
{
    if (event->type() == CreateImageForFrameEventType) {
        CreateImageForFrameEvent* createImageForFrameEvent = dynamic_cast<CreateImageForFrameEvent*>(event);
        RealLifeVideo& rlv = createImageForFrameEvent->_rlv;
        const qreal distance = createImageForFrameEvent->_distance;

        openVideoFile(rlv.videoInformation().videoFilename());
        rlv.setDuration(_formatContext->duration);
        createImageForFrameNumber(rlv, distance, rlv.frameForDistance(distance));
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
    qint64 currentFrameNumber = -1;
    do {
        currentFrameNumber = loadNextFrame();
    } while (currentFrameNumber >= 0 && currentFrameNumber < targetFrameNumber);
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
    sws_scale(_swsContext, _frame->data, _frame->linesize, 0, _codecContext->height, _frameRgb->data, _frameRgb->linesize);
    QImage image(_frame->width, _frame->height, QImage::Format_RGB888);
    std::memcpy(image.bits(), _frameRgb->data[0], _imageBuffer.size());
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

