#ifndef CLOCKITEM_H
#define CLOCKITEM_H

#include <QObject>
#include <QtGui/QFont>
#include <QtWidgets/QGraphicsItem>

class ClockItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit ClockItem(QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
signals:

public slots:

private:
    QFont _font;

};

#endif // CLOCKITEM_H
