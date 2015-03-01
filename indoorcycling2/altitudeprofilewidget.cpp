#include "altitudeprofilewidget.h"
#include "ui_altitudeprofilewidget.h"

#include <QtGui/QPainter>

#include "profilepainter.h"

AltitudeProfileWidget::AltitudeProfileWidget(QWidget *parent) :
    QWidget(parent),
    _startDistance(-1), _endDistance(-1),
    ui(new Ui::AltitudeProfileWidget)
{
    ui->setupUi(this);
}

AltitudeProfileWidget::~AltitudeProfileWidget()
{
    delete ui;
}


void AltitudeProfileWidget::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    _startDistance = -1;
    _endDistance = -1;
    update();
}

void AltitudeProfileWidget::setCourseIndex(const int courseIndex)
{
    if (courseIndex >= 0 && _currentRlv.isValid()) {
        const Course& course = _currentRlv.courses()[courseIndex];
        _startDistance = course.start();
        _endDistance = course.end();
    } else {
        _startDistance = -1;
    }
    update();
}

void AltitudeProfileWidget::setStartAndEndDistance(qreal startDistance, qreal endDistance)
{
    _startDistance = startDistance;
    _endDistance = endDistance;
    update();
}

void AltitudeProfileWidget::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    QPixmap profilePixmap;
    if (_startDistance > 0) {
        profilePixmap = _profilePainter->paintProfileWithHighLight(_currentRlv, _startDistance, _endDistance,
                                                                   this->rect(), palette().highlight());
    } else {
        profilePixmap = _profilePainter->paintProfile(_currentRlv, this->rect());
    }
    painter.drawPixmap(rect(), profilePixmap);
    QWidget::paintEvent(paintEvent);
}

