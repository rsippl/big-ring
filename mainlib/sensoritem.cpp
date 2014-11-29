#include "sensoritem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

SensorItem::SensorItem(const QString &unitString, const QVariant& exampleValue, QObject *parent) :
    QObject(parent), _unitString(unitString)
{
    _font = QFont("Liberation Mono");
    _font.setBold(true);
    _font.setPointSize(36);

    _fieldWidth = exampleValue.toString().length();

    QRect textRect = QFontMetrics(_font).boundingRect(QString("%1 %2").arg(exampleValue.toString()).arg(_unitString));
    _rect = QRect(0, 0, textRect.width() + 20, textRect.height() + 10);
}

QRectF SensorItem::boundingRect() const
{
    return QRectF(_rect);
}

void SensorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(_font);

    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65);
    painter->drawRoundedRect(_rect, 5, 5);

    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);

    painter->drawText(10, _rect.height() - 15, QString("%1 %2").arg(_value.toString(), _fieldWidth).arg(_unitString));
}

void SensorItem::setValue(QVariant value)
{
    _value = value;
    update();
}


