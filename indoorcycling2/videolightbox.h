#ifndef VIDEOLIGHTBOX_H
#define VIDEOLIGHTBOX_H

#include <QObject>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsWidget>
#include <QtWidgets/QPushButton>
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
public slots:
    virtual void setGeometry(const QRectF &rect) override;
signals:
    void playRequested();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
private slots:
    void thumbnailUpdated(QPixmap updatedPixmap);
private:
    QGraphicsItem *addFlag();
    QGraphicsPixmapItem *addThumbnail();
    const RealLifeVideo _rlv;
    QGraphicsPixmapItem* _thumbnailItem;
    QGraphicsItem* _flagItem;
    QGraphicsTextItem* _titleItem;
    Thumbnailer* _thumbnailer;
    ProfileItem* _profileItem;

    QGraphicsProxyWidget *_buttonProxyWidget;
    QPushButton* _playButton;
};


#endif // VIDEOLIGHTBOX_H
