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
}

ANTChannel::ANTChannel(int number, QObject *parent) : QObject(parent), number(number)
{
    init();
}

void
ANTChannel::init()
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
    setId();
}

//
// channel id is in the form nnnnx where nnnn is the device number
// and x is the channel type (p)ower, (c) adence etc the full list
// can be found in ANT.cpp when initialising ant_sensor_types
//
void ANTChannel::setId()
{
    if (channel_type==CHANNEL_TYPE_UNUSED) {
        strcpy(id, "none");
    } else {
        snprintf(id, 10, "%d%c", device_number, ANT::ant_sensor_types[channel_type].suffix);
    }
}

// Open an ant channel assignment.
void ANTChannel::open(int device, int chan_type)
{
    channel_type=chan_type;
    channel_type_flags = CHANNEL_TYPE_QUICK_SEARCH ;
    device_number=device;

    setId();

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
        broadcastEvent(*antMessage);
        break;
    case AntMessage2::CHANNEL_EVENT:
        channelEvent(bytes);
        break;
    case AntMessage2::SET_CHANNEL_ID:
        channelId(ant_message);
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
    //        unsigned char *message=ant_message+2;


    //qDebug()<<"channel event:"<< ANTMessage::channelEventMessage(*(message+1));
    if (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR) {
        attemptTransition(channelEventMessage.messageId());
    } else if (MESSAGE_IS_EVENT_CHANNEL_CLOSED(message)) {
        emit antMessageGenerated(AntMessage2::unassignChannel(number));
    } else if (MESSAGE_IS_EVENT_RX_SEARCH_TIMEOUT(message)) {

        // timeouts are normal for search channel
        if (channel_type_flags & CHANNEL_TYPE_QUICK_SEARCH) {

            channel_type_flags &= ~CHANNEL_TYPE_QUICK_SEARCH;
            channel_type_flags |= CHANNEL_TYPE_WAITING;

            emit searchTimeout(number);

        } else {

            emit lostInfo(number);

            channel_type=CHANNEL_TYPE_UNUSED;
            channel_type_flags=0;
            device_number=0;
            setId();

            emit antMessageGenerated(AntMessage2::unassignChannel(number));
        }
    } else if (MESSAGE_IS_EVENT_RX_FAIL(message)) {
        messages_dropped++;

        if (QDateTime::currentDateTime() > (_lastMessageTime.addMSecs(timeout_drop))) {
            if (channel_type != CHANNEL_TYPE_UNUSED) emit dropInfo(number, messages_dropped, messages_received);
            // this is a hacky way to prevent the drop message from sending multiple times
            _lastMessageTime.addMSecs(2 * timeout_drop);
        }
    } else {

        // XXX not handled!
    }
}


void ANTChannel::handlePowerMessage(ANTMessage antMessage)
{
    uint8_t events = antMessage.eventCount - lastStdPwrMessage.eventCount;
    if (lastStdPwrMessage.type && events) {
        stdNullCount = 0;
        emit powerMeasured(antMessage.instantPower);
        emit cadenceMeasured(antMessage.instantCadence, CHANNEL_TYPE_POWER);
    } else {
        stdNullCount++;
        if (stdNullCount >= 6) { //XXX 6 for standard power?
            emit cadenceMeasured(0.0f, CHANNEL_TYPE_POWER);
            emit powerMeasured(0.0f);
        }
    }
    lastStdPwrMessage = antMessage;
}

void ANTChannel::handleHeartRateMessage(const AntMessage2 &antMessage)
{
    HeartRateMessage lastHeartRateMessage(lastAntMessage);
    HeartRateMessage newHeartRateMessage(antMessage);
    quint16 time = newHeartRateMessage.measurementTime() - lastHeartRateMessage.measurementTime();
    if (time) {
        nullCount = 0;
        qDebug() << "hr = " << newHeartRateMessage.computedHeartRate();
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
void ANTChannel::broadcastEvent(const AntMessage2 &broadcastMessage)
{
    ANTMessage antMessage(channel_type,
                          reinterpret_cast<const unsigned char*>(broadcastMessage.toBytes().data()));
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
        emit antMessageGenerated(AntMessage2::requestMessage(number, AntMessage2::SET_CHANNEL_ID));
        return; // because we can't associate a channel id with the message yet
    }
    //
    // We got some telemetry on this channel
    //
    if (lastMessage.type != 0) {

        switch (channel_type) {

        // Power
        case CHANNEL_TYPE_POWER:
            // what kind of power device is this?
            switch(antMessage.data_page) {
            case ANT_STANDARD_POWER: // 0x10 - standard power
            {
                handlePowerMessage(antMessage);
                savemessage = false;
            }
                break;
            default: // other data page. We'll just ignore it.
                break;
            }
            break;

            // HR
        case CHANNEL_TYPE_HR:
            handleHeartRateMessage(broadcastMessage);
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
        case CHANNEL_TYPE_SandC:
        {
            // cadence first...
            uint16_t time = antMessage.crankMeasurementTime - lastMessage.crankMeasurementTime;
            uint16_t revs = antMessage.crankRevolutions - lastMessage.crankRevolutions;
            if (time) {
                nullCount = 0;
                float cadence = 1024*60*revs / time;
                emit cadenceMeasured(cadence, CHANNEL_TYPE_SandC);
            } else {
                nullCount++;
                if (nullCount >= 12) {
                    emit cadenceMeasured(0.0f, CHANNEL_TYPE_SandC);
                }
            }

            // now speed ...
            time = antMessage.wheelMeasurementTime - lastMessage.wheelMeasurementTime;
            revs = antMessage.wheelRevolutions - lastMessage.wheelRevolutions;
            if (time) {
                dualNullCount = 0;

                float rpm = 1024*60*revs / time;
                emit speedMeasured(rpm, CHANNEL_TYPE_SandC);
            } else {

                dualNullCount++;
                if (dualNullCount >= 12) {
                    emit speedMeasured(0, CHANNEL_TYPE_SandC);
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
        lastAntMessage = broadcastMessage;
    }
}


// we got a channel ID notification
void ANTChannel::channelId(unsigned char *ant_message) {

    unsigned char *message=ant_message+2;

    device_number=CHANNEL_ID_DEVICE_NUMBER(message);
    device_id=CHANNEL_ID_DEVICE_TYPE_ID(message);
    state=MESSAGE_RECEIVED;
    emit channelInfo(number, device_number, device_id);
    setId();

    // if we were searching,
    if (channel_type_flags & CHANNEL_TYPE_QUICK_SEARCH) {
        emit antMessageGenerated(AntMessage2::setSearchTimeout(number, timeout_lost));
    }
    channel_type_flags &= ~CHANNEL_TYPE_QUICK_SEARCH;

    //XXX channel_manager_start_waiting_search(self->parent);
    // if we are quarq channel, hook up with the ant+ channel we are connected to
    //XXX channel_manager_associate_control_channels(self->parent);
}

// are we in the middle of a search?
int ANTChannel::isSearching() {
    return ((channel_type_flags & (CHANNEL_TYPE_WAITING | CHANNEL_TYPE_QUICK_SEARCH)) || (state != MESSAGE_RECEIVED));
}


void ANTChannel::attemptTransition(int message_id)
{
    if (opened)
        return;

    const ant_sensor_type_t *st;

    st=&(ANT::ant_sensor_types[channel_type]);

    // update state
    state=message_id;

    qDebug() << "channel" << number << "state" << QString::number(state, 16);
    // do transitions
    switch (state) {

    case ANT_CLOSE_CHANNEL:
        // next step is unassign and start over
        // but we must wait until event_channel_closed
        // which is its own channel event
        state=MESSAGE_RECEIVED;
        break;

    case ANT_UNASSIGN_CHANNEL:
        channel_assigned=0;

        // lets make sure this channel is assigned to our network
        // regardless of its current state.
        //            parent->sendMessage(AntMessage2::unassignChannel(number)); // unassign whatever we had before

        // reassign to whatever we need!
        emit antMessageGenerated(AntMessage2::assignChannel(number));
        device_id=st->device_id;
        //            parent->sendMessage(AntMessage2::setChannelId(number, 0, device_id)); // lets go back to allowing anything
        setId();
        break;

    case ANT_ASSIGN_CHANNEL:
        channel_assigned=1;
        emit antMessageGenerated(AntMessage2::setChannelId(number, 0, device_id));
        break;

    case ANT_CHANNEL_ID:
        emit antMessageGenerated(AntMessage2::setChannelFrequency(number));
        break;
        //            if (channel_type & CHANNEL_TYPE_QUICK_SEARCH) {
        //                parent->sendMessage(AntMessage2::setSearchTimeout(number, timeout_scan));
        //            } else {
        //                parent->sendMessage(AntMessage2::setSearchTimeout(number, timeout_lost));
        //            }
        //            break;
    case ANT_SEARCH_TIMEOUT:
        //            if (previous_state==ANT_CHANNEL_ID) {
        //                // continue down the intialization chain
        //                parent->sendMessage(ANTMessage::setChannelPeriod(number, st->period));
        //            } else {
        //                // we are setting the ant_search timeout after connected
        //                // we'll just pretend this never happened
        //                state=previous_state;
        //            }
        //            break;

    case ANT_CHANNEL_FREQUENCY:
        emit antMessageGenerated(AntMessage2::setChannelPeriod(number, st->period));
        break;
    case ANT_CHANNEL_PERIOD:
        emit antMessageGenerated(AntMessage2::openChannel(number));
        break;
        //
    case ANT_OPEN_CHANNEL:
        opened = true;
        qDebug() << "Channel" << number << "open";
        //            parent->sendMessage(ANTMessage::open(number));
        break;

    default:
        break;
    }
}
