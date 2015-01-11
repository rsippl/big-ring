#ifndef NEWVIDEOWIDGET_H
#define NEWVIDEOWIDGET_H

#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>
#include <QtWidgets/QGraphicsView>

#include "reallifevideo.h"
#include "profileitem.h"
class Simulation;
class VideoPlayer;

class NewVideoWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit NewVideoWidget(Simulation &simulation, QWidget *parent = 0);
    ~NewVideoWidget();

    bool isReadyToPlay();
signals:
    void readyToPlay(bool ready);

public slots:
    void setRealLifeVideo(RealLifeVideo rlv);
    void setCourse(Course& course);
    void setCourseIndex(int index);
    void setDistance(float distance);
protected:
    virtual void focusOutEvent(QFocusEvent*) override;
    virtual void focusInEvent(QFocusEvent*) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent *) override;
    virtual void enterEvent(QEvent *) override;
    virtual void leaveEvent(QEvent *) override;
    virtual void closeEvent(QCloseEvent*) override;
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void setupVideoPlayer(QGLWidget* paintWidget);

    void seekToStart(Course& course);
    void step(int stepSize);

    void addClock(Simulation& simulation, QGraphicsScene* scene);
    void addWattage(Simulation& simulation, QGraphicsScene* scene);
    void addCadence(Simulation& simulation, QGraphicsScene* scene);
    void addSpeed(Simulation& simulation, QGraphicsScene* scene);
    void addGrade(Simulation& simulation, QGraphicsScene* scene);
    void addDistance(Simulation& simulation, QGraphicsScene* scene);
    void addHeartRate(Simulation& simulation, QGraphicsScene* scene);

    RealLifeVideo _rlv;
    Course _course;
    VideoPlayer* _videoPlayer;

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
