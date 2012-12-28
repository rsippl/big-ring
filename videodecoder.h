#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QDateTime>
#include <QObject>
#include <QImage>
#include <QPair>
#include <QQueue>
#include <QWaitCondition>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwsContext;
#include <limits>

const quint32 UNKNOWN_FRAME_NR = std::numeric_limits<quint32>::max();

typedef QPair<quint32, QImage> Frame;
typedef QList<Frame> FrameList;

class VideoDecoder : public QObject
{
	Q_OBJECT
public:
	explicit VideoDecoder(QObject *parent = 0);
	~VideoDecoder();

signals:
	void error();
	void videoLoaded();
	void framesReady(FrameList frames, quint32 requestId);
public slots:
	void seekFrame(quint32 frameNr);
	void openFile(QString filename);
	/** Load a number of frames from the video file */
	void loadFrames(quint32 numberOfFrame, quint32 requestId);

private:
	void close();
	void closeFramesAndBuffers();
	void initialize();
	void initializeFrames();
	Frame decodeNextFrame();

	int findVideoStream();
	void printError(int errorNr, const QString& message);

	AVFormatContext* _formatContext;
	AVCodecContext* _codecContext;
	AVCodec* _codec;
	AVFrame* _frame;
	AVFrame* _frameRgb;
	int _bufferSize;
	quint8 *_frameBuffer;
	SwsContext* _swsContext;
	QImage _currentImage;

	// seeking
	QTimer* _seekTimer;
	quint32 _seekTargetFrame;

	QDateTime _lastFillTime;
	int _videoStream;

	int _widgetWidth;
	int _widgetHeight;
};

#endif // VIDEODECODER_H
