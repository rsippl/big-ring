/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
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

#ifndef VIDEOTILE_H
#define VIDEOTILE_H

#include <QObject>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsWidget>
#include "reallifevideo.h"
#include "profileitem.h"

class Thumbnailer;
class VideoTile : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit VideoTile(RealLifeVideo rlv, QGraphicsItem *parent = 0);

    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
signals:
    void selected(RealLifeVideo& video);
private slots:
    void thumbnailUpdated(QPixmap updatedPixmap);
private:
    QGraphicsPixmapItem *addThumbnail();
    RealLifeVideo _rlv;
    QGraphicsPixmapItem* _thumbnailItem;
    Thumbnailer* _thumbnailer;
    ProfileItem* _profileItem;

    bool _selected;
};

#endif // VIDEOTILE_H
