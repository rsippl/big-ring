#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QWidget>

#include <vlc/vlc.h>

#include "reallivevideo.h"
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();
signals:

public slots:
    void realLiveVideoSelected(RealLiveVideo rlv);

private slots:
    void playVideo();
    void fastForward();
private:
    RealLiveVideo currentRealLiveVideo;
    QTimer* playDelayTimer;
    QTimer* fastForwardTimer;
    libvlc_instance_t *vlcObject;
    libvlc_media_player_t *vlcPlayer;
};

#endif // VIDEOWIDGET_H
