#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLContext>
extern "C" {
#include <gst/gst.h>
#include <gst/gstpipeline.h>
}

/*!
 * \brief Video player for cycling videos. This is a frame based player, so clients can seek to
 * a particular frame and choose to step to a particular frame number.
 *
 * This video player will paint the frames from a video file onto a QGLWidget. A frame is displayed
 * using the displayCurrentFrame method.
 *
 *
 */
class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief construct a VideoPlayer using the \param paintWidget as the surface to paint to. Optionally,
     * set the \param parent to help disposing this element when no longer needed.
     */
    explicit VideoPlayer(QGLWidget *paintWidget, QObject *parent = 0);

    /*!
     * \brief destroy the videoplayer by cleaning up all internal resources.
     */
    ~VideoPlayer();

    /*!
     * \brief check if the player is ready to play or step.
     * \return true if ready to play, false otherwise.
     */
    bool isReadyToPlay();

signals:
    /*!
     * \brief signal emitted when a video is loaded. \param videoDurationNanoSeconds will hold the length
     * of the video in nanoseconds.
     */
    void videoLoaded(qint64 videoDurationNanoSeconds);
    /*!
     * \brief seekDone is emitted when a seek action is ready.
     */
    void seekDone();
    /*!
     * \brief updateVideo is emitted when a new frame is ready to be shown. Clients should call displayCurrentFrame after
     * this.
     */
    void updateVideo();
public slots:
    /*! stop the video */
    void stop();
    /*! step to a \param frameNumber. */
    void stepToFrame(quint32 frameNumber);

    /*! set the \param uri of the video file and load it. */
    void loadVideo(QString uri);

    bool seekToFrame(quint32 frameNumber, float frameRate);
    void displayCurrentFrame(QPainter* painter, QRectF rect);
private:
    void cleanupCurrentPipeline();
    void createPipeline();
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