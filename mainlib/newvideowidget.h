#ifndef NEWVIDEOWIDGET_H
#define NEWVIDEOWIDGET_H

#include <QWidget>
#include <QtWidgets/QGraphicsView>
#include <QGst/Pipeline>
#include <QGst/Ui/GraphicsVideoWidget>
#include "reallifevideo.h"

class Simulation;

class NewVideoWidget : public QGraphicsView
{
    Q_OBJECT

    enum LoadState
    {
        NONE, VIDEO_LOADING, VIDEO_LOADED, SEEKING, DONE
    };

public:
    explicit NewVideoWidget(Simulation &simulation, QWidget *parent = 0);
    ~NewVideoWidget();

    bool isReadyToPlay();

signals:
    void stateChanged();
    void videoLoaded();
    void readyToPlay(bool ready);

public slots:
    void setRealLifeVideo(RealLifeVideo rlv);
    void setCourseIndex(int index);
    void stop();
    void setDistance(float distance);
protected:
    void resizeEvent(QResizeEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

private:
    void onBusMessage(const QGst::MessagePtr & message);
    void handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm);

    void seekToStart();
    void step(int stepSize);
    void fitVideoWidget();

    void setUpVideoSurface(QGraphicsScene* scene);
    void addClock(Simulation& simulation, QGraphicsScene* scene);

    QGst::Ui::GraphicsVideoWidget* _videoWidget;
    QGst::Ui::GraphicsVideoSurface* _videoSurface;
    QGst::PipelinePtr _pipeline;
    RealLifeVideo _rlv;
    Course _course;
    LoadState _loadState;
    quint32 _currentFrame;
};

#endif // NEWVIDEOWIDGET_H
