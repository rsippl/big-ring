#ifndef PROFILEPAINTER_H
#define PROFILEPAINTER_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include "reallifevideo.h"

class ProfilePainter : public QObject
{
    Q_OBJECT
public:
    explicit ProfilePainter(QObject *parent = 0);

    QPixmap paintProfile(const RealLifeVideo& rlv, const QRect& rect) const;
private:
    QPixmap drawProfilePixmap(QRect& rect, const RealLifeVideo& rlv) const;
    qreal distanceToX(const QRect& rect, const RealLifeVideo& rlv, float distance) const;
    float xToDistance(const QRect& rect, const RealLifeVideo& rlv, int x) const;
    int altitudeToHeight(const QRect& rect, float altitudeAboveMinimum, float altitudeDiff) const;
    QPair<float,float> findMinimumAndMaximumAltiude(const float startAltitude, const QList<ProfileEntry>& profileEntries) const;
    QColor colorForSlope(const float slope) const;
};

#endif // PROFILEPAINTER_H
