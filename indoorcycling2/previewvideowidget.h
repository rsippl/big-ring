#ifndef PREVIEWVIDEOWIDGET_H
#define PREVIEWVIDEOWIDGET_H

#include <QtCore/QTimer>
#include <QtCore/QtGlobal>
#include <QtWidgets/QWidget>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsWidget>
#include <QGst/Message>
#include <QGst/Pipeline>
#include "reallifevideo.h"

class PreviewVideoWidget: public QWidget
{
    Q_OBJECT
public:
    PreviewVideoWidget(QWidget* parent);
    virtual ~PreviewVideoWidget();
    void setRealLifeVideo(RealLifeVideo& rlv);
    void setCourse(const Course& course);
signals:
    void stateChanged();
    void videoLoaded();

public slots:
    void play();
    void step(int stepSize);
    void updateText();

    void setDistance(float distance);

protected:
    void resizeEvent(QResizeEvent *);
private:
    void onBusMessage(const QGst::MessagePtr & message);
    void handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm);
    void setUri(QString uri);
    void stop();
    void seek();

    RealLifeVideo _rlv;
    Course _course;
    quint32 _currentFrame;

    QGst::ElementPtr _videoSink;
    QGst::PipelinePtr _pipeline;
    QTimer* _textTimer;
    QGraphicsView* _graphicsView;
    QGraphicsWidget* _videoWidget;
    QGraphicsTextItem* _text;
    bool _seekDone;
};

#endif // PREVIEWVIDEOWIDGET_H
