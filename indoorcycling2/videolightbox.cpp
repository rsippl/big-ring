#include "videolightbox.h"
#include <QtCore/QtDebug>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsTextItem>
#include <QtGui/QPainter>
#include "thumbnailer.h"
#include "profileitem.h"

VideoLightBox::VideoLightBox(const RealLifeVideo& rlv, const QSizeF size) :
    QGraphicsWidget(), _rlv(rlv), _thumbnailer(new Thumbnailer(this)), _thumbnailItem(nullptr)
{
//    setPen(QPen(QBrush(Qt::black), 1));
//    setBrush(QBrush(Qt::darkGray));
//    setRect(0, 0, size.width(), size.height());
    setGeometry(0, 0, size.width(), size.height());
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
    titleItem->setDefaultTextColor(Qt::white);
    titleItem->setPlainText(_rlv.name());
    titleItem->setPos(_thumbnailItem->boundingRect().topRight());


    QFont font2;
    font2.setPointSizeF(16.0);
    QGraphicsTextItem* distanceItem = new QGraphicsTextItem(this);
    distanceItem->setFont(font2);
    distanceItem->setPlainText(QString("%1 m").arg(_rlv.totalDistance()));
    distanceItem->setPos(_thumbnailItem->boundingRect().right() + 10, titleItem->boundingRect().bottom());
}

QGraphicsItem* VideoLightBox::addFlag() {
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

QGraphicsPixmapItem* VideoLightBox::addThumbnail()
{
    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, this, &VideoLightBox::thumbnailUpdated);
    QPixmap thumbnail = _thumbnailer->thumbnailFor(_rlv);
    QPixmap scaled = thumbnail.scaledToHeight(.5 * rect().height());
    return new QGraphicsPixmapItem(scaled, this);
}

void VideoLightBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->fillRect(boundingRect(), QColor(0, 0, 0, 200));
    QPen pen(Qt::green);
    painter->setPen(pen);
    painter->drawRect(boundingRect());

}

void VideoLightBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    hide();
    event->accept();
}

void VideoLightBox::setSize(const QSizeF& size)
{
    setGeometry(0, 0, size.width(), size.height());
    qDebug() << "SIZE = " << size << isVisible() << boundingRect();
    prepareGeometryChange();
//    setRect(0, 0, size.width(), size.height());

}

void VideoLightBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget::resizeEvent(event);
    if (_thumbnailItem) {
        QPixmap thumbnail = _thumbnailer->thumbnailFor(_rlv);
        QPixmap scaled = thumbnail.scaledToHeight(.5 * event->newSize().height());
        _thumbnailItem->setPixmap(scaled);
    }
    event->accept();
}

void VideoLightBox::thumbnailUpdated(QPixmap updatedPixmap)
{
    _thumbnailItem->setPixmap(updatedPixmap);
}

