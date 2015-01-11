#ifndef VIDEOTILE_H
#define VIDEOTILE_H

#include <QObject>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsWidget>
#include "reallifevideo.h"
#include "profileitem.h"

class Thumbnailer;
class VideoTile : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit VideoTile(RealLifeVideo rlv, QGraphicsItem *parent = 0);

    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
signals:
    void selected(RealLifeVideo& video);
private slots:
    void thumbnailUpdated(QPixmap updatedPixmap);
private:
    QGraphicsPixmapItem *addThumbnail();
    RealLifeVideo _rlv;
    QGraphicsPixmapItem* _thumbnailItem;
    Thumbnailer* _thumbnailer;
    ProfileItem* _profileItem;

    bool _selected;
};

#endif // VIDEOTILE_H
