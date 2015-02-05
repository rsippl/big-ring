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
#include "videoscreenshotlabel.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>

VideoScreenshotLabel::VideoScreenshotLabel(QWidget *parent) :
    QLabel(parent)
{
    // empty
}


void VideoScreenshotLabel::setPixmap(const QPixmap &pixmap)
{
    _videoScreenshot = pixmap;
    scaleAndPlacePixmap();
}

void VideoScreenshotLabel::resizeEvent(QResizeEvent *resizeEvent)
{
    scaleAndPlacePixmap();
    QLabel::resizeEvent(resizeEvent);
}

void VideoScreenshotLabel::scaleAndPlacePixmap()
{
    QPixmap scaled = _videoScreenshot.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QRect scaledRect = scaled.rect();
    scaledRect.moveCenter(rect().center());

    // paint scaled pixmap in the center of a new target pixmap
    QPixmap target(size());
    QPainter painter(&target);


    painter.fillRect(target.rect(), palette().background());
    painter.drawPixmap(scaledRect, scaled);

    QLabel::setPixmap(target);
}
