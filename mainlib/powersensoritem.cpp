#include "powersensoritem.h"

#include <QtCore/QtDebug>

PowerSensorItem::PowerSensorItem(QObject *parent):
    SensorItem(QuantityPrinter::Quantity::Power, parent)
{
    _averagingTime = 3000; // milliseconds
}

void PowerSensorItem::setValue(const QVariant &powerValue)
{
    const int watts = powerValue.toInt();
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime oldestValidTimestamp = now.addMSecs(-_averagingTime);

    _powerValues.push_back(std::make_pair(now, watts));

    // search for the first valid
    auto firstValidValueIt = std::find_if(_powerValues.begin(), _powerValues.end(), [oldestValidTimestamp](const std::pair<QDateTime,int> &powerValue) {
        return powerValue.first >= oldestValidTimestamp;
    });
    _powerValues.erase(_powerValues.begin(), firstValidValueIt);

    int accumulatedPower = 0;
    for (auto value: _powerValues) {
        accumulatedPower += value.second;
    }
    int averagePower = accumulatedPower / _powerValues.size();

    SensorItem::setValue(QVariant::fromValue(averagePower));
}
