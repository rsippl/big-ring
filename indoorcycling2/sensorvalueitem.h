#ifndef SENSORVALUEITEM_H
#define SENSORVALUEITEM_H

#include <QtGui/QFont>
#include <QtWidgets/QGraphicsItem>
#include <QObject>

class SensorValueItem : public QGraphicsItem
{
public:
    explicit SensorValueItem(const QString& unit);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
signals:

public slots:

private:
    QString _value;
    QString _unit;
    QFont _font;
};

#endif // SENSORVALUEITEM_H
