#include "videowidget.h"
#include <QtDebug>
VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent), playDelayTimer(new QTimer(this)),
    fastForwardTimer(new QTimer(this)),
    vlcPlayer(NULL)
{
    playDelayTimer->setSingleShot(true);
    playDelayTimer->setInterval(250);
    fastForwardTimer->setInterval(2000);
    connect(playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));
    connect(fastForwardTimer, SIGNAL(timeout()), SLOT(fastForward()));

    /* Init libVLC */
    if((vlcObject = libvlc_new(0,NULL)) == NULL) {
        qWarning() << Q_FUNC_INFO << "Could not init libVLC";
        exit(1);
    }
}

VideoWidget::~VideoWidget()
{
    if (fastForwardTimer->isActive())
        fastForwardTimer->stop();
    if (vlcPlayer)
        libvlc_media_player_release(vlcPlayer);
    if (vlcObject)
        libvlc_release(vlcObject);
}

void VideoWidget::playVideo()
{
    if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer)) {
        fastForwardTimer->stop();
        libvlc_media_player_stop(vlcPlayer);
        libvlc_media_player_release(vlcPlayer);
    }

    libvlc_media_t* movieFile = libvlc_media_new_path(vlcObject, currentRealLiveVideo.videoInformation().videoFilename().toUtf8());
    if (!movieFile)
        return;
    vlcPlayer = libvlc_media_player_new_from_media(movieFile);
    libvlc_media_release(movieFile);
    libvlc_media_player_set_xwindow(vlcPlayer, this->winId());
    libvlc_media_player_play(vlcPlayer);
    fastForwardTimer->start();
}

void VideoWidget::fastForward()
{
    if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer)) {
        float currentPosition = libvlc_media_player_get_position(vlcPlayer);
        currentPosition += 0.02;
        if (currentPosition > 1.0)
            currentPosition -= 1.0;
        libvlc_media_player_set_position(vlcPlayer, currentPosition);
    }
}

void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
    if (playDelayTimer->isActive())
        playDelayTimer->stop();
    currentRealLiveVideo = rlv;

    playDelayTimer->start();
}


