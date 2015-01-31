#ifndef VIDEOITEMDELEGATE_H
#define VIDEOITEMDELEGATE_H
#include <QtWidgets/QAbstractItemDelegate>
class VideoItemDelegate: public QAbstractItemDelegate
{
    Q_OBJECT
public:
    VideoItemDelegate(QObject* parent);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem&, const QModelIndex&) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // VIDEOITEMDELEGATE_H
