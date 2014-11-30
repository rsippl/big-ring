#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QObject>
#include <QtWidgets/QGraphicsItem>

#include "reallifevideo.h"
#include "simulation.h"

class ProfileItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit ProfileItem(Simulation& simulation, QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
signals:

public slots:
    void setRlv(const RealLifeVideo& rlv);
    void setSize(const QSize &size);

private:
    QPainterPath drawProfile();
    qreal distanceToX(float distance) const;
    qreal altitudeToY(float altitudeAboveMinimum, float altitudeDiff) const;

    QSize _size;
    QRect _internalRect;
    RealLifeVideo _rlv;
    Simulation& _simulation;
    QPainterPath _profile;
};

#endif // PROFILEITEM_H
