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
#include "antchannelhandler.h"

#include <QtCore/QMap>
#include <QtCore/QtDebug>

namespace {
const int SEARCH_TIMEOUT = 5; //seconds.
using indoorcycling::AntChannelHandler;
const QMap<AntChannelHandler::ChannelState,QString> CHANNEL_STATE_STRINGS(
{
            {AntChannelHandler::CHANNEL_CLOSED, "CHANNEL_CLOSED"},
            {AntChannelHandler::CHANNEL_ASSIGNED, "CHANNEL_ASSIGNED"},
            {AntChannelHandler::CHANNEL_ID_SET, "CHANNEL_ID_SET"},
            {AntChannelHandler::CHANNEL_FREQUENCY_SET, "CHANNEL_FREQUENCY_SET"},
            {AntChannelHandler::CHANNEL_PERIOD_SET, "CHANNEL_PERIOD_SET"},
            {AntChannelHandler::CHANNEL_TIMEOUT_SET, "CHANNEL_SEARCH_TIMEOUT_SET"},
            {AntChannelHandler::CHANNEL_OPENED, "CHANNEL_OPENED"},
            {AntChannelHandler::CHANNEL_SEARCHING, "CHANNEL_SEARCHING"},
            {AntChannelHandler::CHANNEL_TRACKING, "CHANNEL_TRACKING"},
            {AntChannelHandler::CHANNEL_UNASSIGNED, "CHANNEL_UNASSIGNED"}
});
}
namespace indoorcycling {
AntChannelHandler::AntChannelHandler(const int channelNumber, const AntSensorType sensorType,
                                     AntSportPeriod channelPeriod, QObject *parent) :
    QObject(parent), _channelNumber(channelNumber), _deviceNumber(0), _sensorType(sensorType),
    _channelPeriod(channelPeriod),_state(CHANNEL_CLOSED)
{
    // empty
}

AntChannelHandler::~AntChannelHandler()
{
    qDebug() << "Deleting AntChannelHandler for channel type" << _sensorType;
}

AntSensorType AntChannelHandler::sensorType() const
{
    return _sensorType;
}

int AntChannelHandler::sensorDeviceNumber() const
{
    return _deviceNumber;
}

void AntChannelHandler::setSensorDeviceNumber(int deviceNumber)
{
    _deviceNumber = deviceNumber;
}

void AntChannelHandler::initialize()
{
    emit antMessageGenerated(AntMessage2::assignChannel(_channelNumber));
    setState(CHANNEL_ASSIGNED);
}

void AntChannelHandler::handleChannelEvent(const AntChannelEventMessage &message)
{
    if (message.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR) {
        advanceState(message.messageId());
    } else if (message.messageCode() == AntChannelEventMessage::EVENT_CHANNEL_RX_SEARCH_TIMEOUT) {
        setState(CHANNEL_SEARCH_TIMEOUT);
        emit searchTimeout(_channelNumber, _sensorType);
        emit antMessageGenerated(AntMessage2::unassignChannel(_channelNumber));
        setState(CHANNEL_UNASSIGNED);
    } else if (message.messageCode() == AntChannelEventMessage::EVENT_CHANNEL_RX_FAIL) {
        qDebug() << "RX Failure on channel" << _channelNumber;
    } else if (message.messageCode() == AntChannelEventMessage::EVENT_CHANNEL_CLOSED) {
        qDebug() << "Channel closed by ANT+ stick.";
        if (_state != CHANNEL_UNASSIGNED) {
            emit antMessageGenerated(AntMessage2::unassignChannel(_channelNumber));
            setState(CHANNEL_UNASSIGNED);
        }
    } else {
        qDebug() << "unhandled message" << message.toString();
    }
}

void AntChannelHandler::handleBroadcastEvent(const BroadCastMessage &broadcastMessage)
{
    if (_state == CHANNEL_SEARCHING) {
        handleFirstBroadCastMessage(broadcastMessage);
    } else if (_state == CHANNEL_TRACKING) {
        handleBroadCastMessage(broadcastMessage);
    } else {
        qDebug() << "Did not expect a broad cast message in state" << CHANNEL_STATE_STRINGS[_state];
    }
}

void AntChannelHandler::handleChannelIdEvent(const SetChannelIdMessage &channelIdMessage)
{
    _deviceNumber = channelIdMessage.deviceNumber();
    setState(CHANNEL_TRACKING);
    emit sensorFound(_channelNumber, _sensorType, _deviceNumber);
    emit antMessageGenerated(AntMessage2::setInfiniteSearchTimeout(_channelNumber));
}

void AntChannelHandler::setState(AntChannelHandler::ChannelState state)
{
    _state = state;
    emit stateChanged(_state);
}

void AntChannelHandler::assertMessageId(const AntMessage2::AntMessageId expected, const quint8 actual)
{
    Q_ASSERT_X(expected == actual, "AntChannelHandler::assertMessageId",
               qPrintable(QString("expected message = %1, but was %2")
                          .arg(QString::number(expected, 16))
                          .arg(QString::number(actual, 16))));
    if (expected != actual) {
        QString message = QString("Expected messageid in state %1 was 0x%2, but it was 0x%3.")
                .arg(CHANNEL_STATE_STRINGS[_state]).arg(QString::number(expected, 16))
                .arg(QString::number(actual, 16));
        qWarning("%s", qPrintable(message));
    }
}

void AntChannelHandler::advanceState(const quint8 messageId)
{
    switch (_state) {
    case CHANNEL_ASSIGNED:
        assertMessageId(AntMessage2::ASSIGN_CHANNEL, messageId);
        emit antMessageGenerated(AntMessage2::setChannelId(_channelNumber,
                                                           _deviceNumber, _sensorType));
        setState(CHANNEL_ID_SET);
        break;
    case CHANNEL_ID_SET:
        assertMessageId(AntMessage2::SET_CHANNEL_ID, messageId);
        emit antMessageGenerated(AntMessage2::setChannelFrequency(_channelNumber));
        setState(CHANNEL_FREQUENCY_SET);
        break;
    case CHANNEL_FREQUENCY_SET:
        assertMessageId(AntMessage2::SET_CHANNEL_FREQUENCY, messageId);
        emit antMessageGenerated(AntMessage2::setChannelPeriod(_channelNumber, _channelPeriod));
        setState(CHANNEL_PERIOD_SET);
        break;
    case CHANNEL_PERIOD_SET:
        assertMessageId(AntMessage2::SET_CHANNEL_PERIOD, messageId);
        emit antMessageGenerated(AntMessage2::setSearchTimeout(_channelNumber, SEARCH_TIMEOUT));
        setState(CHANNEL_TIMEOUT_SET);
        break;
    case CHANNEL_TIMEOUT_SET:
        assertMessageId(AntMessage2::SET_SEARCH_TIMEOUT, messageId);
        emit antMessageGenerated(AntMessage2::openChannel(_channelNumber));
        setState(CHANNEL_OPENED);
        break;
    case CHANNEL_OPENED:
        assertMessageId(AntMessage2::OPEN_CHANNEL, messageId);
        setState(CHANNEL_SEARCHING);
        break;
    case CHANNEL_UNASSIGNED:
        assertMessageId(AntMessage2::UNASSIGN_CHANNEL, messageId);
        emit finished(_channelNumber);
        qDebug() << "Channel unassigned. Can be deleted";
        break;
    case CHANNEL_TRACKING:
        assertMessageId(AntMessage2::SET_SEARCH_TIMEOUT, messageId);
        qDebug() << "search timeout set after acquiring sensor.";
        break;
    default:
        qDebug() << "Unhandled state" << CHANNEL_STATE_STRINGS[_state];
    }
}

/**
 * When the first broad cast message from a sensor is received, we'll ask
 * the sensor for it's id.
 */
void AntChannelHandler::handleFirstBroadCastMessage(const BroadCastMessage&)
{
    qDebug() << QString("channel %1: First broadcast message received, requesting sensor id")
                .arg(_channelNumber);
    emit antMessageGenerated(AntMessage2::requestMessage(_channelNumber,
                                                         AntMessage2::SET_CHANNEL_ID));
    setState(CHANNEL_TRACKING);
}
}
