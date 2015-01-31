#include "videoitemdelegate.h"
#include <QtGui/QPainter>
VideoItemDelegate::VideoItemDelegate(QObject *parent):
    QAbstractItemDelegate(parent)
{
}

void VideoItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setBrush(QBrush(Qt::darkGray));
    painter->drawRoundedRect(option.rect, 15, 15);
    QString name = index.model()->data(index).toString();
    painter->drawText(option.rect.x() + 10, option.rect.y() + 50, name);
}

QSize VideoItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(300, 100);
}


