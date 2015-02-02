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

#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsTextItem>
#include <QtGui/QPainter>
#include "thumbnailer.h"

namespace
{
const int HEIGHT = 200;
}

VideoTile::VideoTile(RealLifeVideo rlv, QGraphicsItem *parent) :
    QGraphicsWidget(parent), _rlv(rlv), _thumbnailer(new Thumbnailer(this)), _selected(false)
{
    setAcceptHoverEvents(true);

    _thumbnailItem = addThumbnail();
    _thumbnailItem->setPos(5, 5);

    QFont font;

    font.setPointSizeF(24.0);
    QGraphicsTextItem* titleItem = new QGraphicsTextItem(this);
    titleItem->setDefaultTextColor(Qt::white);
    titleItem->setFont(font);
    titleItem->setPlainText(_rlv.name());
    titleItem->setPos(_thumbnailItem->boundingRect().topRight());

    QFont font2;
    font2.setPointSizeF(16.0);
    QGraphicsTextItem* distanceItem = new QGraphicsTextItem(this);
    distanceItem->setDefaultTextColor(Qt::white);
    distanceItem->setFont(font2);
    distanceItem->setPlainText(QString("%1 m").arg(_rlv.totalDistance()));
    distanceItem->setPos(_thumbnailItem->boundingRect().right() + 10, titleItem->boundingRect().bottom());

    _profileItem = new ProfileItem(this);
    _profileItem->setRlv(_rlv);
    _profileItem->setGeometry(QRectF(0, 0, geometry().width(), 100));
}

void VideoTile::setGeometry(const QRectF &geom)
{
    prepareGeometryChange();
    QGraphicsWidget::setGeometry(geom);
    setPos(geom.topLeft());

    _profileItem->setGeometry(QRectF(0, rect().height() - 100, geom.width(), 100));
}

QSizeF VideoTile::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    switch (which) {
    case Qt::MinimumSize:
        return QSize(600, 300);
    case Qt::PreferredSize:
        return QSize(600, 300);
    case Qt::MaximumSize:
        return QSizeF(2000,600);
    default:
        break;
    }
    return constraint;
}

QRectF VideoTile::boundingRect() const
{
    return QRectF(QPointF(0, 0), geometry().size());
}

QGraphicsPixmapItem* VideoTile::addThumbnail()
{
    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, this, &VideoTile::thumbnailUpdated);
    QPixmap thumbnail = _thumbnailer->thumbnailFor(_rlv);
    QPixmap scaled = thumbnail.scaled(16.0 / 9 * HEIGHT, HEIGHT, Qt::KeepAspectRatio);
    return new QGraphicsPixmapItem(scaled, this);
}

void VideoTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush background = _selected ? Qt::lightGray : Qt::black;
    QPen pen(_selected ? Qt::lightGray : Qt::black);
    painter->setBrush(background);
    painter->setPen(pen);
    painter->drawRoundedRect(boundingRect(), 5, 5);
    QGraphicsWidget::paint(painter, option, widget);
}

void VideoTile::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit selected(_rlv);
    event->accept();
}

void VideoTile::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    _selected = true;
    update();
}

void VideoTile::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    _selected = false;
    update();
}


void VideoTile::thumbnailUpdated(const RealLifeVideo&, QPixmap updatedPixmap)
{
    QPixmap scaled = updatedPixmap.scaled(16.0 / 9 * HEIGHT, HEIGHT, Qt::KeepAspectRatio);
    _thumbnailItem->setPixmap(scaled);
}
