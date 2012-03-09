#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include "ffmpeg.h"

class VideoDecoder : public QObject
{
    Q_OBJECT
public:
    explicit VideoDecoder(QObject *parent = 0);

    bool openFile(QString filename);
signals:

public slots:

private:
    void close();
    void initialize();

    ffmpeg::AVFormatContext* _formatContext;
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
