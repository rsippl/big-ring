#ifndef VIDEOLIGHTBOX_H
#define VIDEOLIGHTBOX_H

#include <QObject>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsWidget>
#include "reallifevideo.h"

class Thumbnailer;
class VideoLightBox : public QGraphicsWidget
{
    Q_OBJECT
//    Q_INTERFACES(QGraphicsWidget)
public:
    explicit VideoLightBox(const RealLifeVideo& rlv, const QSizeF size);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
signals:

public slots:
    void setSize(const QSizeF &size);

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
};


#endif // VIDEOLIGHTBOX_H
