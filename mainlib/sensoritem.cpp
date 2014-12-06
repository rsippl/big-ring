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
    _textItem->setPlainText(QString("%1").arg(exampleValue.toString()));
    _textItem->setPos(10, 5);

    QFont unitFont = QFont("Liberation Mono");
    unitFont.setBold(true);
    unitFont.setPointSize(18);
    _unitItem = new QGraphicsTextItem(this);
    _unitItem->setFont(unitFont);
    _unitItem->setDefaultTextColor(Qt::white);
    _unitItem->setOpacity(0.6);

    _unitItem->setPos(_textItem->boundingRect().width() + 5, _textItem->boundingRect().bottom() - _unitItem->boundingRect().height() - 3);
    _unitItem->setPlainText(_unitString);
}

QRectF SensorItem::boundingRect() const
{
    return QRectF(0, 0, _textItem->boundingRect().width() + 5 + _unitItem->boundingRect().width() + 20, _textItem->boundingRect().height());
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
    _textItem->setPlainText(QString("%1").arg(value.toString(), _fieldWidth));
    _textItem->show();
}


