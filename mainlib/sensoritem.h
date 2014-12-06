#ifndef SENSORITEM_H
#define SENSORITEM_H

#include <QObject>
#include <QtGui/QFont>
#include <QtWidgets/QGraphicsItem>

#include "simulation.h"

class SensorItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit
    SensorItem(const QString& unitString, const QVariant &exampleValue = QVariant::fromValue(123), QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    void setValue(QVariant value);
private:
    const QString _unitString;
    int _fieldWidth;
    QGraphicsTextItem* _textItem;
    QGraphicsTextItem* _unitItem;
};

#endif // SENSORITEM_H
