#include "videodecoder.h"
extern "C"
{
#include <limits.h>
#include <stdint.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#include <QFile>
#include <QtDebug>

namespace {
const int ERROR_STR_BUF_SIZE = 128;
}
VideoDecoder::VideoDecoder(QObject *parent) :
    QObject(parent), _formatContext(NULL), _codecContext(NULL),
    _codec(NULL), _frame(NULL), _frameRgb(NULL),
    _bufferSize(0), _frameBuffer(NULL), _swsContext(NULL),
    _currentFrame(-1), targetWidth(100), targetHeight(100)
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

void VideoDecoder::closeFramesAndBuffers()
{
    if (_swsContext)
	sws_freeContext(_swsContext);
    if (_frameBuffer)
	delete[] _frameBuffer;
    _frameBuffer = NULL;
    if (_frameRgb)
	av_free(_frameRgb);
    _frameRgb = NULL;
    if (_frame)
	av_free(_frame);
    _frame = NULL;
}

void VideoDecoder::close()
{
    closeFramesAndBuffers();
    _currentFrame = -1;

    _codec = NULL;
    if (_codecContext)
        avcodec_close(_codecContext);
    _codecContext = NULL;
    if (_formatContext)
        av_close_input_file(_formatContext);
    _formatContext = NULL;
}

void VideoDecoder::openFile(QString filename)
{
    close();
    int errorNr = avformat_open_input(&_formatContext, filename.toStdString().c_str(),
				      NULL, NULL);
    if (errorNr != 0) {
	printError(errorNr, QString("Unable to open %1").arg(filename));
    }
    errorNr = av_find_stream_info(_formatContext);
    if (errorNr < 0) {
	printError(errorNr, QString("Unable to find video stream"));
	emit error();
    }
    av_dump_format(_formatContext, 0, filename.toStdString().c_str(), 0);

    _videoStream = findVideoStream();
    if (_videoStream < 0) {
        qWarning("Unable to find video stream");
	emit error();
    }

    _codecContext = _formatContext->streams[_videoStream]->codec;
    if (!_codecContext) {
        qWarning("Unable to open codec context");
	emit error();
    }

    _codec = avcodec_find_decoder(_codecContext->codec_id);
    if (!_codec) {
        qWarning("Unable to find codec");
	emit error();
    }

    errorNr = avcodec_open2(_codecContext, _codec, NULL);
    if (errorNr < 0) {
	printError(errorNr, "Unable to open codec");
	emit error();
    }

    initializeFrames();


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

void VideoDecoder::initializeFrames()
{
    _frame = avcodec_alloc_frame();
    _frameRgb = avcodec_alloc_frame();

    _bufferSize = avpicture_get_size(PIX_FMT_RGB24, targetWidth, targetHeight);
    _frameBuffer = new quint8[_bufferSize];

    avpicture_fill(reinterpret_cast<AVPicture*>(_frameRgb), _frameBuffer, PIX_FMT_RGB24,
		   targetWidth, targetHeight);
    _swsContext = sws_getContext(_codecContext->width, _codecContext->height,
				 _codecContext->pix_fmt,
				 targetWidth, targetHeight,
				 PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
}

void VideoDecoder::nextFrame()
{
    decodeNextFrame();
}

void VideoDecoder::decodeNextFrame()
{
    int frameFinished = 0;
    AVPacket packet;
    _decodeTimer.restart();
    while(!frameFinished) {
	if (av_read_frame(_formatContext, &packet) < 0)
	    return;

	if (packet.stream_index == _videoStream) {

	    avcodec_decode_video2(_codecContext, _frame,
				  &frameFinished, &packet);
//	    int afterDecode = _decodeTimer.restart();
//	    qDebug() << "decoding video took " << afterDecode << " ms";
	    if (frameFinished) {
		sws_scale(_swsContext, _frame->data, _frame->linesize,
			  0, _codecContext->height, _frameRgb->data, _frameRgb->linesize);
//		int afterScale = _decodeTimer.restart();
//		qDebug() << "scaling frame took " << afterScale << " ms";
		// Convert the frame to QImage
//		int afterImage;
		{
		    QMutexLocker locker(&_mutex);
		    _currentImage = QImage(_frameRgb->data[0],
					   targetWidth,
					   targetHeight,
					   targetWidth * 3,
					   QImage::Format_RGB888);
//		    afterImage = _decodeTimer.restart();
//		    qDebug() << "converting to image took " << afterImage << " ms";
		}

		_currentFrame++;
		emit frameReady(static_cast<quint32>(_currentFrame));
		av_free_packet(&packet);
//		qDebug() << "Decoding frame took " << afterDecode + afterScale + afterImage << " ms in total";
	    }
	}
    }
}

void VideoDecoder::lock()
{
    _mutex.lock();
}

void VideoDecoder::unlock()
{
    _mutex.unlock();
}

const QImage* VideoDecoder::currentImage() const
{
    if (_currentImage.isNull())
	return NULL;
    return &_currentImage;
}

void VideoDecoder::targetSizeChanged(int width, int height)
{
    targetWidth = width;
    targetHeight = height;
    closeFramesAndBuffers();

    if (!_formatContext)
	return;

    initializeFrames();
}
