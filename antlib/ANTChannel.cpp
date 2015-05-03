/*
 * Copyright (c) 2009 Mark Rages
 * Copyright (c) 2011 Mark Liversedge (liversedge@gmail.com)
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "antmessage2.h"
#include "ANTChannel.h"
#include <QDebug>
#include <QTime>

namespace {
const quint32 timeout_drop = 2000; // ms
const quint32 timeout_scan = 10000; // ms
const quint32 timeout_lost = 30; // s

const QMap<ANTChannel::ChannelState,QString> CHANNEL_STATE_STRINGS(
{
            {ANTChannel::CHANNEL_CLOSED, "CHANNEL_CLOSED"},
            {ANTChannel::CHANNEL_ASSIGNED, "CHANNEL_ASSIGNED"},
            {ANTChannel::CHANNEL_ID_SET, "CHANNEL_ID_SET"},
            {ANTChannel::CHANNEL_FREQUENCY_SET, "CHANNEL_FREQUENCY_SET"},
            {ANTChannel::CHANNEL_PERIOD_SET, "CHANNEL_PERIOD_SET"},
            {ANTChannel::CHANNEL_OPENED, "CHANNEL_OPENED"},
            {ANTChannel::CHANNEL_SEARCHING, "CHANNEL_SEARCHING"},
            {ANTChannel::CHANNEL_RECEIVING, "CHANNEL_RECEIVING"}
});
}

ANTChannel::ANTChannel(int number, QObject *parent) : QObject(parent), _number(number),
    _state(CHANNEL_CLOSED)
{
    opened = false;
    channel_type=CHANNEL_TYPE_UNUSED;
    channel_type_flags=0;
    device_number=0;
    device_id=0;
    channel_assigned=0;
    messages_received=0;
    messages_dropped=0;
}

// Open an ant channel assignment.
void ANTChannel::open(int device, AntChannelType chan_type)
{
    channel_type=chan_type;
    channel_type_flags = CHANNEL_TYPE_QUICK_SEARCH ;
    device_number=device;

    attemptTransition();
}

// process a channel event message
// XXX should re-use ANTMessage rather than
// raw message data
void ANTChannel::channelEvent(const AntChannelEventMessage &channelEventMessage) {
    const QByteArray message = channelEventMessage.toBytes().mid(2);

    if (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR) {
        attemptTransition();
    } else if (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_CHANNEL_CLOSED) {
        emit antMessageGenerated(AntMessage2::unassignChannel(_number));
    } else if (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_CHANNEL_RX_SEARCH_TIMEOUT) {
        // timeouts are normal for search channel
        if (channel_type_flags & CHANNEL_TYPE_QUICK_SEARCH) {

            channel_type_flags &= ~CHANNEL_TYPE_QUICK_SEARCH;
            channel_type_flags |= CHANNEL_TYPE_WAITING;

            emit searchTimeout(_number);

        } else {

            emit lostInfo(_number);

            channel_type=CHANNEL_TYPE_UNUSED;
            channel_type_flags=0;
            device_number=0;

            emit antMessageGenerated(AntMessage2::unassignChannel(_number));
        }
    } else if (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_CHANNEL_CLOSED) {
        messages_dropped++;

        if (QDateTime::currentDateTime() > (_lastMessageTime.addMSecs(timeout_drop))) {
            if (channel_type != CHANNEL_TYPE_UNUSED) emit dropInfo(_number, messages_dropped, messages_received);
            // this is a hacky way to prevent the drop message from sending multiple times
            _lastMessageTime.addMSecs(2 * timeout_drop);
        }
    } else {
        qDebug() << "Unhandled Channel Event" << channelEventMessage.toString();
    }
}


void ANTChannel::handleCadenceMessage(const CadenceMessage &cadenceMessage)
{
    const CadenceMessage previousCadenceMessage(lastAntMessage);
    calculateCadence(previousCadenceMessage.cadenceEventTime(), previousCadenceMessage.pedalRevolutions(),
                     cadenceMessage.cadenceEventTime(), cadenceMessage.pedalRevolutions(), channel_type);
}

void ANTChannel::handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage &speedAndCadenceMessage)
{
    SpeedAndCadenceMessage lastSpeedAndCadenceMessage(lastAntMessage);
    calculateCadence(lastSpeedAndCadenceMessage.cadenceEventTime(), lastSpeedAndCadenceMessage.pedalRevolutions(),
                     speedAndCadenceMessage.cadenceEventTime(), speedAndCadenceMessage.pedalRevolutions(),
                     channel_type);
    calculateSpeed(lastSpeedAndCadenceMessage.speedEventTime(), lastSpeedAndCadenceMessage.wheelRevolutions(),
                   speedAndCadenceMessage.speedEventTime(), speedAndCadenceMessage.wheelRevolutions(), channel_type);
}

void ANTChannel::handlePowerMessage(const PowerMessage& powerMessage)
{
    if (powerMessage.isPowerOnlyPage()) {
        if (lastStdPwrMessage) {
            if (lastStdPwrMessage->eventCount() != powerMessage.eventCount()) {
                stdNullCount = 0;
                emit powerMeasured(powerMessage.instantaneousPower());
                emit cadenceMeasured(powerMessage.instantaneousCadence(), CHANNEL_TYPE_POWER);
                qDebug() << QDateTime::currentDateTime().toString() << "power" << powerMessage.instantaneousPower() << "cadence" << powerMessage.instantaneousCadence();
            } else {
                stdNullCount += 1;
                if (stdNullCount >= 6) {
                    emit cadenceMeasured(0, CHANNEL_TYPE_POWER);
                    emit powerMeasured(0);
                }
            }
        }

        lastStdPwrMessage.reset(new PowerMessage(powerMessage.antMessage()));
    }
}

void ANTChannel::handleHeartRateMessage(const HeartRateMessage& newHeartRateMessage)
{
    HeartRateMessage lastHeartRateMessage(lastAntMessage);
    quint16 time = newHeartRateMessage.measurementTime() - lastHeartRateMessage.measurementTime();
    if (time) {
        nullCount = 0;
        qDebug() << QTime::currentTime() << "hr = " << newHeartRateMessage.computedHeartRate();
        emit heartRateMeasured(newHeartRateMessage.computedHeartRate());
    } else {
        nullCount++;
        if (nullCount >= 12) {
            emit heartRateMeasured(0);
        }
    }
}

void ANTChannel::handleSpeedMessage(const SpeedMessage &speedMessage)
{
    const SpeedMessage previousSpeedMessage(lastAntMessage);
    calculateSpeed(previousSpeedMessage.speedEventTime(), previousSpeedMessage.wheelRevolutions(),
                   speedMessage.speedEventTime(), speedMessage.wheelRevolutions(), channel_type);
}

void ANTChannel::calculateSpeed(const quint16 previousTime, const quint16 previousWheelRevolutions,
                                const quint16 currentTime, const quint16 currentWheelRevolutions,
                                const AntChannelType channelType)
{
    quint16 time = currentTime - previousTime;
    quint16 revolutions = currentWheelRevolutions - previousWheelRevolutions;
    if (time) {
        dualNullCount = 0;
        float rpm = 1024*60*revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "wheel speed" << rpm << "=" << ((rpm / 60) * 2.070) * 3.6 << "km/h";
        emit speedMeasured(rpm, channelType);
    } else {

        dualNullCount++;
        if (dualNullCount >= 12) {
            emit speedMeasured(0, channelType);
        }
    }
}

void ANTChannel::calculateCadence(const quint16 previousTime, const quint16 previousPedalRevolutions, const quint16 currentTime, const quint16 currentPedalRevolutions, const AntChannelType channelType)
{
    quint16 time = currentTime - previousTime;
    quint16 revolutions = currentPedalRevolutions - previousPedalRevolutions;
    if (time) {
        nullCount = 0;
        float cadence = 1024 * 60 * revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "cadence" << cadence;
        emit cadenceMeasured(cadence, channelType);
    } else {
        nullCount++;
        if (nullCount >= 12) {
            emit cadenceMeasured(0.0f, channelType);
        }
    }
}



/*!
     We got a broadcast event -- this is where inbound
     telemetry gets processed, and for many message types
     we need to remember previous messages to look at the
     deltas during the period XXX this needs fixing!
    */
void ANTChannel::broadcastEvent(const BroadCastMessage &broadcastMessage)
{
    if (channel_type == CHANNEL_TYPE_UNUSED) {
        qDebug() << "Getting a broad cast event for an unused channel. Ignoring..";
        return;
    }

    messages_received++;
    _lastMessageTime = QDateTime::currentDateTime();

    if (messages_received <= 1) {

        // this is mega important! -- when we get broadcast data from a device
        // we ask it to identify itself, then when the channel id message is
        // received we set our channel id to that received. So, if the message
        // below is not sent, we will never set channel properly.

        // The recent bug with not being able to "pair" intermittently, was caused
        // by the write below failing (and any write really, but the one below being
        // pretty critical) -- because the USB stick needed a USB reset which we know
        // do every time we open the USB device
        emit antMessageGenerated(AntMessage2::requestMessage(_number, AntMessage2::SET_CHANNEL_ID));

        lastAntMessage = broadcastMessage.antMessage();
        return; // because we can't associate a channel id with the message yet
    }
    //
    // We got some telemetry on this channel
    //
    if (!lastAntMessage.isNull()) {
        switch (channel_type) {
        // Power
        case CHANNEL_TYPE_POWER:
            handlePowerMessage(broadcastMessage.toSpecificBroadCastMessage<PowerMessage>());
            break;

            // HR
        case CHANNEL_TYPE_HR:
            handleHeartRateMessage(broadcastMessage.toSpecificBroadCastMessage<HeartRateMessage>());
            break;
            // Cadence
        case CHANNEL_TYPE_CADENCE:
            handleCadenceMessage(broadcastMessage.toSpecificBroadCastMessage<CadenceMessage>());
            break;
            // Speed and Cadence
        case CHANNEL_TYPE_SPEED_AND_CADENCE:
            handleSpeedAndCadenceMessage(broadcastMessage.toSpecificBroadCastMessage<SpeedAndCadenceMessage>());
            break;
            // Speed
        case CHANNEL_TYPE_SPEED:
            handleSpeedMessage(broadcastMessage.toSpecificBroadCastMessage<SpeedMessage>());
            break;
        case CHANNEL_TYPE_UNUSED:
            qWarning("We should not be receiving broad cast messages for an unused channel");
        }

    } else {
        // reset nullCount if receiving first telemetry update
        stdNullCount = dualNullCount = nullCount = 0;
    }

    lastAntMessage = broadcastMessage.antMessage();
}


// we got a channel ID notification
void ANTChannel::channelIdEvent(const SetChannelIdMessage& message) {
    device_number = message.deviceNumber();
    device_id = message.deviceTypeId();
    emit channelInfo(_number, device_number, device_id);

    // if we were searching,
    if (channel_type_flags & CHANNEL_TYPE_QUICK_SEARCH) {
        emit antMessageGenerated(AntMessage2::setSearchTimeout(_number, timeout_lost));
    }
    channel_type_flags &= ~CHANNEL_TYPE_QUICK_SEARCH;
}

void ANTChannel::attemptTransition()
{
    qDebug() << "channel" << _number << "current state" << CHANNEL_STATE_STRINGS[_state];
    if (opened)
        return;

    const ant_sensor_type_t& st = ANT::ant_sensor_types[channel_type];

    // do transitions
    switch (_state) {
    case CHANNEL_CLOSED:
        emit antMessageGenerated(AntMessage2::assignChannel(_number));
        _state = CHANNEL_ASSIGNED;
        break;
    case CHANNEL_ASSIGNED:
        emit antMessageGenerated(AntMessage2::setChannelId(_number, 0, channel_type));
        _state = CHANNEL_ID_SET;
        break;
    case CHANNEL_ID_SET:
        emit antMessageGenerated(AntMessage2::setChannelFrequency(_number));
        _state = CHANNEL_FREQUENCY_SET;
        break;
    case CHANNEL_FREQUENCY_SET:
        emit antMessageGenerated(AntMessage2::setChannelPeriod(_number, st.period));
        _state = CHANNEL_PERIOD_SET;
        break;
    case CHANNEL_PERIOD_SET:
        emit antMessageGenerated(AntMessage2::openChannel(_number));
        _state = CHANNEL_OPENED;
        break;
    case CHANNEL_OPENED:
        _state = CHANNEL_SEARCHING;
        break;
    }
}
