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
#ifndef VIDEOITEMDELEGATE_H
#define VIDEOITEMDELEGATE_H
#include <QtWidgets/QAbstractItemDelegate>
#include <QtWidgets/QLabel>

class ProfilePainter;
class RealLifeVideo;
class VideoItemDelegate: public QAbstractItemDelegate
{
    Q_OBJECT
public:
    VideoItemDelegate(QObject* parent);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem&, const QModelIndex&) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintProfile(QPainter* painter, QRect& rect, RealLifeVideo& rlv) const;
    QPixmap drawProfilePixmap(QRect &rect, RealLifeVideo& rlv) const;

    qreal distanceToX(const QRect& rect, const RealLifeVideo& rlv, float distance) const;
    float xToDistance(const QRect& rect, const RealLifeVideo& rlv, int x) const;
    int altitudeToHeight(const QRect& rect, float altitudeAboveMinimum, float altitudeDiff) const;

    ProfilePainter* _profilePainter;
};

#endif // VIDEOITEMDELEGATE_H
