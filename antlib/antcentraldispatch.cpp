/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "antcentraldispatch.h"

#include "antchannelhandler.h"
#include "antdevicefinder.h"
#include "antheartratechannelhandler.h"
#include "antmessage2.h"
#include "antmessagegatherer.h"
#include "antpowerchannelhandler.h"
#include "antspeedandcadencechannelhandler.h"

#include <QtCore/QtDebug>

namespace {
const int INITIALIZATION_TIMEOUT = 3000; // ms

// According to the ANT specification, we should wait at least 500ms after sending
// the System Reset message. To be on the safe side, we'll wait 600ms.
const int ANT_RESET_SYSTEM_TIMEOUT = 600; // ms.

// ANT+ Network
const int ANT_PLUS_NETWORK_NUMBER = 1;
// ANT+ Network Key
const std::array<quint8,8> ANT_PLUS_NETWORK_KEY = { 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45 };
}
namespace indoorcycling {

AntCentralDispatch::AntCentralDispatch(QObject *parent) :
    QObject(parent), _initialized(false), _antMessageGatherer(new AntMessageGatherer(this)), _initializationTimer(new QTimer(this))
{
    connect(_antMessageGatherer, &AntMessageGatherer::antMessageReceived, this,
            &AntCentralDispatch::messageFromAntUsbStick);
    _initializationTimer->setInterval(INITIALIZATION_TIMEOUT);
    _initializationTimer->setSingleShot(true);
    connect(_initializationTimer, &QTimer::timeout, _initializationTimer, [this]() {
        emit initializationFinished(false);
    });
}

bool AntCentralDispatch::antUsbStickPresent() const
{
    return _antUsbStick.get();
}

bool AntCentralDispatch::initialized() const
{
    return _initialized;
}

bool AntCentralDispatch::searchForSensorType(AntSensorType channelType)
{
    return searchForSensor(channelType, 0);
}

bool AntCentralDispatch::searchForSensor(AntSensorType channelType, int deviceNumber)
{
    int channelNumber;
    // search for first non-occupied channel.
    for (channelNumber = 0; channelNumber < _antUsbStick->numberOfChannels(); ++channelNumber) {
        if (!_channels[channelNumber]) {
            break;
        }
    }
    if (channelNumber == _antUsbStick->numberOfChannels()) {
        return false;
    }
    // create and insert new channel
    std::unique_ptr<AntChannelHandler> channel = createChannel(channelNumber, channelType);
    if (deviceNumber != 0) {
        channel->setSensorDeviceNumber(deviceNumber);
    }

    connect(channel.get(), &AntChannelHandler::sensorFound, this, &AntCentralDispatch::setChannelInfo);
    connect(channel.get(), &AntChannelHandler::sensorValue, this, &AntCentralDispatch::handleSensorValue);
    connect(channel.get(), &AntChannelHandler::antMessageGenerated, this, &AntCentralDispatch::sendAntMessage);
    connect(channel.get(), &AntChannelHandler::searchTimeout, this, &AntCentralDispatch::searchTimedOut);

    //    connect(channel.data(), &ANTChannel::lostInfo, this, SLOT(lostInfo(int)));
    //    connect(channel, SIGNAL(staleInfo(int)), this, SLOT(staleInfo(int)));

//    connect(channel.data(), &ANTChannel::heartRateMeasured, this, &AntCentralDispatch::setHeartRate);
//    connect(channel, &ANTChannel::powerMeasured, this, &ANT::powerMeasured);
//    connect(channel, &ANTChannel::cadenceMeasured, this, &ANT::cadenceMeasured);
//    connect(channel, &ANTChannel::speedMeasured, this, &ANT::speedMeasured);

    _channels[channelNumber] = std::move(channel);

    channel->initialize();
    emit searchStarted(channelType, channelNumber);
    return true;
}

int AntCentralDispatch::currentHeartRate() const
{
    return _currentHeartRate;
}

void AntCentralDispatch::initialize()
{
    _initializationTimer->start();
    scanForAntUsbStick();
    if (!_antUsbStick) {
        emit initializationFinished(false);
        return;
    }
    _channels.resize(_antUsbStick->numberOfChannels());
    if (_antUsbStick->isReady()) {
        resetAntSystem();
    } else {
        connect(_antUsbStick.get(), &indoorcycling::AntDevice::deviceReady,
                this, &AntCentralDispatch::resetAntSystem);
    }
}

void AntCentralDispatch::messageFromAntUsbStick(const QByteArray &bytes)
{
    std::unique_ptr<AntMessage2> antMessage = AntMessage2::createMessageFromBytes(bytes);
    switch(antMessage->id()) {
    case AntMessage2::CHANNEL_EVENT:
        handleChannelEvent(*antMessage->asChannelEventMessage());
        break;
    case AntMessage2::BROADCAST_EVENT:
        handleBroadCastMessage(BroadCastMessage(*antMessage));
        break;
    case AntMessage2::SET_CHANNEL_ID:
        handleChannelIdMessage(SetChannelIdMessage(*antMessage));
    default:
        qDebug() << "unhandled ANT+ message" << antMessage->toString();
    }
}

void AntCentralDispatch::scanForAntUsbStick()
{
    AntDeviceFinder deviceFinder;
    _antUsbStick = deviceFinder.openAntDevice();
    if (_antUsbStick) {
        connect(_antUsbStick.get(), &indoorcycling::AntDevice::bytesRead, _antMessageGatherer,
                &AntMessageGatherer::submitBytes);
    }
    emit antUsbStickScanningFinished(_antUsbStick.get());
}

std::unique_ptr<AntChannelHandler> AntCentralDispatch::createChannel(int channelNumber, AntSensorType &sensorType)
{
    switch (sensorType) {
    case SENSOR_TYPE_CADENCE:
        return AntSpeedAndCadenceChannelHandler::createCadenceChannelHandler(channelNumber);
    case SENSOR_TYPE_HR:
        return std::unique_ptr<AntChannelHandler>(new AntHeartRateChannelHandler(channelNumber));
    case SENSOR_TYPE_POWER:
        return std::unique_ptr<AntChannelHandler>(new AntPowerChannelHandler(channelNumber));
    case SENSOR_TYPE_SPEED:
        return AntSpeedAndCadenceChannelHandler::createSpeedChannelHandler(channelNumber);
    case SENSOR_TYPE_SPEED_AND_CADENCE:
        return AntSpeedAndCadenceChannelHandler::createCombinedSpeedAndCadenceChannelHandler(channelNumber);
    default:
        qFatal("Unknown sensor type %d", sensorType);
        return std::unique_ptr<AntChannelHandler>();
    }
}

void AntCentralDispatch::resetAntSystem()
{
    sendAntMessage(AntMessage2::systemReset());
    QTimer::singleShot(ANT_RESET_SYSTEM_TIMEOUT, this, SLOT(sendNetworkKey()));
}

void AntCentralDispatch::sendNetworkKey()
{
    sendAntMessage(AntMessage2::setNetworkKey(ANT_PLUS_NETWORK_NUMBER, ANT_PLUS_NETWORK_KEY));
}

void AntCentralDispatch::setChannelInfo(int, AntSensorType sensorType, int sensorDeviceNumber)
{
    emit sensorFound(sensorType, sensorDeviceNumber);
}

void AntCentralDispatch::searchTimedOut(int channelNumber, AntSensorType)
{
    qDebug() << "Search timed out for channel" << channelNumber;
    emit sensorNotFound(_channels[channelNumber]->sensorType());
}

void AntCentralDispatch::handleSensorValue(const SensorValueType sensorValueType,
                                           const AntSensorType, const QVariant &sensorValue)
{
    if (sensorValueType == SENSOR_VALUE_HEARTRATE_BPM) {
        _currentHeartRate = sensorValue.toInt();
        emit heartRateMeasured(_currentHeartRate);
    }
}

void AntCentralDispatch::sendAntMessage(const AntMessage2 &message)
{
    Q_ASSERT_X(_antUsbStick.get(), "AntCentralDispatch::sendAntMessage", "usb stick should be present.");
    if (_antUsbStick) {
        qDebug() << "Sending ANT Message:" << message.toString();
        _antUsbStick->writeAntMessage(message);
    }
}

void AntCentralDispatch::handleChannelEvent(const AntChannelEventMessage &channelEventMessage)
{
    if (channelEventMessage.messageId() == AntMessage2::SET_NETWORK_KEY) {
        _initializationTimer->stop();
        _initialized = (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR);
        emit initializationFinished(_initialized);
    } else {
        bool sent = sendToChannel<AntChannelEventMessage>(channelEventMessage,
                                                          [](indoorcycling::AntChannelHandler& channel,
                                                          const AntChannelEventMessage& msg) {
            channel.handleChannelEvent(msg);
        });
        if (!sent) {
            qDebug() << "Unhandled channel event" << channelEventMessage.toString();
        }
    }
}

void AntCentralDispatch::handleBroadCastMessage(const BroadCastMessage &broadCastMessage)
{
    bool sent = sendToChannel<BroadCastMessage>(broadCastMessage, [](indoorcycling::AntChannelHandler& channel,
                                                const BroadCastMessage& msg) {
        channel.handleBroadcastEvent(msg);
    });
    if (!sent) {
        qDebug() << "Unhandled broad cast event for channel" << broadCastMessage.channelNumber();
    }
}

void AntCentralDispatch::handleChannelIdMessage(const SetChannelIdMessage &channelIdMessage)
{
    bool sent = sendToChannel<SetChannelIdMessage>(channelIdMessage,
                                                   [](indoorcycling::AntChannelHandler& channel,
                                                   const SetChannelIdMessage& msg) {
        channel.handleChannelIdEvent(msg);
    });
    if (!sent) {
        qDebug() << "Unhandled set channel id event for channel" << channelIdMessage.channelNumber();
    }
}

}

