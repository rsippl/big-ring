#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <QtCore/QEvent>
#include <QtCore/QObject>

#include "framebuffer.h"

class RealLifeVideo;
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;

class VideoReader2 : public QObject
{
    Q_OBJECT
public:
    explicit VideoReader2(QObject *parent = 0);
    virtual ~VideoReader2();

    void openVideoFile(const QString &videoFilename);
    void copyNextFrame(const FrameBuffer& buffer, int skipFrames = 0);
    void seekToFrame(qint64 frameNumber);
signals:
    void error(const QString& errorMessage);
    void videoOpened(const QString& videoFilename, const QSize& videoSize,
                     const QSize& totalFrameSize, const qint64 numberOfFrames);
    void frameCopied(int index, qint64 frameNumber, const QSize& frameSize);
    void seekReady(qint64 frameNumber);

protected:
    virtual bool event(QEvent *);
private:
    void initialize();
    void close();

    void openVideoFileInternal(const QString &videoFilename);
    void copyNextFrameInternal(const FrameBuffer &buffer, int skipFrames);
    void seekToFrameInternal(const qint64 frameNumber);
    void performSeek(qint64 targetFrameNumber);
    void loadFramesUntilTargetFrame(qint64 targetFrameNumber);
    qint64 loadNextFrame();
    QSize totalFrameSize();
    void printError(int errorNumber, const QString& message);
    void printError(const QString &message);

    // libav functions
    int findVideoStream(AVFormatContext* formatContext) const;

    bool _initialized;
    // libav specific data
    AVCodec* _codec;
    AVCodecContext* _codecContext;
    AVFormatContext* _formatContext;
    AVFrame* _frame;

    qint64 _currentFrameNumber;
    int _currentVideoStream;

};

#endif // VIDEOREADER_H
