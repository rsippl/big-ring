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

#ifndef VIDEOTILEVIEW_H
#define VIDEOTILEVIEW_H

#include <QtCore/QList>
#include <QtWidgets/QGraphicsGridLayout>
#include <QtWidgets/QGraphicsView>
#include <QtCore/QScopedPointer>
#include <QtCore/QScopedPointerDeleteLater>

#include "reallifevideo.h"
#include "videotile.h"
#include "videolightbox.h"

class VideoTileView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit VideoTileView(QWidget *parent = 0);

signals:

public slots:
    void rlvsLoaded(RealLifeVideoList& rlvs);

signals:
    void startRlv(RealLifeVideo& rlv);

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *) override;
    virtual void scrollContentsBy(int dx, int dy) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
private:
    void placeTiles();
    void showSelected(RealLifeVideo &rlv);
    void removeSelectedVideoLightBox();

    QGraphicsWidget* _mainWidget;
    QGraphicsGridLayout* _layout;
    QList<VideoTile*> _items;
    QScopedPointer<VideoLightBox,QScopedPointerObjectDeleteLater<VideoLightBox>> _selectedVideoLightBox;
    RealLifeVideo _selectedRlv;
};

#endif // VIDEOTILEVIEW_H
