#include "videodecoder.h"
extern "C"
{
#include <limits.h>
#include <stdint.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include <cmath>
#include <QDateTime>
#include <QFile>
#include <QMetaType>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>
#include <QtDebug>




namespace {
const int ERROR_STR_BUF_SIZE = 128;
}
VideoDecoder::VideoDecoder(QObject *parent) :
	QObject(parent),
	_formatContext(NULL), _codecContext(NULL),
	_codec(NULL), _frame(NULL), _frameRgb(NULL),
	_bufferSize(0), _frameBuffer(NULL), _swsContext(NULL),
	_seekTimer(new QTimer(this)), _seekTargetFrame(0),
	_lastFillTime(QDateTime::fromTime_t(0))
{
	qRegisterMetaType<FrameList>("FrameList");
	initialize();
}

VideoDecoder::~VideoDecoder()
{
	_seekTimer->stop();
	close();
}

void VideoDecoder::initialize()
{
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

	_codec = NULL;
	if (_codecContext)
		avcodec_close(_codecContext);
	_codecContext = NULL;
	if (_formatContext)
		avformat_close_input(&_formatContext);
}

void VideoDecoder::openFile(QString filename)
{
	close();
	int errorNr = avformat_open_input(&_formatContext, filename.toStdString().c_str(),
									  NULL, NULL);
	if (errorNr != 0) {
		printError(errorNr, QString("Unable to open %1").arg(filename));
	}
	errorNr = avformat_find_stream_info(_formatContext, NULL);
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
	emit videoLoaded();
}

void VideoDecoder::loadFrames(quint32 numberOfFrame, quint32 requestId)
{
	Frame frame;
	quint32 decoded = 0;
	FrameList frames;
	while(decoded < numberOfFrame) {
		frame = decodeNextFrame();
		if (frame.first == UNKNOWN_FRAME_NR)
			break;

		frames << frame;
		++decoded;
	}
	emit framesReady(frames, requestId);
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

	_bufferSize = avpicture_get_size(PIX_FMT_RGB24, _codecContext->width, _codecContext->height);
	_frameBuffer = new quint8[_bufferSize];

	avpicture_fill(reinterpret_cast<AVPicture*>(_frameRgb), _frameBuffer, PIX_FMT_RGB24,
				   _codecContext->width, _codecContext->height);
	_swsContext = sws_getContext(_codecContext->width, _codecContext->height,
								 _codecContext->pix_fmt,
								 _codecContext->width, _codecContext->height,
								 PIX_FMT_RGB24, SWS_POINT, NULL, NULL, NULL);
}

void VideoDecoder::seekFrame(quint32 frameNr)
{
	qDebug() << "seeking to frame " << frameNr;
	av_seek_frame(_formatContext, _videoStream, frameNr, AVSEEK_FLAG_ANY);
	_seekTargetFrame = frameNr;
}

Frame VideoDecoder::decodeNextFrame()
{

	QImage nullImage;
	Frame newImage = qMakePair(UNKNOWN_FRAME_NR, nullImage);

	/** We might get here before having loaded anything */
	if (_formatContext != NULL) {
		AVPacket packet;
		int frameFinished = 0;
		while(!frameFinished) {
			if (av_read_frame(_formatContext, &packet) < 0)
				break;

			if (packet.stream_index == _videoStream) {
				avcodec_decode_video2(_codecContext, _frame,
									  &frameFinished, &packet);
				if (frameFinished) {
					sws_scale(_swsContext, _frame->data, _frame->linesize,
							  0, _codecContext->height, _frameRgb->data, _frameRgb->linesize);
					_currentImage = QImage(_frameRgb->data[0],
										   _codecContext->width,
										   _codecContext->height,
										   _codecContext->width * 3,
										   QImage::Format_RGB888);
					newImage = qMakePair(static_cast<quint32>(packet.dts), _currentImage.copy());
					av_free_packet(&packet);
				}
			}
		}
	}
	return newImage;
}
