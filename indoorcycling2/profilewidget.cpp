#include "profilewidget.h"

#include "profilepainter.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QStyleOption>
ProfileWidget::ProfileWidget(QWidget *parent) :
    QWidget(parent), _profilePainter(new ProfilePainter(this)), _courseIndex(-1)
{
}

QSize ProfileWidget::sizeHint() const
{
    return QSize(300, 100);
}

void ProfileWidget::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    _courseIndex = -1;
    repaint();
}

void ProfileWidget::setCourseIndex(const int courseIndex)
{
    _courseIndex = courseIndex;
    repaint();
}

void ProfileWidget::paintEvent(QPaintEvent *paintEvent)
{
    QPainter painter(this);

    QPixmap profilePixmap;
    if (_courseIndex >= 0) {
        const Course& course = _currentRlv.courses()[_courseIndex];
        profilePixmap = _profilePainter->paintProfileWithHighLight(_currentRlv, course.start(), course.end(),
                                                                   this->rect());
    } else {
        profilePixmap = _profilePainter->paintProfile(_currentRlv, this->rect());
    }
    painter.drawPixmap(rect(), profilePixmap);
    QWidget::paintEvent(paintEvent);
}

