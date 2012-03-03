#include "videowidget.h"
#include <QtDebug>
VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent), timer(new QTimer(this)), vlcPlayer(NULL)
{
    timer->setSingleShot(true);
    timer->setInterval(250);
    connect(timer, SIGNAL(timeout()), SLOT(playVideo()));

    /* Init libVLC */
    if((vlcObject = libvlc_new(0,NULL)) == NULL) {
        qWarning() << Q_FUNC_INFO << "Could not init libVLC";
        exit(1);
    }
}

VideoWidget::~VideoWidget()
{
    if (vlcPlayer)
        libvlc_media_player_release(vlcPlayer);
    if (vlcObject)
        libvlc_release(vlcObject);
}

void VideoWidget::playVideo()
{
    if (vlcPlayer && libvlc_media_player_is_playing(vlcPlayer)) {
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
}

void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
    if (timer->isActive())
        timer->stop();
    currentRealLiveVideo = rlv;

    timer->start();
}


