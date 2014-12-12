#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsTextItem>
#include <QtGui/QPainter>


namespace
{
const int HEIGHT = 200;
}

VideoTile::VideoTile(const RealLifeVideo rlv, QObject *parent) :
    QObject(parent), QGraphicsRectItem(), _rlv(rlv)
{
    setAcceptHoverEvents(true);
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(Qt::darkGray));
    setRect(0, 0, 100, HEIGHT);


    QPixmap picture("/home/ibooij/Videos/out.jpg");
    QPixmap scaled = picture.scaledToHeight(.9 * HEIGHT);
    QGraphicsPixmapItem* pict = new QGraphicsPixmapItem(scaled, this);
    pict->setPos(5, 5);
    QGraphicsItem* flagItem = addFlag();
    if (flagItem) {
        flagItem->setPos(mapFromItem(pict, pict->boundingRect().topLeft()));
    }

    QFont font;
    font.setPointSizeF(24.0);
    QGraphicsTextItem* titleItem = new QGraphicsTextItem(this);
    titleItem->setFont(font);
    titleItem->setPlainText(_rlv.name());
    titleItem->setPos(pict->boundingRect().topRight());

    QFont font2;
    font2.setPointSizeF(16.0);
    QGraphicsTextItem* distanceItem = new QGraphicsTextItem(this);
    distanceItem->setFont(font2);
    distanceItem->setPlainText(QString("%1 m").arg(_rlv.totalDistance()));
    distanceItem->setPos(pict->boundingRect().right() + 10, titleItem->boundingRect().bottom());
}

QGraphicsItem* VideoTile::addFlag() {
    QString name = _rlv.name();
    if (name[2] == '_') {
        QString alpha2Code = name.left(2).toLower();
        QString filename = QString("/home/ibooij/Downloads/icons/png/%1.png").arg(alpha2Code);
        QPixmap flagPixmap = QPixmap(filename).scaledToHeight(20, Qt::SmoothTransformation);

        qDebug() << filename << flagPixmap.isNull();
        QGraphicsPixmapItem* flagItem = new QGraphicsPixmapItem(flagPixmap, this);
        qDebug() << "country found" << alpha2Code << flagItem->boundingRect().width();
        return flagItem;
    }
    return nullptr;
}

void VideoTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //    QGraphicsView* view = scene()->views().first();
    //    setRect(0, 0, view->width() / 2 - 10, view->width() / 2 - 10);
    QGraphicsRectItem::paint(painter, option, widget);
}

void VideoTile::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "mouse event in" << _rlv.name();
    event->accept();
}

void VideoTile::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    setBrush(QBrush(Qt::lightGray));
}

void VideoTile::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    setBrush(QBrush(Qt::darkGray));
}

void VideoTile::setWidth(int width)
{
    prepareGeometryChange();
    setRect(0, 0, width, HEIGHT);
}
