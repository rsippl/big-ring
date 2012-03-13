#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QWidget>

#include "reallivevideo.h"
#include "videodecoder.h"
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
private:
    RealLiveVideo _currentRealLiveVideo;
    VideoDecoder* _videoDecoder;

    QTimer* _playDelayTimer;
};

#endif // VIDEOWIDGET_H
