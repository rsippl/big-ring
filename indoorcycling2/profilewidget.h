#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QtWidgets/QWidget>

#include "reallifevideo.h"

class ProfilePainter;
class ProfileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ProfileWidget(QWidget *parent = 0);

    virtual QSize sizeHint() const override;
public slots:
    void setVideo(RealLifeVideo& rlv);
    void setCourseIndex(const int courseIndex);
protected:
    virtual void paintEvent(QPaintEvent *) override;
private:
    ProfilePainter* _profilePainter;
    RealLifeVideo _currentRlv;
    int _courseIndex;
};

#endif // PROFILEWIDGET_H
