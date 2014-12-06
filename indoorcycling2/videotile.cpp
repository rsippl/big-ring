#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtGui/QPainter>
VideoTile::VideoTile(const RealLifeVideo rlv, QObject *parent) :
    QObject(parent), QGraphicsRectItem(), _rlv(rlv)
{
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(Qt::green));
    setRect(0, 0, 100, 100);
}

void VideoTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsView* view = scene()->views().first();
    setRect(0, 0, view->width() / 2 - 10, view->width() / 2 - 10);
    QGraphicsRectItem::paint(painter, option, widget);
    painter->drawText(0, 100, _rlv.name());
}
