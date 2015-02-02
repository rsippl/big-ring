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

#include "videolightbox.h"
#include <QtCore/QtDebug>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtSvg/QGraphicsSvgItem>
#include <QtSvg/QSvgRenderer>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QGraphicsColorizeEffect>
#include <QtWidgets/QGraphicsGridLayout>
#include <QtGui/QPainter>

#include "profileitem.h"
#include "thumbnailer.h"

VideoLightBox::VideoLightBox(const RealLifeVideo& rlv) :
    QGraphicsWidget(), _rlv(rlv), _thumbnailItem(nullptr), _thumbnailer(new Thumbnailer(this)),
    _profileItem(nullptr)
{
    _thumbnailItem = addThumbnail();
    _thumbnailItem->setPos(0, 0);

    _flagItem = addFlag();

    QFont font;
    font.setPointSizeF(24.0);

    _titleItem = new QGraphicsTextItem(this);
    _titleItem->setFont(font);
    _titleItem->setDefaultTextColor(Qt::white);
    _titleItem->setPlainText(_rlv.name());

    _playButton = new QPushButton("PLAY!");
    connect(_playButton, &QPushButton::clicked, _playButton, [=]() {
        qDebug() <<  "PLAY!";
        emit playRequested();
    });
    _buttonProxyWidget =  new QGraphicsProxyWidget(this);
    _buttonProxyWidget->setWidget(_playButton);

    QFont font2;
    font2.setPointSizeF(16.0);
    QGraphicsTextItem* distanceItem = new QGraphicsTextItem(this);
    distanceItem->setFont(font2);
    distanceItem->setDefaultTextColor(Qt::white);
    distanceItem->setPlainText(QString("%1 m").arg(_rlv.totalDistance()));

    _profileItem = new ProfileItem(this);
    _profileItem->setRlv(_rlv);
}

VideoLightBox::~VideoLightBox()
{
    qDebug() << "deleting lightbox for video" << _rlv.name();
}

QGraphicsItem *VideoLightBox::addFlag() {
    QString name = _rlv.name();
    if (name[2] == '_') {
        QString alpha2Code = name.left(2).toLower();
        QString filename = QString(":///images/flags/%1.svg").arg(alpha2Code);

        qDebug() << "flag for" << _rlv.name() << "is" << filename;
        QGraphicsSvgItem* flagItem = new QGraphicsSvgItem(filename, this);

        flagItem->setTransform(QTransform::fromScale(.35, .2));
        return flagItem;
    }
    return nullptr;
}

QGraphicsPixmapItem* VideoLightBox::addThumbnail()
{
    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, this, &VideoLightBox::thumbnailUpdated);
    QPixmap thumbnail = _thumbnailer->thumbnailFor(_rlv);
    QPixmap scaled = thumbnail.scaledToHeight(.5 * rect().height());
    return new QGraphicsPixmapItem(scaled, this);
}

void VideoLightBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush background = Qt::black;
    QPen pen(Qt::black);
    painter->setBrush(background);
    painter->setPen(pen);
    painter->drawRoundedRect(boundingRect(), 5, 5);
    QGraphicsWidget::paint(painter, option, widget);
}

void VideoLightBox::setGeometry(const QRectF &rect)
{
    prepareGeometryChange();
    QGraphicsWidget::setGeometry(rect);

    if (_thumbnailItem) {
        QPixmap thumbnail = _thumbnailer->thumbnailFor(_rlv);
        QPixmap scaled = thumbnail.scaledToHeight(.5 * rect.height());
        _thumbnailItem->setPixmap(scaled);
    }
    if (_profileItem) {
        _profileItem->setGeometry(QRectF(0, rect.height() - 200, boundingRect().width(), 200));
    }
    if (_titleItem) {
        _titleItem->setPos(boundingRect().width() - _titleItem->boundingRect().width(), boundingRect().top());
    }
    if (_flagItem) {
        _flagItem->setPos(boundingRect().topLeft());
    }
    if (_buttonProxyWidget) {
        _buttonProxyWidget->setPos(boundingRect().right() - _buttonProxyWidget->boundingRect().width(), boundingRect().top());
    }
}


void VideoLightBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    hide();
    event->accept();
}

void VideoLightBox::thumbnailUpdated(const RealLifeVideo&, QPixmap updatedPixmap)
{
    _thumbnailItem->setPixmap(updatedPixmap);
}

