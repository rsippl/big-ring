#ifndef CLOCKGRAPHICSITEM_H
#define CLOCKGRAPHICSITEM_H

#include <QObject>
#include <QtWidgets/QGraphicsItem>

class Simulation;

class ClockGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit ClockGraphicsItem(Simulation& simulation, QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    void setTime(QTime& time);
signals:

public slots:

private:
    QGraphicsTextItem* _textItem;
};

#endif // CLOCKGRAPHICSITEM_H
