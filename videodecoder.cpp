#include "videodecoder.h"
extern "C"
{
#include <limits.h>
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
}
#include <cmath>
#include <QDateTime>
#include <QFile>
#include <QMetaType>
#include <QThread>
#include <QTimer>
#include <QtDebug>




namespace {
const int ERROR_STR_BUF_SIZE = 128;
}
VideoDecoder::VideoDecoder(QObject *parent) :
	QObject(parent),
	_formatContext(NULL), _codecContext(NULL),
	_codec(NULL), _frame(NULL),
	_seekTimer(new QTimer(this)), _seekTargetFrame(0)
{
	qRegisterMetaType<Frame>("Frame");
	qRegisterMetaType<FrameList>("FrameList");
	initialize();
	_seekTimer->setSingleShot(true);
	connect(_seekTimer, SIGNAL(timeout()), SLOT(decodeUntilCorrectFrame()));
}

VideoDecoder::~VideoDecoder()
{
	qDebug() << "destroying videodecoder" << QThread::currentThreadId();
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

void VideoDecoder::loadFrames(quint32 numberOfFrame, quint32 skip)
{
	if (numberOfFrame > 1) {
		qDebug() << "request for" << numberOfFrame << "frames";
	}
	QTime now;
	now.start();
	Frame frame;
	quint32 decoded = 0;
	quint32 skipped = 0;
	FrameList frames;
	while(frames.size() < (int) numberOfFrame) {
		if (decoded % (skip + 1) == 0) {
			frame = decodeNextFrame();
			if (frame.frameNr == UNKNOWN_FRAME_NR)
				break;
			frames << frame;
		} else {
			skipNextFrame();
			++skipped;
		}
		++decoded;
	}
	if (frames.isEmpty())
		qDebug() << "request finished. No frames found.";
	else
		if (now.elapsed() > 100) {
			qDebug() << "request finished. Frames." << frames.first().frameNr
					 << "to" << frames.last().frameNr << "skipped" << skipped << "frames. Took" << now.elapsed() << "ms";
		}
	emit framesReady(frames);

}

// Find the target frame of a seek.
void VideoDecoder::decodeUntilCorrectFrame()
{
	AVPacket packet;
	quint32 frameNr = 0;
	while(frameNr <= _seekTargetFrame) {
		if (!decodeNextAVFrame(packet))
			return;

		frameNr = packet.dts;
	}

	emit seekFinished(convertFrame(packet));
	av_free_packet(&packet);
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

	_lineSizes.reset(new int[_codecContext->height]);
	for (int line = 0; line < _codecContext->height; ++line)
		_lineSizes[line] = _codecContext->width * 4;
}

void VideoDecoder::seekFrame(quint32 frameNr)
{
	quint32 frameToSeek = (frameNr > 250) ? frameNr - 250: 0;
	qDebug() << "need to seek to frame" << frameNr << "but seeking shorter to hopefully get a key frame before:" << frameToSeek;
	av_seek_frame(_formatContext, _videoStream, frameToSeek, AVSEEK_FLAG_FRAME);
	avcodec_flush_buffers(_codecContext);
	_seekTargetFrame = frameNr;
	_seekTimer->start(500);
}

Frame VideoDecoder::convertFrame(AVPacket& packet)
{
	Frame frame;
	frame.frameNr = packet.dts;
	frame.width = _codecContext->width;
	frame.height = _codecContext->height;
	frame.numBytes = _lineSizes[0] * _codecContext->height;
	quint8* ptr = (quint8*)malloc((_frame->linesize[0] * _codecContext->height * 6) / 4);
	mempcpy(ptr, _frame->data[0], _frame->linesize[0] * _codecContext->height);
	size_t uOffset = _frame->linesize[0] * _codecContext->height;
	mempcpy(ptr + uOffset, _frame->data[1], _frame->linesize[0] * _codecContext->height / 4);
	size_t vOffset = uOffset + (uOffset / 4);
	mempcpy(ptr + vOffset, _frame->data[2], _frame->linesize[0] * _codecContext->height / 4);
	frame.data = QSharedPointer<quint8>(ptr);

	frame.yLineSize = _frame->linesize[0];
	frame.uLineSize = _frame->linesize[1];
	frame.vLineSize = _frame->linesize[1];

	return frame;
}

Frame VideoDecoder::decodeNextFrame()
{
	Frame newFrame;
	newFrame.frameNr = UNKNOWN_FRAME_NR;

	/** We might get here before having loaded anything */
	if (_formatContext != NULL) {
		AVPacket packet;

		if (decodeNextAVFrame(packet)) {
			newFrame = convertFrame(packet);
		}
		av_free_packet(&packet);
	}
	return newFrame;
}

void VideoDecoder::skipNextFrame()
{
	AVPacket packet;
	decodeNextAVFrame(packet);
}

bool VideoDecoder::decodeNextAVFrame(AVPacket& packet)
{
	int frameFinished = 0;
	while (!frameFinished) {
		if (av_read_frame(_formatContext, &packet) < 0)
			return false;
		if (packet.stream_index == _videoStream) {
			avcodec_decode_video2(_codecContext, _frame,
								  &frameFinished, &packet);
		}
	}
	return true;
}
