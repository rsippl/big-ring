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
protected:
    virtual void paintEvent(QPaintEvent *) override;
private:
    ProfilePainter* _profilePainter;
    RealLifeVideo _currentRlv;
    int _courseIndex;
    Ui::AltitudeProfileWidget *ui;
};

#endif // ALTITUDEPROFILEWIDGET_H
