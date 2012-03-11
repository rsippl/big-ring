#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;

class VideoDecoder : public QObject
{
    Q_OBJECT
public:
    explicit VideoDecoder(QObject *parent = 0);
    ~VideoDecoder();

    bool openFile(QString filename);
signals:

public slots:

private:
    void close();
    void initialize();

    int findVideoStream();
    void printError(int errorNr, const QString& message);
    AVFormatContext* _formatContext;
    AVCodecContext* _codecContext;
    AVCodec* _codec;
    AVFrame* _frame;
    AVFrame* _frameRgb;
    int _videoStream;
    int _bufferSize;
    quint8 *_frameBuffer;
//          int videoStream;
//          ffmpeg::AVCodecContext  *pCodecCtx;
//          ffmpeg::AVCodec         *pCodec;
//          ffmpeg::AVFrame         *pFrame;
//          ffmpeg::AVFrame         *pFrameRGB;
//          ffmpeg::AVPacket        packet;
//          ffmpeg::SwsContext      *img_convert_ctx;
//          uint8_t                 *buffer;
//          int                     numBytes;

};

#endif // VIDEODECODER_H
