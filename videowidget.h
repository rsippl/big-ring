#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QVideoWidget>
#include <QMediaPlayer>

#include "reallivevideo.h"
#include "videodecoder.h"

class QThread;

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();

signals:

public slots:
    void realLiveVideoSelected(RealLiveVideo rlv);
    void courseSelected(int courseNr);


private slots:
	void playVideo();
	void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:

	QVideoWidget *_videoWidget;
	QMediaPlayer *_mediaPlayer;

    RealLiveVideo _currentRealLiveVideo;

    QTimer* _playDelayTimer;
};

#endif // VIDEOWIDGET_H
