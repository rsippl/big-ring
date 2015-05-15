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
#ifndef ANTCHANNELHANDLER_H
#define ANTCHANNELHANDLER_H

#include <memory>
#include <QtCore/QObject>

#include "antmessage2.h"
#include "antsensortype.h"
namespace indoorcycling
{
class AntChannelHandler : public QObject
{
    Q_OBJECT
public:
    virtual ~AntChannelHandler();

    /** TODO: move this to a "better location".*/
    enum AntSportPeriod {
        ANT_SPORT_UNUSED_PERIOD = 0,
        ANT_SPORT_HR_PERIOD = 8070,
        ANT_SPORT_POWER_PERIOD = 8182,
        ANT_SPORT_SPEED_PERIOD = 8118,
        ANT_SPORT_CADENCE_PERIOD = 8102,
        ANT_SPORT_SPEED_AND_CADENCE_PERIOD = 8086
    };
    enum ChannelState {
        CHANNEL_CLOSED,
        CHANNEL_ASSIGNED,
        CHANNEL_ID_SET,
        CHANNEL_FREQUENCY_SET,
        CHANNEL_PERIOD_SET,
        CHANNEL_TIMEOUT_SET,
        CHANNEL_OPENED,
        CHANNEL_SEARCHING,
        CHANNEL_SEARCH_TIMEOUT,
        CHANNEL_TRACKING,
        CHANNEL_LOST_CONNECTION,
        CHANNEL_UNASSIGNED
    };

    ChannelState state() const;
    AntSensorType sensorType() const;
    int sensorDeviceNumber() const;
signals:
    void antMessageGenerated(const AntMessage2& message);
    void sensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                     const QVariant& sensorValue);
    void stateChanged(ChannelState state);
    void sensorFound(int channelNumber, AntSensorType sensorType, int sensorDeviceNumber);
    void searchTimeout(int channelNumber, AntSensorType sensorType);
    /** emitted when communication on the channel is finished and it can be deleted */
    void finished(int channelNumber);
public slots:
    /** Set the device number of the sensor. Call this method before initializing
     * the channel, because it will be used to set the channel's parameters. */
    void setSensorDeviceNumber(int deviceNumber);
    /** initialize the channel */
    void initialize();
    /**
     * @brief close the channel
     */
    void close();

    void handleChannelEvent(const AntChannelEventMessage& message);
    void handleBroadcastEvent(const BroadCastMessage& broadcastMessage);
    void handleChannelIdEvent(const SetChannelIdMessage& channelIdMessage);
protected:
    explicit AntChannelHandler(const int channelNumber, const AntSensorType sensorType,
                               AntSportPeriod channelPeriod, QObject* parent);
    /** This method should be implemented by subclasses for their
     * specific way of handling broadcast messages.
     */
    virtual void handleBroadCastMessage(const BroadCastMessage& message) = 0;
private:
    void setState(ChannelState state);
    void advanceState(const AntMessage2::AntMessageId messageId);
    void handleFirstBroadCastMessage(const BroadCastMessage&);
    void assertMessageId(const AntMessage2::AntMessageId expected, const AntMessage2::AntMessageId actual);

    const int _channelNumber;
    int _deviceNumber;
    const AntSensorType _sensorType;
    const AntSportPeriod _channelPeriod;
    ChannelState _state;
};
}
#endif // ANTCHANNELHANDLER_H
