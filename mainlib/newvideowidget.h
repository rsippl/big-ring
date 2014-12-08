#ifndef NEWVIDEOWIDGET_H
#define NEWVIDEOWIDGET_H

#include <QtCore/QTimer>
#include <QWidget>
#include <QtWidgets/QGraphicsView>

extern "C" {
#include <gst/gst.h>
#include <gst/gstpipeline.h>
}
#include "reallifevideo.h"
#include "profileitem.h"
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
    static void onBusMessage(GstBus *bus, GstMessage *msg, NewVideoWidget* context);
    void handleAsyncDone();
    void setUpVideoSink();

    static void onVideoUpdate(GObject *src, guint, NewVideoWidget* context);

    void seekToStart();
    void step(int stepSize);

    void addClock(Simulation& simulation, QGraphicsScene* scene);
    void addWattage(Simulation& simulation, QGraphicsScene* scene);
    void addCadence(Simulation& simulation, QGraphicsScene* scene);
    void addSpeed(Simulation& simulation, QGraphicsScene* scene);
    void addGrade(Simulation& simulation, QGraphicsScene* scene);
    void addDistance(Simulation& simulation, QGraphicsScene* scene);
    void addHeartRate(Simulation& simulation, QGraphicsScene* scene);

    void pollBus();

    GstBus* _bus;
    GstElement* _pipeline;
    GstElement* _videoSink;
    RealLifeVideo _rlv;
    Course _course;
    LoadState _loadState;
    quint32 _currentFrame;
    QGraphicsItem* _clockItem;
    QGraphicsItem* _wattageItem;
    QGraphicsItem* _heartRateItem;
    QGraphicsItem* _cadenceItem;
    QGraphicsItem* _speedItem;
    QGraphicsItem* _distanceItem;
    QGraphicsItem* _gradeItem;
    ProfileItem* _profileItem;
    QTimer* _busTimer;
};

#endif // NEWVIDEOWIDGET_H
