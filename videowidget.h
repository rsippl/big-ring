#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTime>
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

    void paintGL();
    void resizeGL(int w, int h);
signals:

public slots:
    void realLiveVideoSelected(RealLiveVideo rlv);
    void courseSelected(int courseNr);


private slots:
    void playVideo();
    void frameReady(quint32 frameNr);

private:
    void drawImage(const QRectF &r, const QImage &image, const QRectF &sr);

    RealLiveVideo _currentRealLiveVideo;
    VideoDecoder* _videoDecoder;

    QTimer* _playDelayTimer;
    QTimer* _playTimer;
    QThread* _playThread;
    QTime _paintTime;
    QImage _glImage;
};

#endif // VIDEOWIDGET_H
