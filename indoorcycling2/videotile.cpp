#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsTextItem>
#include <QtGui/QPainter>
#include "thumbnailer.h"

namespace
{
const int HEIGHT = 200;
}

VideoTile::VideoTile(const RealLifeVideo rlv, QGraphicsItem *parent) :
    QGraphicsWidget(parent), _rlv(rlv), _thumbnailer(new Thumbnailer(this)), _selected(false)
{
    setAcceptHoverEvents(true);
    setGeometry(0, 0, 100, HEIGHT);
    Thumbnailer thumbnailer;
    qDebug() << "cache file: " << thumbnailer.cacheFilePathFor(rlv);

    _thumbnailItem = addThumbnail();
    _thumbnailItem->setPos(5, 5);
    QGraphicsItem* flagItem = addFlag();
    if (flagItem) {
        flagItem->setPos(mapFromItem(_thumbnailItem, _thumbnailItem->boundingRect().topLeft()));
    }

    QFont font;
    font.setPointSizeF(24.0);
    QGraphicsTextItem* titleItem = new QGraphicsTextItem(this);
    titleItem->setFont(font);
    titleItem->setPlainText(_rlv.name());
    titleItem->setPos(_thumbnailItem->boundingRect().topRight());

    QFont font2;
    font2.setPointSizeF(16.0);
    QGraphicsTextItem* distanceItem = new QGraphicsTextItem(this);
    distanceItem->setFont(font2);
    distanceItem->setPlainText(QString("%1 m").arg(_rlv.totalDistance()));
    distanceItem->setPos(_thumbnailItem->boundingRect().right() + 10, titleItem->boundingRect().bottom());
}

QGraphicsItem* VideoTile::addFlag() {
    QString name = _rlv.name();
    if (name[2] == '_') {
        QString alpha2Code = name.left(2).toLower();
        QString filename = QString("/home/ibooij/Downloads/icons/png/%1.png").arg(alpha2Code);
        QPixmap flagPixmap = QPixmap(filename).scaledToHeight(20, Qt::SmoothTransformation);

        qDebug() << filename << flagPixmap.isNull();
        QGraphicsPixmapItem* flagItem = new QGraphicsPixmapItem(flagPixmap, this);
        return flagItem;
    }
    return nullptr;
}

QGraphicsPixmapItem* VideoTile::addThumbnail()
{
    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, this, &VideoTile::thumbnailUpdated);
    QPixmap thumbnail = _thumbnailer->thumbnailFor(_rlv);
    QPixmap scaled = thumbnail.scaledToHeight(.9 * HEIGHT);
    return new QGraphicsPixmapItem(scaled, this);
}

void VideoTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush background = _selected ? Qt::lightGray : Qt::darkGray;
    painter->fillRect(boundingRect(), background);
    QGraphicsWidget::paint(painter, option, widget);

}

void VideoTile::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit selected(_rlv);
    event->accept();
}

void VideoTile::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    _selected = true;
    update();
}

void VideoTile::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    _selected = false;
    update();
}


void VideoTile::thumbnailUpdated(QPixmap updatedPixmap)
{
    _thumbnailItem->setPixmap(updatedPixmap.scaledToHeight(.9 * HEIGHT));
}
