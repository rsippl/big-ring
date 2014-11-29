#include "clockgraphicsitem.h"

#include <QtCore/QTime>
#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

#include "simulation.h"

ClockGraphicsItem::ClockGraphicsItem(Simulation& simulation, QObject *parent) :
    QObject(parent), _time(0, 0, 0)
{
    _font = QFont("Liberation Mono");

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

    QPen pen(Qt::red);
    pen.setWidth(3);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65);
    painter->drawRoundedRect(0, -10, _textWidth + 20, _textHeight + 12, 3, 3);
    painter->setPen(Qt::white);
    painter->setBrush(Qt::white);
    painter->drawText(10, _textHeight - 10, _time.toString("hh:mm:ss"));
}

void ClockGraphicsItem::setTime(QTime time)
{
    _time = time;
    update();
}
