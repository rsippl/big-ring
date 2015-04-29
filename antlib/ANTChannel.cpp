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
    state=ANT_UNASSIGN_CHANNEL;
    messages_received=0;
    messages_dropped=0;
}

// Open an ant channel assignment.
void ANTChannel::open(int device, AntChannelType chan_type)
{
    channel_type=chan_type;
    channel_type_flags = CHANNEL_TYPE_QUICK_SEARCH ;
    device_number=device;

    attemptTransition(ANT_UNASSIGN_CHANNEL);
}

//
// The main read loop is in ANT.cpp, it will pass us
// the inbound message received for our channel.
// XXX fix this up to re-use ANTMessage for decoding
// all the inbound messages
//
void ANTChannel::receiveMessage(const QByteArray& bytes)
{
    QByteArray copy(bytes);
    std::unique_ptr<AntMessage2> antMessage = AntMessage2::createMessageFromBytes(bytes);
    unsigned char* ant_message = reinterpret_cast<unsigned char*>(copy.data());
    switch(antMessage->id()) {
    case AntMessage2::BROADCAST_EVENT:
        broadcastEvent(BroadCastMessage(*antMessage));
        break;
    case AntMessage2::CHANNEL_EVENT:
        channelEvent(bytes);
        break;
    case AntMessage2::SET_CHANNEL_ID:
        channelId(ant_message);
        break;
    default:
        qDebug() << "Unhandled message" << antMessage->toString();
    }
}


// process a channel event message
// XXX should re-use ANTMessage rather than
// raw message data
void ANTChannel::channelEvent(const QByteArray &bytes) {
    AntChannelEventMessage channelEventMessage(bytes);
    const QByteArray message = bytes.mid(2);

    //qDebug()<<"channel event:"<< ANTMessage::channelEventMessage(*(message+1));
    if (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR) {
        qDebug() << channelEventMessage.toString();
        attemptTransition(channelEventMessage.messageId());
    } else if (MESSAGE_IS_EVENT_CHANNEL_CLOSED(message)) {
        emit antMessageGenerated(AntMessage2::unassignChannel(_number));
    } else if (MESSAGE_IS_EVENT_RX_SEARCH_TIMEOUT(message)) {
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
    } else if (MESSAGE_IS_EVENT_RX_FAIL(message)) {
        messages_dropped++;

        if (QDateTime::currentDateTime() > (_lastMessageTime.addMSecs(timeout_drop))) {
            if (channel_type != CHANNEL_TYPE_UNUSED) emit dropInfo(_number, messages_dropped, messages_received);
            // this is a hacky way to prevent the drop message from sending multiple times
            _lastMessageTime.addMSecs(2 * timeout_drop);
        }
    } else {

        // XXX not handled!
    }
}


void ANTChannel::handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage &speedAndCadenceMessage)
{
    SpeedAndCadenceMessage lastSpeedAndCadenceMessage(lastAntMessage);
    quint16 time = speedAndCadenceMessage.cadenceEventTime()- lastSpeedAndCadenceMessage.cadenceEventTime();
    quint16 revolutions = speedAndCadenceMessage.pedalRevolutions() -
            lastSpeedAndCadenceMessage.pedalRevolutions();
    if (time) {
        nullCount = 0;
        float cadence = 1024 * 60 * revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "cadence" << cadence;
        emit cadenceMeasured(cadence, CHANNEL_TYPE_SPEED_AND_CADENCE);
    } else {
        nullCount++;
        if (nullCount >= 12) {
            emit cadenceMeasured(0.0f, CHANNEL_TYPE_SPEED_AND_CADENCE);
        }
    }

    time = speedAndCadenceMessage.speedEventTime()- lastSpeedAndCadenceMessage.speedEventTime();
    revolutions = speedAndCadenceMessage.wheelRevolutions() - lastSpeedAndCadenceMessage.wheelRevolutions();
    if (time) {
        dualNullCount = 0;
        float rpm = 1024*60*revolutions / static_cast<float>(time);
        qDebug() << QTime::currentTime().toString() << "wheel speed" << rpm << ((rpm / 60) * 2.070) * 3.6;
        emit speedMeasured(rpm, CHANNEL_TYPE_SPEED_AND_CADENCE);
    } else {

        dualNullCount++;
        if (dualNullCount >= 12) {
            emit speedMeasured(0, CHANNEL_TYPE_SPEED_AND_CADENCE);
        }
    }
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

/*!
     We got a broadcast event -- this is where inbound
     telemetry gets processed, and for many message types
     we need to remember previous messages to look at the
     deltas during the period XXX this needs fixing!
    */
void ANTChannel::broadcastEvent(const BroadCastMessage &broadcastMessage)
{
    ANTMessage antMessage(channel_type,
                          reinterpret_cast<const unsigned char*>(broadcastMessage.antMessage().toBytes().data()));
    bool savemessage = true; // flag to stop lastmessage being
    // overwritten for standard power
    // messages


//    double timestamp= QDateTime::currentMSecsSinceEpoch();

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
    if (lastMessage.type != 0) {

        switch (channel_type) {

        // Power
        case CHANNEL_TYPE_POWER:
            savemessage = false;
            handlePowerMessage(broadcastMessage.toPowerMessage());
            break;

            // HR
        case CHANNEL_TYPE_HR:
            handleHeartRateMessage(broadcastMessage.toHeartRateMessage());
            break;
            // Cadence
        case CHANNEL_TYPE_CADENCE:
        {
            uint16_t time = antMessage.crankMeasurementTime - lastMessage.crankMeasurementTime;
            uint16_t revs = antMessage.crankRevolutions - lastMessage.crankRevolutions;
            if (time) {
                float cadence = 1024*60*revs / time;
                emit cadenceMeasured(cadence, CHANNEL_TYPE_CADENCE);
            }
        }
            break;

            // Speed and Cadence
        case CHANNEL_TYPE_SPEED_AND_CADENCE:
        {
            handleSpeedAndCadenceMessage(broadcastMessage.toSpeedAndCadenceMessage());

            // now speed ...
            quint16 time = antMessage.wheelMeasurementTime - lastMessage.wheelMeasurementTime;
            quint16 revs = antMessage.wheelRevolutions - lastMessage.wheelRevolutions;
            if (time) {
                dualNullCount = 0;

                float rpm = 1024*60*revs / time;
                emit speedMeasured(rpm, CHANNEL_TYPE_SPEED_AND_CADENCE);
            } else {

                dualNullCount++;
                if (dualNullCount >= 12) {
                    emit speedMeasured(0, CHANNEL_TYPE_SPEED_AND_CADENCE);
                }
            }
        }
            break;

            // Speed
        case CHANNEL_TYPE_SPEED:
        {
            uint16_t time = antMessage.wheelMeasurementTime - lastMessage.wheelMeasurementTime;
            uint16_t revs = antMessage.wheelRevolutions - lastMessage.wheelRevolutions;
            if (time) {
                nullCount=0;
                float rpm = 1024*60*revs / time;
                emit speedMeasured(rpm, CHANNEL_TYPE_SPEED);
            } else {
                nullCount++;

                if (nullCount >= 12) {
                    emit speedMeasured(0, CHANNEL_TYPE_SPEED);
                }
            }
        }
            break;

        default:
            break; // unknown?
        }

    } else {
        // reset nullCount if receiving first telemetry update
        stdNullCount = dualNullCount = nullCount = 0;
    }

    // we don't overwrite for Standard Power messages
    // these are maintained separately in lastStdPwrMessage
    if (savemessage) {
        lastMessage = antMessage;
        lastAntMessage = broadcastMessage.antMessage();
    }
}


// we got a channel ID notification
void ANTChannel::channelId(unsigned char *ant_message) {

    unsigned char *message=ant_message+2;

    device_number=CHANNEL_ID_DEVICE_NUMBER(message);
    device_id=CHANNEL_ID_DEVICE_TYPE_ID(message);
    state=MESSAGE_RECEIVED;
    emit channelInfo(_number, device_number, device_id);

    // if we were searching,
    if (channel_type_flags & CHANNEL_TYPE_QUICK_SEARCH) {
        emit antMessageGenerated(AntMessage2::setSearchTimeout(_number, timeout_lost));
    }
    channel_type_flags &= ~CHANNEL_TYPE_QUICK_SEARCH;
}

// are we in the middle of a search?
int ANTChannel::isSearching() {
    return ((channel_type_flags & (CHANNEL_TYPE_WAITING | CHANNEL_TYPE_QUICK_SEARCH)) || (state != MESSAGE_RECEIVED));
}


void ANTChannel::attemptTransition(int message_id)
{
    qDebug() << "channel" << _number << "current state" << CHANNEL_STATE_STRINGS[_state];
    if (opened)
        return;

    const ant_sensor_type_t& st = ANT::ant_sensor_types[channel_type];

    // update state
    state=message_id;

    qDebug() << "channel" << _number << "state" << QString::number(state, 16);
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
