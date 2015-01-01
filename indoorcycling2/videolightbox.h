#ifndef VIDEOLIGHTBOX_H
#define VIDEOLIGHTBOX_H

#include <QObject>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsWidget>
#include "reallifevideo.h"

class ProfileItem;
class Thumbnailer;
class VideoLightBox : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit VideoLightBox(const RealLifeVideo& rlv);
    virtual ~VideoLightBox();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
private slots:
    void thumbnailUpdated(QPixmap updatedPixmap);
private:
    QGraphicsItem *addFlag();
    QGraphicsPixmapItem *addThumbnail();
    const RealLifeVideo _rlv;
    QGraphicsPixmapItem* _thumbnailItem;
    Thumbnailer* _thumbnailer;
    ProfileItem* _profileItem;
};


#endif // VIDEOLIGHTBOX_H
