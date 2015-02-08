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
#include "profilepainter.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>

namespace
{
const int MININUM_HUE = 0; // red
const int MAXIMUM_HUE = 240; // dark blue;
const float MINIMUM_SLOPE = -12.0;
const float MAXIMUM_SLOPE = 12.0;
const float INVERSE_SLOPE_RANGE = 1 / (MAXIMUM_SLOPE - MINIMUM_SLOPE);
}

ProfilePainter::ProfilePainter(QObject *parent) :
    QObject(parent)
{
}

QPixmap ProfilePainter::paintProfile(const RealLifeVideo &rlv, const QRect &rect) const
{
    QPixmap profilePixmap;
    if (rlv.isValid()) {
        QRect profileRect = QRect(QPoint(0,0),rect.size());

        if (!QPixmapCache::find(rlv.name(), &profilePixmap) || profilePixmap.size() != rect.size()) {
            qDebug() << "creating new profile pixmap for" << rlv.name();
            profilePixmap = drawProfilePixmap(profileRect, rlv);
            QPixmapCache::insert(rlv.name(), profilePixmap);
        }
    }
    return profilePixmap;
}

QPixmap ProfilePainter::paintProfileWithHighLight(const RealLifeVideo &rlv, qreal startDistance, qreal endDistance,
                                                  const QRect &rect) const
{
    QPixmap profilePixmap = paintProfile(rlv, rect);
    if (profilePixmap.isNull()) {
        return profilePixmap;
    }

    int startX = distanceToX(rect, rlv, startDistance);
    int endX = distanceToX(rect, rlv, endDistance);

    QPixmap copy(profilePixmap);
    QPainter painter(&copy);

    // TODO: get this color directly from the stylesheet. I don't know how to do this.
    QColor color("#3D7848");
    color.setAlphaF(0.5);
    painter.setBrush(color);
    painter.drawRect(startX, 0, endX - startX, rect.height());
    painter.end();

    return copy;
}

QPixmap ProfilePainter::drawProfilePixmap(QRect& rect, const RealLifeVideo& rlv) const
{
    if (rect.isEmpty()) {
        return QPixmap();
    }

    QPixmap pixmap(rect.size());
    QPainter painter(&pixmap);

    const QList<ProfileEntry>& profileEntries = rlv.profile().entries();
    const QPair<float,float> minAndMaxAltitude = findMinimumAndMaximumAltiude(rlv.profile().startAltitude(), profileEntries);

    float minimumAltitude = minAndMaxAltitude.first;
    float maximumAltitude = minAndMaxAltitude.second;

    float altitudeDiff = maximumAltitude - minimumAltitude;
    painter.setBrush(Qt::gray);
    painter.drawRect(rect);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    for(int x = 0; x < rect.width(); x += 1) {
        float distance = xToDistance(rect, rlv, x);
        float altitude = rlv.profile().altitudeForDistance(distance);
        painter.setBrush(colorForSlope(rlv.profile().slopeForDistance(distance)));

        int y = altitudeToHeight(rect, altitude - minimumAltitude, altitudeDiff);
        QRect  box(x, rect.bottom() - y, 1, rect.bottom());
        painter.drawRect(box);
    }
    painter.end();
    return pixmap;
}



qreal ProfilePainter::distanceToX(const QRect& rect, const RealLifeVideo& rlv, float distance) const
{
    return (distance / rlv.totalDistance()) * rect.width() + rect.left();
}

float ProfilePainter::xToDistance(const QRect& rect, const RealLifeVideo& rlv, int x) const
{
    float relative = x * 1.0 / rect.width();
    return rlv.totalDistance() * relative;
}

int ProfilePainter::altitudeToHeight(const QRect& rect, float altitudeAboveMinimum, float altitudeDiff) const
{
    return static_cast<int>(((altitudeAboveMinimum) / altitudeDiff) * rect.height() * .9);
}

QPair<float,float> ProfilePainter::findMinimumAndMaximumAltiude(const float startAltitude, const QList<ProfileEntry>& profileEntries) const
{
    float minimumAltitude = std::numeric_limits<float>::max();
    float maximumAltitude = std::numeric_limits<float>::min();
    foreach (const ProfileEntry& entry, profileEntries) {
        minimumAltitude = qMin(minimumAltitude, entry.altitude() + startAltitude);
        maximumAltitude = qMax(maximumAltitude, entry.altitude() + startAltitude);
    }
    return qMakePair(minimumAltitude, maximumAltitude);
}

QColor ProfilePainter::colorForSlope(const float slope) const {
    const float boundedSlope = qBound(MINIMUM_SLOPE, slope, MAXIMUM_SLOPE);
    /* 0 is MINIMUM_SLOPE or lower, 1 = MAXIMUM_SLOPE or higher*/
    const float normalizedSlope = (boundedSlope - MINIMUM_SLOPE) * INVERSE_SLOPE_RANGE;

    return QColor::fromHsv(240 - (normalizedSlope * 240), 255, 255);
}

