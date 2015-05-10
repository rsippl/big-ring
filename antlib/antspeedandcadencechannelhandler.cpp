#include "antspeedandcadencechannelhandler.h"

#include <QtCore/QtDebug>
#include <QtCore/QTime>

namespace indoorcycling
{
std::unique_ptr<AntSpeedAndCadenceChannelHandler>
AntSpeedAndCadenceChannelHandler::createCombinedSpeedAndCadenceChannelHandler(int channelNumber)
{
    return std::unique_ptr<AntSpeedAndCadenceChannelHandler>(
                new AntSpeedAndCadenceChannelHandler(channelNumber, SENSOR_TYPE_SPEED_AND_CADENCE,
                                                     ANT_SPORT_SPEED_AND_CADENCE_PERIOD));
}

std::unique_ptr<AntSpeedAndCadenceChannelHandler> AntSpeedAndCadenceChannelHandler::createCadenceChannelHandler(int channelNumber)
{
    return std::unique_ptr<AntSpeedAndCadenceChannelHandler>(
                new AntSpeedAndCadenceChannelHandler(channelNumber, SENSOR_TYPE_CADENCE, ANT_SPORT_CADENCE_PERIOD));
}

std::unique_ptr<AntSpeedAndCadenceChannelHandler> AntSpeedAndCadenceChannelHandler::createSpeedChannelHandler(int channelNumber)
{
    return std::unique_ptr<AntSpeedAndCadenceChannelHandler>(
                new AntSpeedAndCadenceChannelHandler(channelNumber, SENSOR_TYPE_SPEED, ANT_SPORT_SPEED_PERIOD));
}

AntSpeedAndCadenceChannelHandler::AntSpeedAndCadenceChannelHandler(int channelNumber, AntSensorType sensorType, AntSportPeriod period):
    AntChannelHandler(channelNumber, sensorType, period)
{
    // empty
}

void AntSpeedAndCadenceChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    if (!_previousMessage.isNull()) {
        switch(sensorType()) {
        case SENSOR_TYPE_SPEED_AND_CADENCE:
            handleSpeedAndCadenceMessage(SpeedAndCadenceMessage(message.antMessage()));
            break;
        case SENSOR_TYPE_CADENCE:
            handleCadenceMessage(CadenceMessage(message.antMessage()));
            break;
        case SENSOR_TYPE_SPEED:
            handleSpeedMessage(SpeedMessage(message.antMessage()));
            break;
        default:
            qFatal("Wrong sensor type. Only Speed&Cadence, Cadence or Speed is possible!");
        }
    }
    _previousMessage = message;
}


void AntSpeedAndCadenceChannelHandler::handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage &message)
{
    SpeedAndCadenceMessage previousMessage(_previousMessage.antMessage());
    calculateCadence(previousMessage.cadenceEventTime(), previousMessage.pedalRevolutions(),
                     message.cadenceEventTime(), message.pedalRevolutions());
    calculateSpeed(previousMessage.speedEventTime(), previousMessage.wheelRevolutions(),
                   message.speedEventTime(), message.wheelRevolutions());
}

void AntSpeedAndCadenceChannelHandler::handleCadenceMessage(const CadenceMessage &message)
{
    CadenceMessage previousMessage(_previousMessage.antMessage());
    calculateCadence(previousMessage.cadenceEventTime(), previousMessage.pedalRevolutions(),
                     message.cadenceEventTime(), message.pedalRevolutions());
}

void AntSpeedAndCadenceChannelHandler::handleSpeedMessage(const SpeedMessage &message)
{
    SpeedMessage previousMessage(_previousMessage.antMessage());
    calculateSpeed(previousMessage.speedEventTime(), previousMessage.wheelRevolutions(),
                     message.speedEventTime(), message.wheelRevolutions());
}

void AntSpeedAndCadenceChannelHandler::calculateSpeed(const quint16 previousTime,
                                                          const quint16 previousWheelRevolutions,
                                                          const quint16 currentTime,
                                                          const quint16 currentWheelRevolutions)
{
    quint16 time = currentTime - previousTime;
    quint16 revolutions = currentWheelRevolutions - previousWheelRevolutions;
    if (time) {
        float rpm = 1024*60*revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "wheel speed" << rpm << "=" << ((rpm / 60) * 2.070) * 3.6 << "km/h";
        emit sensorValue(SENSOR_VALUE_WHEEL_SPEED_RPM, sensorType(), QVariant::fromValue(rpm));
    }
}

void AntSpeedAndCadenceChannelHandler::calculateCadence(const quint16 previousTime,
                                                            const quint16 previousPedalRevolutions,
                                                            const quint16 currentTime,
                                                            const quint16 currentPedalRevolutions)
{
    quint16 time = currentTime - previousTime;
    quint16 revolutions = currentPedalRevolutions - previousPedalRevolutions;
    if (time) {
        float cadence = 1024 * 60 * revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "cadence" << cadence;
        emit sensorValue(SENSOR_VALUE_CADENCE_RPM, sensorType(), QVariant::fromValue(cadence));
    }
}
}
