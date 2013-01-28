#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QDateTime>
#include <QLinkedList>
#include <QObject>
#include <QImage>
#include <QPair>
#include <QQueue>
#include <QWaitCondition>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
#include <limits>

const quint32 UNKNOWN_FRAME_NR = std::numeric_limits<quint32>::max();

typedef QPair<quint32, QImage> Frame;
typedef QLinkedList<Frame> FrameList;

class VideoDecoder : public QObject
{
	Q_OBJECT
public:
	explicit VideoDecoder(QObject *parent = 0);
	~VideoDecoder();

	Frame convertFrameToQImage(AVPacket &packet);
signals:
	void error();
	void videoLoaded();
	void framesReady(FrameList frames, quint32 requestId);
	void seekFinished(Frame frame);

public slots:
	void seekFrame(quint32 frameNr);
	void openFile(QString filename);
	/** Load a number of frames from the video file */
	void loadFrames(quint32 numberOfFrame, quint32 requestId);

private slots:
	void decodeUntilCorrectFrame();

private:
	void close();
	void closeFramesAndBuffers();
	void initialize();
	void initializeFrames();
	Frame decodeNextFrame();
	bool decodeNextAVFrame(AVPacket& packet);

	int findVideoStream();
	void printError(int errorNr, const QString& message);

	AVFormatContext* _formatContext;
	AVCodecContext* _codecContext;
	AVCodec* _codec;
	AVFrame* _frame;
	SwsContext* _swsContext;
	QImage _currentImage;

	// seeking
	QTimer* _seekTimer;
	quint32 _seekTargetFrame;

	int _videoStream;

    QScopedArrayPointer<int> _lineSizes;
};

#endif // VIDEODECODER_H
