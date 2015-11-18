#ifndef POWERSENSORITEM_H
#define POWERSENSORITEM_H

#include "sensoritem.h"

#include <QtCore/QDateTime>
#include <deque>

class PowerSensorItem : public SensorItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit PowerSensorItem(QObject *parent = 0);
public slots:
    virtual void setValue(const QVariant &value) override;
private:
    int _averagingTime;
    std::deque<std::pair<QDateTime, int>> _powerValues;
};

#endif // POWERSENSORITEM_H
