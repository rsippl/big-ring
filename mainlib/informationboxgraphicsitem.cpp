#include "informationboxgraphicsitem.h"

#include <QtGui/QFont>
#include <QtGui/QPainter>

InformationBoxGraphicsItem::InformationBoxGraphicsItem(QObject *parent) :
    QObject(parent)
{
    QFont font = QFont("Liberation Mono");
    font.setBold(true);
    font.setPointSize(30);

    _textItem = new QGraphicsTextItem(this);
    _textItem->setFont(font);

    _textItem->setDefaultTextColor(Qt::white);

    _textItem->setPlainText("Empty");
    _textItem->setPos(10, 5);
    _textItem->setOpacity(0.65);
}

QRectF InformationBoxGraphicsItem::boundingRect() const
{
    return QRectF(0, 0, _textItem->boundingRect().width() + 20, _textItem->boundingRect().height() + 10);
}

void InformationBoxGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    _textItem->setOpacity(opacity());
    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65 * opacity());
    painter->drawRoundedRect(0, -10, boundingRect().width(), boundingRect().height(), 3, 3);
}

void InformationBoxGraphicsItem::setText(const QString &text)
{
    _textItem->setHtml(text);
}
