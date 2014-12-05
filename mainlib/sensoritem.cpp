#include "sensoritem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

SensorItem::SensorItem(const QString &unitString, const QVariant& exampleValue, QObject *parent) :
    QObject(parent), _unitString(unitString), _fieldWidth(exampleValue.toString().size())
{
    QFont font = QFont("Liberation Mono");
    font.setBold(true);
    font.setPointSize(36);

    _textItem = new QGraphicsTextItem(this);
    _textItem->setFont(font);
    _textItem->setDefaultTextColor(Qt::white);
    _textItem->setOpacity(0.6);

    _textItem->hide();
    _textItem->setPlainText(QString("%1 %2").arg(exampleValue.toString()).arg(_unitString));

    _textItem->setPos(10, 5);
}

QRectF SensorItem::boundingRect() const
{
    return QRectF(0, 0, _textItem->boundingRect().width() + 20, _textItem->boundingRect().height());
}

void SensorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65);
    painter->drawRoundedRect(boundingRect(), 5, 5);
}


void SensorItem::setValue(QVariant value)
{
    _textItem->setPlainText(QString("%1 %2").arg(value.toString(), _fieldWidth).arg(_unitString));
    _textItem->show();
}


