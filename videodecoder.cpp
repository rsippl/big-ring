#include "videodecoder.h"
extern "C"
{
#include <limits.h>
#include <stdint.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

namespace {
const int ERROR_STR_BUF_SIZE = 128;
}
VideoDecoder::VideoDecoder(QObject *parent) :
    QObject(parent), _formatContext(NULL), _codecContext(NULL),
    _codec(NULL), _frame(NULL), _frameRgb(NULL),
    _bufferSize(0), _frameBuffer(NULL)
{
    initialize();
}

VideoDecoder::~VideoDecoder()
{
    close();
}

void VideoDecoder::initialize()
{
    avcodec_init();
    avcodec_register_all();
    av_register_all();
}

void VideoDecoder::close()
{
    if (_frameBuffer)
        delete[] _frameBuffer;
    _frameBuffer = NULL;
    if (_frameRgb)
        av_free(_frameRgb);
    _frameRgb = NULL;
    if (_frame)
        av_free(_frame);
    _frame = NULL;
    _codec = NULL;
    if (_codecContext)
        avcodec_close(_codecContext);
    _codecContext = NULL;
    if (_formatContext)
        av_close_input_file(_formatContext);
    _formatContext = NULL;
}

bool VideoDecoder::openFile(QString filename)
{
    close();
    int error = avformat_open_input(&_formatContext, filename.toStdString().c_str(),
                                    NULL, NULL);
    if (error != 0) {
        printError(error, QString("Unable to open %1").arg(filename));
        return false;
    }
    error = av_find_stream_info(_formatContext);
    if (error < 0) {
        printError(error, QString("Unable to find video stream"));
        return false;
    }
    av_dump_format(_formatContext, 0, filename.toStdString().c_str(), 0);

    _videoStream = findVideoStream();
    if (_videoStream < 0) {
        qWarning("Unable to find video stream");
        return false;
    }

    _codecContext = _formatContext->streams[_videoStream]->codec;
    if (!_codecContext) {
        qWarning("Unable to open codec context");
        return false;
    }

    _codec = avcodec_find_decoder(_codecContext->codec_id);
    if (!_codec) {
        qWarning("Unable to find codec");
        return false;
    }

    error = avcodec_open2(_codecContext, _codec, NULL);
    if (error < 0) {
        printError(error, "Unable to open codec");
        return false;
    }

    _frame = avcodec_alloc_frame();
    _frameRgb = avcodec_alloc_frame();

    _bufferSize = avpicture_get_size(PIX_FMT_RGB24, _codecContext->width, _codecContext->height);
    _frameBuffer = new quint8[_bufferSize];

    avpicture_fill(reinterpret_cast<AVPicture*>(_frameRgb), _frameBuffer, PIX_FMT_RGB24,
                   _codecContext->width, _codecContext->height);

    return true;
}

int VideoDecoder::findVideoStream()
{
    for (quint32 i = 0; i < _formatContext->nb_streams; ++i)
        if (_formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            return static_cast<int>(i);
    return -1;
}

void VideoDecoder::printError(int errorNr, const QString& message)
{
    char errorstr[ERROR_STR_BUF_SIZE];
    av_strerror(errorNr, errorstr, ERROR_STR_BUF_SIZE);

    QString completeMessage = message + ": %s";
    qWarning(completeMessage.toStdString().c_str(), errorstr);
}
