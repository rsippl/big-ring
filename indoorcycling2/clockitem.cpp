#include "clockitem.h"

#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QPainter>

ClockItem::ClockItem(QObject *parent) :
    QObject(parent), QGraphicsItem()
{
    _font.setBold(true);
    _font.setPointSize(48);
    QTimer* timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, [this]() {
        update();
    });
}

QRectF ClockItem::boundingRect() const
{
    return QRectF();
}

void ClockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setFont(_font);
    painter->setPen(Qt::yellow);

    painter->drawText(100, 100, QTime::currentTime().toString("mm:ss:zzz"));
}
