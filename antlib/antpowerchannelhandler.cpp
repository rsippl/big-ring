#include "antpowerchannelhandler.h"
#include <QtCore/QTime>
#include <QtCore/QtDebug>
#include <QtCore/QVariant>
namespace
{
const quint8 SENDING_POWER_CHANNEL_TRANSMISSION_TYPE = static_cast<quint8>(0x05);
}

namespace indoorcycling
{
PowerMessage::PowerMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

AntMessage2 PowerMessage::createPowerMessage(quint8 channel, quint8 eventCount, quint16 accumulatedPower, quint16 instantaneousPower)
{
    QByteArray content;
    content += channel;
    content += static_cast<quint8>(0x10);
    content += eventCount;
    content += 0xFF; // no pedal balance available
    content += 0xFF; // no cadence
    content += accumulatedPower & 0xFF;
    content += (accumulatedPower >> 8) & 0xFF;
    content += instantaneousPower & 0xFF;
    content += (instantaneousPower >> 8) & 0xFF;

    return AntMessage2(AntMessage2::AntMessageId::BROADCAST_EVENT, content);
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

AntPowerReceiveChannelHandler::AntPowerReceiveChannelHandler(int channelNumber, QObject *parent) :
    AntChannelHandler(channelNumber, SENSOR_TYPE_POWER, ANT_SPORT_POWER_PERIOD, parent)
{
}

void AntPowerReceiveChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
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

AntPowerTransmissionChannelHandler::AntPowerTransmissionChannelHandler(int channelNumber, QObject *parent):
    AntChannelHandler(channelNumber, SENSOR_TYPE_POWER, ANT_SPORT_POWER_PERIOD, parent), _updateTimer(new QTimer(this)), _eventCount(0u), _accumulatedPower(0u)
{
    setSensorDeviceNumber(1);
    _updateTimer->setTimerType(Qt::PreciseTimer);
    _updateTimer->setInterval(250);
    connect(_updateTimer, &QTimer::timeout, this, &AntPowerTransmissionChannelHandler::sendPowerUpdate);
}

void AntPowerTransmissionChannelHandler::setPower(quint16 power)
{
    _eventCount += 1;
    _instantaneousPower = power;
    _accumulatedPower += power;
}

void AntPowerTransmissionChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    qDebug() << "broadcast message received, page" << message.dataPage();
}

bool AntPowerTransmissionChannelHandler::isMasterNode() const
{
    return true;
}

quint8 AntPowerTransmissionChannelHandler::transmissionType() const
{
    return SENDING_POWER_CHANNEL_TRANSMISSION_TYPE;
}

void AntPowerTransmissionChannelHandler::channelOpened()
{
    qDebug() << "Power transmission channel opened";
    _updateTimer->start();
}

void AntPowerTransmissionChannelHandler::sendPowerUpdate()
{
    qDebug() << "sending power update" << _eventCount << _instantaneousPower << _accumulatedPower;
    emit antMessageGenerated(PowerMessage::createPowerMessage(channelNumber(), _eventCount,
                                                              _accumulatedPower, _instantaneousPower));
}

}
