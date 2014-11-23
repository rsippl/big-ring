#ifndef CLOCKGRAPHICSITEM_H
#define CLOCKGRAPHICSITEM_H

#include <QObject>
#include <QtCore/QTime>
#include <QtGui/QFont>
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
    void setTime(QTime time);
signals:

public slots:

private:
    QTime _time;
    QFont _font;

    int _textWidth;
    int _textHeight;
};

#endif // CLOCKGRAPHICSITEM_H
