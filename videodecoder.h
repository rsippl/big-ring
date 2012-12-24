#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwsContext;
#include <limits>

const quint32 UNKNOWN_FRAME_NR = std::numeric_limits<quint32>::max();

class ImageFrame {
public:
	ImageFrame(): _frameNr(UNKNOWN_FRAME_NR) {}
	ImageFrame(quint32 frameNr, QImage image): _frameNr(frameNr), _image(image) {}
	quint32 frameNr() const { return _frameNr; }
	QImage image() const { return _image; }
private:
	quint32 _frameNr;
	QImage _image;
};

class ImageQueue: public QObject
{
	Q_OBJECT
public:
	ImageQueue(int capacity, int low, QObject *parent = NULL);
	bool offer(ImageFrame& image);
	ImageFrame take();
	void drain();
signals:
	void lowMarkReached();

private:
	int _capacity;
	int _low;
	QQueue<ImageFrame> _queue;
	QMutex _mutex;
	QWaitCondition _condition;
};

class VideoDecoder : public QObject
{
	Q_OBJECT
public:
	explicit VideoDecoder(ImageQueue* imageQueue, QObject *parent = 0);
	~VideoDecoder();

signals:
	void error();
	void videoLoaded();
	void bufferFilled();
public slots:
	void seekFrame(quint32 frameNr);
	void openFile(QString filename);

private slots:
	void refillBuffer();
	void seekDelayFinished();
private:
	void close();
	void closeFramesAndBuffers();
	void initialize();
	void initializeFrames();
	ImageFrame decodeNextFrame();

	int findVideoStream();
	void printError(int errorNr, const QString& message);

	ImageQueue* _imageQueue;
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


	int _videoStream;
	QMutex _mutex;

	qint32 _currentFrame;
	int _widgetWidth;
	int _widgetHeight;
};

#endif // VIDEODECODER_H
