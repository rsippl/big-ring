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
#include "antchanneltype.h"
#include "antmessage2.h"
#include <QObject>
#include <QDateTime>
#include <QtCore/QScopedPointer>

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
        CHANNEL_TRACKING
    };
private:
    int _channelNumber;
    ChannelState _state;

    AntMessage2 lastAntMessage;
    QScopedPointer<PowerMessage> lastStdPwrMessage;
    int dualNullCount, nullCount, stdNullCount;
    QDateTime _lastMessageTime;

    int messages_received; // for signal strength metric
    int messages_dropped;

    void handleCadenceMessage(const CadenceMessage& cadenceMessage);
    void handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage &speedAndCadenceMessage);
    void handlePowerMessage(const PowerMessage &powerMessage);
    void handleHeartRateMessage(const HeartRateMessage &newMessage);
    void handleSpeedMessage(const SpeedMessage& speedMessage);

    void calculateSpeed(const quint16 previousTime, const quint16 previousWheelRevolutions,
                        const quint16 currentTime, const quint16 currentWheelRevolutions,
                        const indoorcycling::AntChannelType channelType);
    void calculateCadence(const quint16 previousTime, const quint16 previousPedalRevolutions,
                        const quint16 currentTime, const quint16 currentPedalRevolutions,
                        const indoorcycling::AntChannelType channelType);
    void attemptTransition();
    // convert ANT value to human string
    const QString deviceTypeDescription(indoorcycling::AntChannelType type);


public:
    indoorcycling::AntChannelType channel_type;
    int deviceNumber;
    indoorcycling::AntChannelType deviceTypeId;

    ANTChannel(int channelNumber, QObject *parent = 0);

    void open(int deviceNumber, indoorcycling::AntChannelType channel_type);

    void channelEvent(const AntChannelEventMessage& bytes);
    void broadcastEvent(const BroadCastMessage& broadcastMessage);
    void channelIdEvent(const SetChannelIdMessage& channelIdMessage);

signals:

    void channelInfo(int channelNumber, int deviceNumber, indoorcycling::AntChannelType deviceTypeId, const QString& description); // we got a channel info message
    void lostInfo(int channelNumber);    // we lost a connection
    void searchTimeout(int channelNumber); // search timed out
    void antMessageGenerated(const AntMessage2& antMessage);
    /** heart rate in beats per minute */
    void heartRateMeasured(int bpm);
    /** power in watts */
    void powerMeasured(float watts);
    /** cadence in revolutions per minute */
    void cadenceMeasured(float rpm, indoorcycling::AntChannelType channelType);
    /** wheel speed in revolutions per minute */
    void speedMeasured(float rpm, indoorcycling::AntChannelType channelType);
};
#endif
