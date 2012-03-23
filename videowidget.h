#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QGLWidget>

#include "reallivevideo.h"
#include "videodecoder.h"

class QThread;

class VideoWidget : public QGLWidget, public VideoImageHandler
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();

    void paintGL();
    void resizeGL(int w, int h);
    virtual void handleImage(const QImage& image);
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
    QImage _glImage;
};

#endif // VIDEOWIDGET_H
