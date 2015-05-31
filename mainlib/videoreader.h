#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <QtCore/QEvent>
#include <QtCore/QObject>

class RealLifeVideo;
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwsContext;



class VideoReader : public QObject
{
    Q_OBJECT
public:
    explicit VideoReader(QObject *parent = 0);
    virtual ~VideoReader();

    void createImageForFrame(const RealLifeVideo& rlv, const qreal distance);
signals:
    void error(const QString& errorMessage);
    void videoOpened(const QString& videoFilename, const QSize& videoSize);
    void newFrameReady(const RealLifeVideo& rlv, qreal distance, const QImage& frame);
    void seekReady(qint64 frameNumber);

protected:
    virtual bool event(QEvent *);
private:
    void initialize();
    void close();
    void openVideoFile(const QString& videoFilename);

    void performSeek(qint64 targetFrameNumber);
    void loadFramesUntilTargetFrame(qint64 targetFrameNumber);
    qint64 loadNextFrame();
    void createImageForFrameNumber(RealLifeVideo &rlv, const qreal distance);
    void printError(int errorNumber, const QString& message);
    void printError(const QString &message);
    QImage createImage();
    // libav functions
    int findVideoStream(AVFormatContext* formatContext) const;

    bool _initialized;
    // libav specific data
    AVCodec* _codec;
    AVCodecContext* _codecContext;
    AVFormatContext* _formatContext;
    AVFrame* _frame;
    AVFrame* _frameRgb;
    SwsContext* _swsContext;
    QByteArray _imageBuffer;
    int _currentVideoStream;
};

#endif // VIDEOREADER_H
