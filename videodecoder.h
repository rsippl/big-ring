#ifndef VIDEODECODER_H
#define VIDEODECODER_H
#include <QDateTime>
#include <QLinkedList>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QTimer>
#include <QWaitCondition>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
#include <limits>

const quint32 UNKNOWN_FRAME_NR = std::numeric_limits<quint32>::max();

struct Frame
{
	quint32 frameNr;
	AVFrame* avFrame;
};

typedef QLinkedList<Frame> FrameList;

class VideoFrameSink {
public:
	virtual ~VideoFrameSink() { /* empty */ }
	virtual void offerFrame(Frame& frame) = 0;
	virtual void seekFinished(Frame& frame) = 0;
};

class VideoDecoder : public QObject
{
	Q_OBJECT
public:
	explicit VideoDecoder(VideoFrameSink* sink, QObject *parent = 0);
	~VideoDecoder();

	Frame convertFrame(AVPacket &packet);
signals:
	void error();
	void videoLoaded();
	void framesReady(FrameList frames);

public slots:
	void seekFrame(quint32 frameNr);
	void openFile(QString filename);
	/** Load a number of frames from the video file */
	void loadFrames(quint32 skip);

private slots:
	void decodeUntilCorrectFrame();

private:
	void close();
	void closeFramesAndBuffers();
	void initialize();
	void initializeFrames();
	void decodeNextFrame(bool offerToSink);
	void skipNextFrame();
	bool decodeNextAVFrame(AVPacket& packet);

	int findVideoStream();
	void printError(int errorNr, const QString& message);

	VideoFrameSink* _sink;
	AVFormatContext* _formatContext;
	AVCodecContext* _codecContext;
	AVCodec* _codec;
	AVFrame* _frame;

	// seeking
	QTimer* _seekTimer;
	quint32 _seekTargetFrame;

	int _videoStream;
};

#endif // VIDEODECODER_H
