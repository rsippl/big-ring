#include "sensorvalueitem.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>

SensorValueItem::SensorValueItem(const QString& unit):
    QGraphicsItem(),
    _unit(unit), _font()
{
    _font.setBold(true);
    _font.setPointSize(48);
}

QRectF SensorValueItem::boundingRect() const
{
    return QRectF();
}

void SensorValueItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
//    qDebug() << "paint";
    painter->setFont(_font);
    painter->setPen(Qt::blue);

    painter->drawText(100, 100, "Dit is de text");
}
