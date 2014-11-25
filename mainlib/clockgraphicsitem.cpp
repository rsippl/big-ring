#include "clockgraphicsitem.h"

#include <QtCore/QTime>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

#include "simulation.h"

ClockGraphicsItem::ClockGraphicsItem(Simulation& simulation, QObject *parent) :
    QObject(parent)
{
    _font.setBold(true);
    _font.setPointSize(48);

    QFontMetrics fm(_font);
    _textWidth = fm.width("88:88:88");
    _textHeight = fm.height();

    connect(&simulation, &Simulation::runTimeChanged, this, &ClockGraphicsItem::setTime);
}

QRectF ClockGraphicsItem::boundingRect() const
{
    return QRectF(0, 0, _textWidth + 20, _textHeight + 10);
}

void ClockGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(_font);

    painter->setPen(Qt::gray);
    painter->setBrush(Qt::gray);
    painter->setOpacity(0.8);
    painter->drawRoundedRect(0, 0, _textWidth + 20, _textHeight + 10, 15, 15);
    painter->setPen(Qt::green);
    painter->setBrush(Qt::white);
    painter->drawText(5, _textHeight, _time.toString("hh:mm:ss"));
}

void ClockGraphicsItem::setTime(QTime time)
{
    _time = time;
}
