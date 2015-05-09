#include "antheartratechannelhandler.h"
#include <QtCore/QtDebug>
#include <QtCore/QTime>
#include <QtCore/QVariant>
namespace {
const auto HR_CHANNEL_PERIOD = indoorcycling::AntChannelHandler::ANT_SPORT_HR_PERIOD;
}
indoorcycling::AntHeartRateChannelHandler::AntHeartRateChannelHandler(int channelNumber, QObject *parent):
    AntChannelHandler(channelNumber, SENSOR_TYPE_HR, HR_CHANNEL_PERIOD, parent)
{
    // empty
}

void indoorcycling::AntHeartRateChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    HeartRateMessage heartRateMessage(message.antMessage());
    if (_lastMessage.isNull() || heartRateMessage.measurementTime() != _lastMessage.measurementTime()) {
        qDebug() << QTime::currentTime().toString() << "HR:" << heartRateMessage.computedHeartRate();
        emit sensorValue(SENSOR_HEARTRATE_BPM, sensorType(),
                         QVariant::fromValue(heartRateMessage.computedHeartRate()));
    }
    _lastMessage = heartRateMessage;
}
