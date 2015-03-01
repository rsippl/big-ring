#ifndef ALTITUDEPROFILEWIDGET_H
#define ALTITUDEPROFILEWIDGET_H

#include <QWidget>

#include "reallifevideo.h"

class ProfilePainter;

namespace Ui {
class AltitudeProfileWidget;
}

class AltitudeProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AltitudeProfileWidget(QWidget *parent = 0);
    ~AltitudeProfileWidget();
public slots:
    void setVideo(RealLifeVideo& rlv);
    void setCourseIndex(const int courseIndex);
    void setStartAndEndDistance(qreal startDistance, qreal endDistance);
protected:
    virtual void paintEvent(QPaintEvent *) override;
private:
    ProfilePainter* _profilePainter;
    RealLifeVideo _currentRlv;
    qreal _startDistance;
    qreal _endDistance;
    Ui::AltitudeProfileWidget *ui;
};

#endif // ALTITUDEPROFILEWIDGET_H
