#ifndef NEWVIDEOWIDGET_H
#define NEWVIDEOWIDGET_H

#include <QWidget>
#include <QtWidgets/QGraphicsView>
#include <QGst/Pipeline>
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
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void onBusMessage(const QGst::MessagePtr & message);
    void handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm);
    void setUpVideoSink();

    void onVideoUpdate();

    void seekToStart();
    void step(int stepSize);

    void addClock(Simulation& simulation, QGraphicsScene* scene);
    void addWattage(Simulation& simulation, QGraphicsScene* scene);

    QGst::ElementPtr _videoSink;
    QGst::PipelinePtr _pipeline;
    RealLifeVideo _rlv;
    Course _course;
    LoadState _loadState;
    quint32 _currentFrame;
    QGraphicsItem* _clockItem;
    QGraphicsItem* _wattageItem;
};

#endif // NEWVIDEOWIDGET_H
