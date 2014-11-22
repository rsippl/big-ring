#ifndef NEWVIDEOWIDGET_H
#define NEWVIDEOWIDGET_H

#include <QWidget>
#include <QGst/Pipeline>
#include <QGst/Ui/VideoWidget>

#include "reallifevideo.h"

class NewVideoWidget : public QGst::Ui::VideoWidget
{
    Q_OBJECT

    enum LoadState
    {
        NONE, VIDEO_LOADING, VIDEO_LOADED, SEEKING, DONE
    };

public:
    explicit NewVideoWidget(QWidget *parent = 0);
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

private:
    void onBusMessage(const QGst::MessagePtr & message);
    void handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm);

    void seekToStart();
    void step(int stepSize);

    QGst::PipelinePtr _pipeline;
    RealLifeVideo _rlv;
    Course _course;
    LoadState _loadState;
    quint32 _currentFrame;
};

#endif // NEWVIDEOWIDGET_H
