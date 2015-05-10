#include "antpowerchannelhandler.h"
#include <QtCore/QTime>
#include <QtCore/QtDebug>
#include <QtCore/QVariant>
namespace indoorcycling
{
AntPowerChannelHandler::AntPowerChannelHandler(int channelNumber) :
    AntChannelHandler(channelNumber, SENSOR_TYPE_POWER, ANT_SPORT_POWER_PERIOD)
{
}

void AntPowerChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    PowerMessage powerMessage(message.antMessage());
    if (powerMessage.isPowerOnlyPage()) {
        if (!_lastPowerMessage.isNull()) {
            if (_lastPowerMessage.eventCount() != powerMessage.eventCount()) {
                emit sensorValue(SENSOR_VALUE_POWER_WATT, SENSOR_TYPE_POWER,
                                 QVariant::fromValue(powerMessage.instantaneousPower()));
                emit sensorValue(SENSOR_VALUE_CADENCE_RPM, SENSOR_TYPE_POWER,
                                 QVariant::fromValue(powerMessage.instantaneousCadence()));
                qDebug() << QTime::currentTime().toString()
                         << "power" << powerMessage.instantaneousPower() << "cadence" << powerMessage.instantaneousCadence();
            }
        }
        _lastPowerMessage = powerMessage;
    }
}

}
