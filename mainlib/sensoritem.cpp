#include "sensoritem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

SensorItem::SensorItem(const QString &unitString, QObject *parent) :
    QObject(parent), _unitString(unitString)
{
    _font.setBold(true);
    _font.setPointSize(24);

    QFontMetrics fm(_font);
    _textWidth = fm.width("000 " + _unitString);
    _textHeight = fm.height();
}

QRectF SensorItem::boundingRect() const
{
    return QRectF(0, 0, _textWidth + 20, _textHeight + 10);
}

void SensorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(_font);

    painter->setPen(Qt::gray);
    painter->setBrush(Qt::darkGray);
    painter->setOpacity(0.9);
    painter->drawRoundedRect(0, 0, _textWidth + 10, _textHeight + 5, 5, 5);
    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);
    painter->drawText(5, _textHeight, QString("%1 %2").arg(_value.toString()).arg(_unitString));
}

void SensorItem::setValue(QVariant value)
{
    _value = value;
}


