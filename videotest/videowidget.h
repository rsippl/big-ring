#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QGraphicsView>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include "openglpainter2.h"
#include "videoreader2.h"

class VideoWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    virtual ~VideoWidget();
signals:

public slots:
private slots:
    void setVideoInformation(const QString& videoFilename, const QSize& videoSize,
                             const qint64 numberOfFrames);
    void getNextFrame();
    void setFrameLoaded(int index, const QSize& frameSize);
    void setFrameNeeded(const FrameBuffer& frameBuffer);
protected:
    virtual void resizeEvent(QResizeEvent*);
    virtual void drawBackground(QPainter *painter, const QRectF &rect);
private:
    OpenGLPainter2* _painter;
    VideoReader2* _videoReader;
    QThread* _videoReaderThread;
    QTimer* _playTimer;
    QTime _time;
    qint64 _framesLoaded;
    qint64 _currentFrame;
};

#endif // VIDEOWIDGET_H
