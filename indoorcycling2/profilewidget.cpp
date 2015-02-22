/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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
                                                                   this->rect(), palette().highlight());
    } else {
        profilePixmap = _profilePainter->paintProfile(_currentRlv, this->rect());
    }
    painter.drawPixmap(rect(), profilePixmap);
    QWidget::paintEvent(paintEvent);
}

