#include "clockgraphicsitem.h"

#include <QtGui/QFont>
#include <QtCore/QTime>
#include <QtGui/QPainter>
#include <QtDebug>
#include "simulation.h"

ClockGraphicsItem::ClockGraphicsItem(Simulation& simulation, QObject *parent) :
    QObject(parent)
{
    QFont font = QFont("Liberation Mono");
    font.setBold(true);
    font.setPointSize(48);

    _textItem = new QGraphicsTextItem(this);
    _textItem->setFont(font);
    _textItem->setDefaultTextColor(Qt::white);

    _textItem->setPlainText("00:00:00");
    _textItem->hide();
    _textItem->setPos(10, 5);
    _textItem->setOpacity(0.65);

    connect(&simulation, &Simulation::runTimeChanged, this, &ClockGraphicsItem::setTime);
}

QRectF ClockGraphicsItem::boundingRect() const
{
    return QRectF(0, 0, _textItem->boundingRect().width() + 20, _textItem->boundingRect().height() + 10);
}

void ClockGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65);
    painter->drawRoundedRect(0, -10, boundingRect().width(), boundingRect().height(), 3, 3);
}

void ClockGraphicsItem::setTime(QTime& time)
{
    _textItem->setPlainText(time.toString("hh:mm:ss"));
    if (!_textItem->isVisible()) {
        _textItem->show();
    }
}
