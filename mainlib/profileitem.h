#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QtCore/QObject>
#include <QtWidgets/QGraphicsWidget>

#include "reallifevideo.h"
#include "simulation.h"

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
    QPixmap drawProfile();
    qreal distanceToX(float distance) const;
    float xToDistance(int x) const;
    int altitudeToHeight(float altitudeAboveMinimum, float altitudeDiff) const;

    QRect _internalRect;
    bool _dirty;
    RealLifeVideo _rlv;
    Simulation* _simulation;
    QPixmap _profilePixmap;
};

#endif // PROFILEITEM_H
