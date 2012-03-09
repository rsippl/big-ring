#include "videodecoder.h"
//using namespace ffmpeg;
VideoDecoder::VideoDecoder(QObject *parent) :
    QObject(parent), _formatContext(NULL)
{
    initialize();
}

void VideoDecoder::initialize()
{
    ffmpeg::avcodec_init();
    ffmpeg::avcodec_register_all();
    ffmpeg::av_register_all();
}

void VideoDecoder::close()
{
    if (_formatContext)
        av_close_input_file(_formatContext);
    _formatContext = NULL;
}

bool VideoDecoder::openFile(QString filename)
{

    if (avformat_open_input(&_formatContext, filename.toStdString().c_str(),
                            NULL, NULL) != 0)
        return false;

    if (av_find_stream_info(_formatContext) < 0)
        return false;

    av_dump_format(_formatContext, 0, filename.toStdString().c_str(), 0);

    return true;
}
