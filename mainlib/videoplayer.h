#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLContext>
extern "C" {
#include <gst/gst.h>
#include <gst/gstpipeline.h>
}

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit VideoPlayer(QGLWidget *paintWidget, QObject *parent = 0);
    ~VideoPlayer();



    /*!
     * \brief check if the player is ready to play or step.
     * \return true if ready to play, false otherwise.
     */
    bool isReadyToPlay();
signals:
    void videoLoaded(qint64 videoDurationNanoSeconds);
    void seekDone();
    void updateVideo();
public slots:
    void stop();
    void stepToFrame(quint32 frameNumber);

    void setUri(QString uri);
    bool seekToFrame(quint32 frameNumber, float frameRate);
    void displayCurrentFrame(QPainter* painter, QRectF rect);
private:
    void setUpVideoSink(QGLWidget *glWidget);
    static void onBusMessage(GstBus *bus, GstMessage *msg, VideoPlayer* context);
    static void onVideoUpdate(GObject *src, guint, VideoPlayer* context);
    void sendVideoUpdated();
    void handleAsyncDone();

    void pollBus();


    enum LoadState
    {
        NONE, VIDEO_LOADING, VIDEO_LOADED, SEEKING, DONE, PLAYING
    };

    GstElement* _pipeline;
    GstElement* _videoSink;
    GstBus* _pipelineBus;

    QTimer* _busTimer;
    LoadState _loadState;
    quint32 _currentFrameNumber;
};

#endif // VIDEOPLAYER_H
