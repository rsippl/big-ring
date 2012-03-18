#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QGLWidget>

#include "reallivevideo.h"

class QThread;
class VideoDecoder;

class VideoWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();

protected:
	void paintEvent(QPaintEvent *);
signals:

public slots:
    void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);

private slots:
    void playVideo();
	void frameReady(quint32 frameNr);

private:
    RealLiveVideo _currentRealLiveVideo;
    VideoDecoder* _videoDecoder;

    QTimer* _playDelayTimer;
	QTimer* _playTimer;
	QThread* _playThread;
};

#endif // VIDEOWIDGET_H
