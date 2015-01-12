#include "profileitem.h"

#include <QtCore/QPair>
#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsDropShadowEffect>

namespace
{
QPair<float,float> findMinimumAndMaximumAltiude(const float startAltitude, const QList<ProfileEntry>& profileEntries)
{
    float minimumAltitude = std::numeric_limits<float>::max();
    float maximumAltitude = std::numeric_limits<float>::min();
    foreach (const ProfileEntry& entry, profileEntries) {
        minimumAltitude = qMin(minimumAltitude, entry.altitude() + startAltitude);
        maximumAltitude = qMax(maximumAltitude, entry.altitude() + startAltitude);
    }
    return qMakePair(minimumAltitude, maximumAltitude);
}

const int MININUM_HUE = 0; // red
const int MAXIMUM_HUE = 240; // dark blue;
const float MINIMUM_SLOPE = -12.0;
const float MAXIMUM_SLOPE = 12.0;
const float INVERSE_SLOPE_RANGE = 1 / (MAXIMUM_SLOPE - MINIMUM_SLOPE);

QColor colorForSlope(const float slope) {
    const float boundedSlope = qBound(MINIMUM_SLOPE, slope, MAXIMUM_SLOPE);
    /* 0 is MINIMUM_SLOPE or lower, 1 = MAXIMUM_SLOPE or higher*/
    const float normalizedSlope = (boundedSlope - MINIMUM_SLOPE) * INVERSE_SLOPE_RANGE;

    return QColor::fromHsv(240 - (normalizedSlope * 240), 255, 255);
}
}
ProfileItem::ProfileItem(QGraphicsItem *parent): ProfileItem(nullptr, parent)
{
    // empty
}

ProfileItem::ProfileItem(Simulation *simulation, QGraphicsItem *parent) :
    QGraphicsWidget(parent), _simulation(simulation), _dirty(false)
{
    setOpacity(0.75);
    QFont font("Sans");
    font.setBold(false);
    font.setPointSize(16);
}

void ProfileItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(QColor(Qt::green));
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    painter->setOpacity(0.5);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);
    painter->setBrush(Qt::lightGray);
    painter->drawRoundedRect(boundingRect(), 5, 5);

    if (_rlv.isValid()) {
        if (_dirty) {
            _profilePixmap = drawProfile();
            _dirty = false;
        }
        if (!_profilePixmap.isNull()) {
            painter->drawPixmap(_internalRect, _profilePixmap);

            if (_simulation) {
                float distanceRatio = _simulation->cyclist().distance() / _rlv.totalDistance();
                QBrush brush(Qt::black);
                QPen pen(QColor(Qt::black));
                painter->setOpacity(0.4);
                pen.setStyle(Qt::SolidLine);
                pen.setWidth(2);
                painter->setPen(pen);
                painter->setBrush(brush);
                painter->drawRect(_internalRect.left(), _internalRect.top(), distanceRatio * _internalRect.width(), _internalRect.bottom());
            }
        }
    }

}

void ProfileItem::setGeometry(const QRectF &rect)
{
    _dirty = true;
    prepareGeometryChange();
    QGraphicsWidget::setGeometry(rect);
    _internalRect = QRect(1, 1, rect.width() - 2, rect.height() - 2);
    _dirty = true;
}

void ProfileItem::setRlv(const RealLifeVideo &rlv)
{
    _rlv = rlv;
    _dirty = true;
    update();
}

QPixmap ProfileItem::drawProfile()
{
    if (_internalRect.isEmpty())  {
        return QPixmap();
    }
    QPixmap pixmap(_internalRect.size());
    QPainter painter(&pixmap);

    const QList<ProfileEntry>& profileEntries = _rlv.profile().entries();
    const QPair<float,float> minAndMaxAltitude = findMinimumAndMaximumAltiude(_rlv.profile().startAltitude(), profileEntries);

    float minimumAltitude = minAndMaxAltitude.first;
    float maximumAltitude = minAndMaxAltitude.second;

    float altitudeDiff = maximumAltitude - minimumAltitude;
    painter.setBrush(Qt::gray);
    painter.drawRect(_internalRect);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    for(int x = 0; x < _internalRect.width(); x += 1) {
        float distance = xToDistance(x);
        float altitude = _rlv.profile().altitudeForDistance(distance);
        painter.setBrush(colorForSlope(_rlv.profile().slopeForDistance(distance)));

        int y = altitudeToHeight(altitude - minimumAltitude, altitudeDiff);
        QRect  box(x, _internalRect.bottom() - y, 1, _internalRect.bottom());
        painter.drawRect(box);
    }
    return pixmap;
}


qreal ProfileItem::distanceToX(float distance) const
{
    return (distance / _rlv.totalDistance()) * _internalRect.width() + _internalRect.left();
}

float ProfileItem::xToDistance(int x) const
{
    float relative = x * 1.0 / _internalRect.width();
    return _rlv.totalDistance() * relative;
}

int ProfileItem::altitudeToHeight(float altitudeAboveMinimum, float altitudeDiff) const
{
    return static_cast<int>(((altitudeAboveMinimum) / altitudeDiff) * _internalRect.height() * .9);
}
