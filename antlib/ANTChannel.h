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

#ifndef gc_ANTChannel_h
#define gc_ANTChannel_h

#include "ANT.h"
#include "antmessage2.h"
#include <QObject>
#include <QDateTime>
#include <QtCore/QScopedPointer>

#define CHANNEL_TYPE_QUICK_SEARCH 0x10 // or'ed with current channel type
/* after fast search, wait for slow search.  Otherwise, starting slow
   search might postpone the fast search on another channel. */
#define CHANNEL_TYPE_WAITING 0x20
#define CHANNEL_TYPE_PAIR   0x40 // to do an Ant pair
#define MESSAGE_RECEIVED -1

class ANTChannel : public QObject {
    Q_OBJECT
public:
    enum ChannelState {
        CHANNEL_CLOSED,
        CHANNEL_ASSIGNED,
        CHANNEL_ID_SET,
        CHANNEL_FREQUENCY_SET,
        CHANNEL_PERIOD_SET,
        CHANNEL_OPENED,
        CHANNEL_SEARCHING,
        CHANNEL_RECEIVING
    };
private:
    int _number;
    ChannelState _state;

    AntMessage2 lastAntMessage;
    QScopedPointer<PowerMessage> lastStdPwrMessage;
    int dualNullCount, nullCount, stdNullCount;
    QDateTime _lastMessageTime;

    bool channel_assigned;
    bool opened;

    int messages_received; // for signal strength metric
    int messages_dropped;

    void handleCadenceMessage(const CadenceMessage& cadenceMessage);
    void handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage &speedAndCadenceMessage);
    void handlePowerMessage(const PowerMessage &powerMessage);
    void handleHeartRateMessage(const HeartRateMessage &newMessage);
    void handleSpeedMessage(const SpeedMessage& speedMessage);

    void calculateSpeed(const quint16 previousTime, const quint16 previousWheelRevolutions,
                        const quint16 currentTime, const quint16 currentWheelRevolutions,
                        const AntChannelType channelType);
    void calculateCadence(const quint16 previousTime, const quint16 previousPedalRevolutions,
                        const quint16 currentTime, const quint16 currentPedalRevolutions,
                        const AntChannelType channelType);
public:
    // Channel Information - to save tedious set/getters made public
     // Channel number within Ant chip
    int state;
    AntChannelType channel_type;
    int device_number;
    int channel_type_flags;
    int device_id;
    int search_type;

    ANTChannel(int _number, QObject *parent);

    void open(int device_number, AntChannelType channel_type);

    void channelEvent(const AntChannelEventMessage& bytes);
    void broadcastEvent(const BroadCastMessage& broadcastMessage);
    void channelIdEvent(const SetChannelIdMessage& channelIdMessage);

    void attemptTransition(int message_code);

    // search
    int isSearching();

signals:

    void channelInfo(int _number, int device_number, int device_id); // we got a channel info message
    void dropInfo(int _number, int dropped, int received);    // we dropped a packet
    void lostInfo(int _number);    // we lost a connection
    void staleInfo(int _number);   // the connection is stale
    void searchTimeout(int _number); // search timed out
    void searchComplete(int _number); // search completed successfully
    void antMessageGenerated(const AntMessage2& antMessage);
    /** heart rate in beats per minute */
    void heartRateMeasured(int bpm);
    /** power in watts */
    void powerMeasured(float watts);
    /** cadence in revolutions per minute */
    void cadenceMeasured(float rpm, AntChannelType channelType);
    /** wheel speed in revolutions per minute */
    void speedMeasured(float rpm, AntChannelType channelType);
};
#endif
