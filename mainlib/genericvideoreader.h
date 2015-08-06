#ifndef GENERICVIDEOREADER_H
#define GENERICVIDEOREADER_H

#include <QObject>

struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPicture;

/** Simple wrapper around an AVFrame struct. */
class AVFrameWrapper {
public:
    AVFrameWrapper();
    ~AVFrameWrapper();

    /** return the AVFrame as an AVPicture. */
    AVPicture* asPicture();

    AVFrame* frame;
};

class GenericVideoReader : public QObject
{
    Q_OBJECT
public:
    explicit GenericVideoReader(QObject *parent = 0);
    virtual ~GenericVideoReader();

signals:
    void error(const QString& errorMessage);
    void seekReady(qint64 frameNumber);

public slots:

protected:
    virtual void openVideoFileInternal(const QString &videoFilename);
    void performSeek(qint64 targetFrameNumber);
    void loadFramesUntilTargetFrame(qint64 targetFrameNumber);
    qint64 loadNextFrame();
    qint64 totalNumberOfFrames();

    AVCodecContext *codecContext() const;
    AVFormatContext *formatContext() const;
    AVFrameWrapper &frameYuv() const;
private:
    void initialize();
    void close();
    void printError(int errorNumber, const QString& message);
    void printError(const QString &message);
    int findVideoStream(AVFormatContext* formatContext) const;

    bool _initialized = false;
    // libav specific data
    AVCodec* _codec;
    AVCodecContext* _codecContext;
    AVFormatContext* _formatContext;
    QScopedPointer<AVFrameWrapper> _frameYuv;
    int _currentVideoStream;
};

#endif // GENERICVIDEOREADER_H
