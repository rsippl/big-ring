#include "altitudeprofilewidget.h"
#include "ui_altitudeprofilewidget.h"

#include <QtGui/QPainter>

#include "profilepainter.h"

AltitudeProfileWidget::AltitudeProfileWidget(QWidget *parent) :
    QWidget(parent),
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
    _courseIndex = -1;
    repaint();
}

void AltitudeProfileWidget::setCourseIndex(const int courseIndex)
{
    _courseIndex = courseIndex;
    repaint();
}

void AltitudeProfileWidget::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    QPixmap profilePixmap;
    if (_courseIndex >= 0) {
        const Course& course = _currentRlv.courses()[_courseIndex];
        profilePixmap = _profilePainter->paintProfileWithHighLight(_currentRlv, course.start(), course.end(),
                                                                   this->rect(), palette().highlight());
    } else {
        profilePixmap = _profilePainter->paintProfile(_currentRlv, this->rect());
    }
    painter.drawPixmap(rect(), profilePixmap);
    QWidget::paintEvent(paintEvent);
}

