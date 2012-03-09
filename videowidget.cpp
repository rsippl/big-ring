#include "videowidget.h"
#include <QtDebug>
#include <QResizeEvent>

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent), _videoDecoder(new VideoDecoder(this)),
    _playDelayTimer(new QTimer(this))
{
    _playDelayTimer->setSingleShot(true);
    _playDelayTimer->setInterval(250);
    connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));
}

VideoWidget::~VideoWidget()
{
    // nooop
}

void VideoWidget::playVideo()
{
    _videoDecoder->openFile(_currentRealLiveVideo.videoInformation().videoFilename());
    // stop current video if playing
    // open new video and start playing.
}


void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
    if (_playDelayTimer->isActive())
        _playDelayTimer->stop();
    _currentRealLiveVideo = rlv;

    _playDelayTimer->start();
}
