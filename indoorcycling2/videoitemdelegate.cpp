#include "videoitemdelegate.h"
#include "videolistmodel.h"
#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>

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

VideoItemDelegate::VideoItemDelegate(QObject *parent):
    QAbstractItemDelegate(parent)
{
//    QPixmapCache::setCacheLimit(102400);
}

void VideoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QBrush rectBrush = (option.state & QStyle::State_Selected) ? option.palette.highlight() :option.palette.window();
    QPen pen(((option.state & QStyle::State_Selected) ? option.palette.highlightedText() : option.palette.text()).color());
    painter->setBrush(rectBrush);
    painter->drawRoundedRect(option.rect, 3, 3);
    QVariant rlvData = index.data(VideoDataRole);
    RealLifeVideo video = rlvData.value<RealLifeVideo>();

    QSize rectSize = option.rect.size();
    QRect profileRect(option.rect.topLeft(), QSize(rectSize).scaled(.9 * rectSize.width(), .9 * rectSize.height(), Qt::KeepAspectRatio));
    profileRect.moveCenter(option.rect.center());
    paintProfile(painter, profileRect, video);

    painter->setPen(pen);
    painter->drawText(option.rect, video.name());
}

QSize VideoItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(300, 100);
}

void VideoItemDelegate::paintProfile(QPainter *painter, QRect &rect, RealLifeVideo &rlv) const
{
    if (rlv.isValid()) {
        QRect profileRect = QRect(QPoint(0,0),rect.size());
        QPixmap profilePixmap;
        if (!QPixmapCache::find(rlv.name(), &profilePixmap) || profilePixmap.size() != rect.size()) {
            qDebug() << "creating new profile pixmap for" << rlv.name();
            profilePixmap = drawProfilePixmap(profileRect, rlv);
            QPixmapCache::insert(rlv.name(), profilePixmap);
        }
        if (!profilePixmap.isNull()) {
            painter->drawPixmap(rect, profilePixmap);
        }
    }
}

QPixmap VideoItemDelegate::drawProfilePixmap(QRect& rect, RealLifeVideo& rlv) const
{
    if (rect.isEmpty()) {
        return QPixmap();
    }

    QPixmap pixmap(rect.size());
    QPainter painter(&pixmap);

    const QList<ProfileEntry>& profileEntries = rlv.profile().entries();
    const QPair<float,float> minAndMaxAltitude = findMinimumAndMaximumAltiude(rlv.profile().startAltitude(), profileEntries);

    float minimumAltitude = minAndMaxAltitude.first;
    float maximumAltitude = minAndMaxAltitude.second;

    float altitudeDiff = maximumAltitude - minimumAltitude;
    painter.setBrush(Qt::gray);
    painter.drawRect(rect);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    for(int x = 0; x < rect.width(); x += 1) {
        float distance = xToDistance(rect, rlv, x);
        float altitude = rlv.profile().altitudeForDistance(distance);
        painter.setBrush(colorForSlope(rlv.profile().slopeForDistance(distance)));

        int y = altitudeToHeight(rect, altitude - minimumAltitude, altitudeDiff);
        QRect  box(x, rect.bottom() - y, 1, rect.bottom());
        painter.drawRect(box);
    }
    return pixmap;
}



qreal VideoItemDelegate::distanceToX(const QRect& rect, const RealLifeVideo& rlv, float distance) const
{
    return (distance / rlv.totalDistance()) * rect.width() + rect.left();
}

float VideoItemDelegate::xToDistance(const QRect& rect, const RealLifeVideo& rlv, int x) const
{
    float relative = x * 1.0 / rect.width();
    return rlv.totalDistance() * relative;
}

int VideoItemDelegate::altitudeToHeight(const QRect& rect, float altitudeAboveMinimum, float altitudeDiff) const
{
    return static_cast<int>(((altitudeAboveMinimum) / altitudeDiff) * rect.height() * .9);
}

