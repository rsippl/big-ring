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
    void setSize(const QSize &size);
signals:

public slots:
    void setRlv(const RealLifeVideo& rlv);

private:
    QPainterPath drawProfile();
    qreal distanceToX(float distance, int xMargin) const;
    qreal altitudeToY(float altitudeAboveMinimum, float altitudeDiff, int pathHeight) const;

    QSize _size;
    RealLifeVideo _rlv;
    Simulation& _simulation;
    QPainterPath _profile;
};

#endif // PROFILEITEM_H
