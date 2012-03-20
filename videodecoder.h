#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QTime>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwsContext;

class VideoDecoder : public QObject
{
    Q_OBJECT
public:
    explicit VideoDecoder(QObject *parent = 0);
    ~VideoDecoder();

    void lock();
    void unlock();
    const QImage* currentImage() const;


signals:
    void frameReady(quint32 frameNr);
    void error();
public slots:
    void nextFrame();
    void openFile(QString filename);
    void targetSizeChanged(int width, int height);

private:
    void close();
    void closeFramesAndBuffers();
    void initialize();
    void initializeFrames();
    void decodeNextFrame();

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


    int _videoStream;
    QMutex _mutex;
    QTime _decodeTimer;

    qint32 _currentFrame;
    int targetWidth;
    int targetHeight;
};

#endif // VIDEODECODER_H
