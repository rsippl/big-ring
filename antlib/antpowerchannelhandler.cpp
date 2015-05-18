#include "antpowerchannelhandler.h"
#include <QtCore/QTime>
#include <QtCore/QtDebug>
#include <QtCore/QVariant>
namespace indoorcycling
{
PowerMessage::PowerMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

bool PowerMessage::isPowerOnlyPage() const
{
    return dataPage() == POWER_ONLY_PAGE;
}

quint8 PowerMessage::eventCount() const
{
    return antMessage().contentByte(2);
}

quint8 PowerMessage::instantaneousCadence() const
{
    return antMessage().contentByte(4);
}

quint16 PowerMessage::accumulatedPower() const
{
    return antMessage().contentShort(5);
}

quint16 PowerMessage::instantaneousPower() const
{
    return antMessage().contentShort(7);
}

AntPowerChannelHandler::AntPowerChannelHandler(int channelNumber, QObject *parent) :
    AntChannelHandler(channelNumber, SENSOR_TYPE_POWER, ANT_SPORT_POWER_PERIOD, parent)
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
