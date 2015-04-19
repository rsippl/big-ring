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
const quint32 timeout_blanking = 2000; // ms
const quint32 timeout_drop = 2000; // ms
const quint32 timeout_scan = 10000; // ms
const quint32 timeout_lost = 30; // s
}

ANTChannel::ANTChannel(int number, ANT *parent) : QObject(parent), parent(parent), number(number)
{
    init();
}

void
ANTChannel::init()
{
    channel_type=CHANNEL_TYPE_UNUSED;
    channel_type_flags=0;
    manufacturer_id=0;
    product_id=0;
    product_version=0;
    device_number=0;
    device_id=0;
    channel_assigned=0;
    state=ANT_UNASSIGN_CHANNEL;
    blanked=1;
    messages_received=0;
    messages_dropped=0;
    setId();
    burstInit();
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
        snprintf(id, 10, "%d%c", device_number, parent->ant_sensor_types[channel_type].suffix);
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

    // close an ant channel assignment
    void ANTChannel::close()
    {
        emit lostInfo(number);
        lastMessage = ANTMessage();
        parent->sendMessage(ANTMessage::close(number));
        init();
    }

    //
    // The main read loop is in ANT.cpp, it will pass us
    // the inbound message received for our channel.
    // XXX fix this up to re-use ANTMessage for decoding
    // all the inbound messages
    //
    void ANTChannel::receiveMessage(unsigned char *ant_message)
    {
        switch (ant_message[2]) {
        case ANT_CHANNEL_EVENT:
            channelEvent(ant_message);
            break;
        case ANT_BROADCAST_DATA:
            broadcastEvent(ant_message);
            break;
        case ANT_CHANNEL_ID:
            channelId(ant_message);
            break;
        case ANT_BURST_DATA:
            burstData(ant_message);
            break;
        default:
            break; //XXX should trap error here, but silently ignored for now
        }

        if (QDateTime::currentDateTime().toMSecsSinceEpoch() > blanking_timestamp + timeout_blanking) {
            if (!blanked) {
                blanked=1;
                emit staleInfo(number);
            }
        } else blanked=0;
    }


    // process a channel event message
    // XXX should re-use ANTMessage rather than
    // raw message data
    void ANTChannel::channelEvent(unsigned char *ant_message) {

        unsigned char *message=ant_message+2;

        //qDebug()<<"channel event:"<< ANTMessage::channelEventMessage(*(message+1));

        if (MESSAGE_IS_RESPONSE_NO_ERROR(message)) {

            attemptTransition(RESPONSE_NO_ERROR_MESSAGE_ID(message));

        } else if (MESSAGE_IS_EVENT_CHANNEL_CLOSED(message)) {
            parent->sendMessage(AntMessage2::unassignChannel(number));
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

                parent->sendMessage(AntMessage2::unassignChannel(number));
            }

            //XXX channel_manager_start_waiting_search(self->parent);

        } else if (MESSAGE_IS_EVENT_RX_FAIL(message)) {

            messages_dropped++;
            qint64 t = QDateTime::currentMSecsSinceEpoch();

            if (t > (last_message_timestamp + timeout_drop)) {
                if (channel_type != CHANNEL_TYPE_UNUSED) emit dropInfo(number, messages_dropped, messages_received);
                // this is a hacky way to prevent the drop message from sending multiple times
                last_message_timestamp+=2*timeout_drop;
            }

        } else if (MESSAGE_IS_EVENT_RX_ACKNOWLEDGED(message)) {

            exit(-10);

        } else if (MESSAGE_IS_EVENT_TRANSFER_TX_COMPLETED(message)) {

            if (tx_ack_disposition) {} //XXX tx_ack_disposition();

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

    /*!
     We got a broadcast event -- this is where inbound
     telemetry gets processed, and for many message types
     we need to remember previous messages to look at the
     deltas during the period XXX this needs fixing!
    */
    void ANTChannel::broadcastEvent(unsigned char *ant_message)
    {
        ANTMessage antMessage(channel_type, ant_message);
        bool savemessage = true; // flag to stop lastmessage being
        // overwritten for standard power
        // messages

        unsigned char *message=ant_message+2;
        double timestamp= QDateTime::currentMSecsSinceEpoch();

        messages_received++;
        last_message_timestamp=timestamp;

        if (messages_received <= 1) {

            // this is mega important! -- when we get broadcast data from a device
            // we ask it to identify itself, then when the channel id message is
            // received we set our channel id to that received. So, if the message
            // below is not sent, we will never set channel properly.

            // The recent bug with not being able to "pair" intermittently, was caused
            // by the write below failing (and any write really, but the one below being
            // pretty critical) -- because the USB stick needed a USB reset which we know
            // do every time we open the USB device
            parent->sendMessage(ANTMessage::requestMessage(number, ANT_CHANNEL_ID));
            blanking_timestamp=QDateTime::currentMSecsSinceEpoch();
            blanked=0;
            return; // because we can't associate a channel id with the message yet
        }

        // for automatically opening quarq channel on early cinqo
        if (MESSAGE_IS_PRODUCT(message)) {
            product_version&=0x00ff;
            product_version|=(PRODUCT_SW_REV(message))<<8;

        } else if (MESSAGE_IS_MANUFACTURER(message)) {
            product_version&=0xff00;
            product_version|=MANUFACTURER_HW_REV(message);
            manufacturer_id=MANUFACTURER_MANUFACTURER_ID(message);
            product_id=MANUFACTURER_MODEL_NUMBER_ID(message);

        } else {
            //
            // We got some telemetry on this channel
            //
            if (lastMessage.type != 0) {

                switch (channel_type) {

                // Power
                case CHANNEL_TYPE_POWER:
                    // what kind of power device is this?
                    switch(antMessage.data_page) {

                    case ANT_SPORT_CALIBRATION_MESSAGE:
                    {
                        // Always ack calibs unless they are acks too!
                        if (antMessage.data[6] != 0xAC) {
                            antMessage.data[6] = 0xAC;
                            parent->sendMessage(antMessage);
                        }
                    } // ANT_SPORT_CALIBRATION
                        savemessage = false; // we don't want to overwrite other messages
                        break;
                        //
                        // Standard Power - interleaved with other messages 1 per second
                        //                  NOTE: Standard power messages are interleaved
                        //                        with other power broadcast messages and so
                        //                        we need to make sure lastmessage is NOT
                        //                        updated with this message and instead we
                        //                        store in a special lastStdPwrMessage
                        //
                    case ANT_STANDARD_POWER: // 0x10 - standard power
                    {
                        handlePowerMessage(antMessage);
                        savemessage = false;
                    }
                        break;
                    default: // UNKNOWN POWER DEVICE? XXX Garmin (Metrigear) Vector????
                        break;
                    }
                    break;

                    // HR
                case CHANNEL_TYPE_HR:
                {
                    // cadence first...
                    uint16_t time = antMessage.measurementTime - lastMessage.measurementTime;
                    if (time) {
                        nullCount = 0;
                        emit heartRateMeasured(antMessage.instantHeartrate);
                    } else {
                        nullCount++;
                        if (nullCount >= 12) {
                            emit heartRateMeasured(0);
                        }
                    }
                }
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
            if (savemessage) lastMessage = antMessage;
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
            parent->sendMessage(AntMessage2::setSearchTimeout(number, timeout_lost));
        }
        channel_type_flags &= ~CHANNEL_TYPE_QUICK_SEARCH;

        //XXX channel_manager_start_waiting_search(self->parent);
        // if we are quarq channel, hook up with the ant+ channel we are connected to
        //XXX channel_manager_associate_control_channels(self->parent);
    }

    // get ready to burst
    void ANTChannel::burstInit() {
        rx_burst_data_index=0;
        rx_burst_next_sequence=0;
        rx_burst_disposition=NULL;
    }

    // are we in the middle of a search?
    int ANTChannel::isSearching() {
        return ((channel_type_flags & (CHANNEL_TYPE_WAITING | CHANNEL_TYPE_QUICK_SEARCH)) || (state != MESSAGE_RECEIVED));
    }


    // receive burst data
    void ANTChannel::burstData(unsigned char *ant_message) {

        unsigned char *message=ant_message+2;
        char seq=(message[1]>>5)&0x3;
        char last=(message[1]>>7)&0x1;
        const unsigned char next_sequence[4]={1,2,3,1};

        if (seq!=rx_burst_next_sequence) {
            // XXX handle errors
        } else {

            int len=ant_message[ANT_OFFSET_LENGTH]-3;

            if ((rx_burst_data_index + len)>(RX_BURST_DATA_LEN)) {
                len = RX_BURST_DATA_LEN-rx_burst_data_index;
            }

            rx_burst_next_sequence=next_sequence[(int)seq];
            memcpy(rx_burst_data+rx_burst_data_index, message+2, len);
            rx_burst_data_index+=len;
        }

        if (last) {
            if (rx_burst_disposition) {
                //XXX what does this do? rx_burst_disposition();
            }
            burstInit();
        }
    }

    // choose this one..
    void ANTChannel::setChannelID(int device_number, int device_id, int)
    {
        parent->sendMessage(AntMessage2::setChannelId(number, device_number, device_id)); // lets go back to allowing anything
        parent->sendMessage(ANTMessage::open(number)); // lets go back to allowing anything
    }

    void ANTChannel::attemptTransition(int message_id)
    {

        const ant_sensor_type_t *st;
        int previous_state=state;
        st=&(parent->ant_sensor_types[channel_type]);

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
            parent->sendMessage(AntMessage2::assignChannel(number, 0, st->network)); // recieve channel on network 1
            device_id=st->device_id;
//            parent->sendMessage(AntMessage2::setChannelId(number, 0, device_id)); // lets go back to allowing anything
            setId();
            break;

        case ANT_ASSIGN_CHANNEL:
            channel_assigned=1;
            parent->sendMessage(AntMessage2::setChannelId(number, 0, device_id));
            break;

        case ANT_CHANNEL_ID:
            parent->sendMessage(ANTMessage::setChannelFreq(number, st->frequency));
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
            parent->sendMessage(ANTMessage::setChannelPeriod(number, st->period));
            break;
        case ANT_CHANNEL_PERIOD:
            parent->sendMessage(ANTMessage::open(number));
            break;
//
        case ANT_OPEN_CHANNEL:
            qDebug() << "Channel" << number << "open";
//            parent->sendMessage(ANTMessage::open(number));
            break;

        default:
            break;
        }
    }

    //
    // Calibrate... XXX not used at present
    //
    // request the device on this channel calibrates itselt
    void ANTChannel::requestCalibrate() {
        parent->sendMessage(ANTMessage::requestCalibrate(number));
    }
