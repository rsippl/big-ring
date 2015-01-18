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

#ifndef VIDEOLIGHTBOX_H
#define VIDEOLIGHTBOX_H

#include <QObject>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsWidget>
#include <QtWidgets/QPushButton>
#include "reallifevideo.h"

class ProfileItem;
class Thumbnailer;
class VideoLightBox : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit VideoLightBox(const RealLifeVideo& rlv);
    virtual ~VideoLightBox();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
public slots:
    virtual void setGeometry(const QRectF &rect) override;
signals:
    void playRequested();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
private slots:
    void thumbnailUpdated(QPixmap updatedPixmap);
private:
    QGraphicsItem *addFlag();
    QGraphicsPixmapItem *addThumbnail();
    const RealLifeVideo _rlv;
    QGraphicsPixmapItem* _thumbnailItem;
    QGraphicsItem* _flagItem;
    QGraphicsTextItem* _titleItem;
    Thumbnailer* _thumbnailer;
    ProfileItem* _profileItem;

    QGraphicsProxyWidget *_buttonProxyWidget;
    QPushButton* _playButton;
};


#endif // VIDEOLIGHTBOX_H
