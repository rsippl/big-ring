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

#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QtCore/QObject>
#include <QtWidgets/QGraphicsWidget>

#include "reallifevideo.h"
#include "simulation.h"

class ProfilePainter;

class ProfileItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit ProfileItem(QGraphicsItem *parent = 0);
    explicit ProfileItem(Simulation* simulation, QGraphicsItem *parent = 0);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void setGeometry(const QRectF &rect) override;
signals:

public slots:
    void setRlv(const RealLifeVideo& rlv);

private:
    ProfilePainter* _profilePainter;
    QRect _internalRect;
    bool _dirty;
    RealLifeVideo _rlv;
    Simulation* _simulation;
    QPixmap _profilePixmap;
};

#endif // PROFILEITEM_H
