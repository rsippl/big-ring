#include "sensoritem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

SensorItem::SensorItem(const QString &unitString, QObject *parent) :
    QObject(parent), _unitString(unitString)
{
    _font = QFont("Liberation Mono");
    _font.setBold(true);
    _font.setPointSize(24);

    QRect textRect = QFontMetrics(_font).boundingRect("000 " + _unitString);
    _rect = QRect(0, 0, textRect.width() + 20, textRect.height() + 10);
}

QRectF SensorItem::boundingRect() const
{
    return QRectF(_rect);
}

void SensorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(_font);

    QPen pen(Qt::red);
    pen.setWidth(3);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65);
    painter->drawRoundedRect(_rect, 5, 5);

    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);
    painter->drawText(0, _rect.height() - 10, QString("%1 %2").arg(_value.toString()).arg(_unitString));
}

void SensorItem::setValue(QVariant value)
{
    _value = value;
    update();
}


