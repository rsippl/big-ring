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
#include <queue>
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

    enum class ChannelState {
        CLOSED,
        ASSIGNED,
        ID_SET,
        FREQUENCY_SET,
        PERIOD_SET,
        TIMEOUT_SET,
        OPENED,
        SEARCHING,
        SEARCH_TIMEOUT,
        TRACKING,
        LOST_CONNECTION,
        UNASSIGNED
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

    static const int ACKNOWLEDGED_MESSAGE_QUEUE_CAPACITY = 10;
    /**
     * Queue an Acknowledged message. The message will be sent after all other messages on the queue have been sent.
     * If the queue is filled up to ACKNOWLEDGED_MESSAGE_QUEUE_CAPACITY, the message will be dropped and false will be returned.
     */
    void queueAcknowledgedMessage(const AntMessage2 &message);

    quint8 channelNumber() const;

    /** This method should be implemented by subclasses for their
     * specific way of handling broadcast messages.
     */
    virtual void handleBroadCastMessage(const BroadCastMessage& message) = 0;
    /**
     * If true, we're implementing a master node of a channel. By default, we'll implement slaves.
     */
    virtual bool isMasterNode() const;

    /**
     * Subclasses may implement transmissionType() to indicate the transmission type for the
     * channel. For slave channels, the result will always be 0u, which is the default.
     */
    virtual quint8 transmissionType() const;
    /**
     * Subclasses may implement channelOpened to get a notification of this channel being opened.
     * The default implementation is empty.
     */
    virtual void channelOpened();
private:
    void setState(ChannelState state);
    void advanceState(const AntMessage2::AntMessageId messageId);
    void handleFirstBroadCastMessage(const BroadCastMessage&);
    void assertMessageId(const AntMessage2::AntMessageId expected, const AntMessage2::AntMessageId actual);
    void transferTxCompleted();
    void transferTxFailed();
    void sendNextAcknowledgedMessage();

    const int _channelNumber;
    int _deviceNumber;
    const AntSensorType _sensorType;
    const AntSportPeriod _channelPeriod;
    ChannelState _state;

    /** A queue of acknowledged messages that must be sent. */
    std::queue<AntMessage2> _acknowledgedMessagesToSend;
    /** If this is true, there is an acknowledged message in flight and we cannot send a new one. */
    bool _acknowledgedMessageInFlight = false;
};

class AntMasterChannelHandler: public AntChannelHandler
{
    Q_OBJECT
public:
    AntMasterChannelHandler(int channelNumber, const AntSensorType sensorType,
                            AntSportPeriod channelPeriod, QObject* parent);
    virtual ~AntMasterChannelHandler() {}
    virtual void sendSensorValue(const SensorValueType valueType, const QVariant& value) = 0;
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) final;

    /** these channels are always master */
    virtual bool isMasterNode() const final { return true; }
};
}
#endif // ANTCHANNELHANDLER_H
