#include "videowidget.h"
#include <QtDebug>
#include <QResizeEvent>

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent), playDelayTimer(new QTimer(this)),
    fastForwardTimer(new QTimer(this)),
    vlcPlayer(NULL)
{
    playDelayTimer->setSingleShot(true);
    playDelayTimer->setInterval(250);
    fastForwardTimer->setInterval(10000);
    connect(playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));
    connect(fastForwardTimer, SIGNAL(timeout()), SLOT(fastForward()));

    /* Init libVLC */
    const char * const vlc_args[] = {
                       "-I", "dummy", /* Don't use any interface */
                       "--ignore-config", /* Don't use VLC's config */
                       "--disable-screensaver", /* disable screensaver during playback */
                       "--no-xlib", // avoid xlib thread error messages
                       "--verbose=-1", // -1 = no output at all
                       "--quiet"
                   };

    if((vlcObject = libvlc_new(0,vlc_args)) == NULL) {
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

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "new size is " << event->size();
}
