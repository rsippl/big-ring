#include "profileitem.h"

#include <QtCore/QPair>
#include <QtCore/QtDebug>
#include <QtGui/QPainter>

namespace
{
QPair<float,float> findMinimumAndMaximumAltiude(const QList<ProfileEntry>& profileEntries)
{
    float minimumAltitude = std::numeric_limits<float>::max();
    float maximumAltitude = std::numeric_limits<float>::min();
    foreach (const ProfileEntry& entry, profileEntries) {
        minimumAltitude = qMin(minimumAltitude, entry.altitude());
        maximumAltitude = qMax(maximumAltitude, entry.altitude());
    }
    return qMakePair(minimumAltitude, maximumAltitude);
}
}
ProfileItem::ProfileItem(Simulation& simulation, QObject *parent) :
    QObject(parent), _simulation(simulation)
{
    setOpacity(0.65);
}

QRectF ProfileItem::boundingRect() const
{
    return QRectF(0,0,_size.width(), _size.height());
}

void ProfileItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(QColor(Qt::green));
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    painter->setOpacity(0.5);
    painter->setPen(pen);
    painter->setBrush(Qt::lightGray);
    painter->drawRoundedRect(0, 0, _size.width(), _size.height(), 5, 5);
    if (_rlv.isValid()) {
        if (!_profile.isEmpty()) {
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillPath(_profile, QBrush(Qt::white));

            float distanceRatio = _simulation.cyclist().distance() / _rlv.totalDistance();
            QPen pen(QColor(Qt::red));
            pen.setStyle(Qt::SolidLine);
            pen.setWidth(2);
            painter->setPen(pen);
            painter->drawLine(distanceRatio * _internalRect.width(), _internalRect.top() + 3, distanceRatio * _internalRect.width(), _internalRect.bottom());
        }
    }
}

void ProfileItem::setSize(const QSize &size)
{
    qDebug() << "size =" << size.width() << size.height();
    _size = size;
    if (_rlv.isValid()) {
        _profile = drawProfile();
    }

    _internalRect = QRect(1, 1, _size.width() - 2, _size.height() - 2);
    update();
}

void ProfileItem::setRlv(const RealLifeVideo &rlv)
{
    _rlv = rlv;
    _profile = drawProfile();
    update();
}

QPainterPath ProfileItem::drawProfile()
{
    qDebug() << "creating profile path";

    const QList<ProfileEntry>& profileEntries = _rlv.profile().entries();
    const QPair<float,float> minAndMaxAltitude = findMinimumAndMaximumAltiude(profileEntries);

    float minimumAltitude = minAndMaxAltitude.first;
    float maximumAltitude = minAndMaxAltitude.second;

    float altitudeDiff = maximumAltitude - minimumAltitude;

    QPainterPath path(_internalRect.bottomLeft());
    foreach(const ProfileEntry& entry, profileEntries) {
        qreal x = distanceToX(entry.totalDistance());
        qreal y = altitudeToY(entry.altitude() - minimumAltitude, altitudeDiff);

        path.lineTo(x, y);
    }
    path.lineTo(_internalRect.bottomRight());
    path.lineTo(_internalRect.bottomLeft());

    return path;
}

qreal ProfileItem::distanceToX(float distance) const
{
    return (distance / _rlv.totalDistance()) * _internalRect.width() + _internalRect.left();
}

qreal ProfileItem::altitudeToY(float altitudeAboveMinimum, float altitudeDiff) const
{
    return _internalRect.height() - (((altitudeAboveMinimum) / altitudeDiff) * _internalRect.height());
}
