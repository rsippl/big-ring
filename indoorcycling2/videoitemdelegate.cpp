#include "videoitemdelegate.h"
#include "videolistmodel.h"
#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>

#include "profilepainter.h"

VideoItemDelegate::VideoItemDelegate(QObject *parent):
    QAbstractItemDelegate(parent), _profilePainter(new ProfilePainter)
{
    // empty
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
    QRect profileRect(option.rect.topLeft(), QSize(rectSize).scaled(.9 * rectSize.width(), .9 * rectSize.height(),
                                                                    Qt::KeepAspectRatio));
    profileRect.moveCenter(option.rect.center());
    paintProfile(painter, profileRect, video);

    painter->setPen(pen);
    painter->drawText(option.rect, video.name());
}

QSize VideoItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(300, 100);
}

void VideoItemDelegate::paintProfile(QPainter *painter, QRect &rect, RealLifeVideo &rlv) const
{
    QPixmap profilePixmap = _profilePainter->paintProfile(rlv, rect);
    if (!profilePixmap.isNull()) {
        painter->drawPixmap(rect, profilePixmap);
    }
}
