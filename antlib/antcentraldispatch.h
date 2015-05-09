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
#ifndef ANTCENTRALDISPATCH_H
#define ANTCENTRALDISPATCH_H

#include <functional>
#include <memory>

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>

namespace indoorcycling {
class AntChannelHandler;
}
class AntChannelEventMessage;
class AntMessageGatherer;
class AntMessage2;
class BroadCastMessage;
class SetChannelIdMessage;


#include "antdevice.h"
#include "antsensortype.h"

namespace indoorcycling {
/**
 * The Main ANT+ class that manages the connection with the ANT+ usb stick and through that stick, to the ANT+
 * sensors that are found.
 */
class AntCentralDispatch : public QObject
{
    Q_OBJECT
public:
    explicit AntCentralDispatch(QObject *parent = 0);

    bool antUsbStickPresent() const;
    bool initialized() const;

    bool searchForSensorType(AntSensorType channelType);
    bool searchForSensor(AntSensorType channelType, int deviceNumber);

    int currentHeartRate() const;

signals:
    /** signal emitted when scanning for an usb stick is finished. @param found indicates whether or not an ANT+ usb
     * stick was found.
     */
    void antUsbStickScanningFinished(bool found);
    /**
     * Signal emitted when the connection to the ANT+ stick and the ANT+ network has been set up. After this, we
     * can try to connect to or search for ANT+ sensors. If success is false, we have not been able to initialize the
     * ANT+ connection.
     */
    void initializationFinished(bool success);
    /**
     * emitted when the search for a certain sensor type is started.
     */
    void searchStarted(AntSensorType channelType, int channelNumber);
    /**
     * emitted when an ANT+ sensor is found. @param channelType denotes the type of sensor, @param deviceNumber the
     * number of the device itself.
     */
    void sensorFound(AntSensorType channelType, int deviceNumber);
    /**
     *  emitted when a search for sensor has timed out.
     */
    void sensorNotFound(AntSensorType channelType);
    /**
     * heart rate measured
     */
    void heartRateMeasured(int heartRate);

public slots:
    /**
     * Initialize the connection to the ANT+ stick. After calling this, listen for the signal initializationFinished(bool)
     * to check if the connection was successful.
     */
    void initialize();
private slots:
    void messageFromAntUsbStick(const QByteArray& bytes);
    /**
     * Reset ANT+ USB Stick.
     */
    void resetAntSystem();
    /**
     * Set ANT+ Network Key
     */
    void sendNetworkKey();
    /**
     * This slot is called when an Ant Channel is set to a certain device number.
     */
    void setChannelInfo(int channelNumber, AntSensorType sensorType, int sensorDeviceNumber);
    /**
     * slot called when a search is timed out.
     */
    void searchTimedOut(int channelType, AntSensorType sensorType);
    /**
     * handle a new sensor value from one of the sensors.
     */
    void handleSensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                     const QVariant& sensorValue);
private:
    /**
     * Start scanning for an ANT+ usb stick. When scanning is finished, antUsbStickScanningFinished(AntDeviceType) is emitted.
     */
    void scanForAntUsbStick();

    /**
     * Send a message to the USB ANT+ Stick.
     */
    void sendAntMessage(const AntMessage2& message);

    /**
     * handle a channel event message.
     */
    void handleChannelEvent(const AntChannelEventMessage& channelEventMessage);
    /**
     * handle a broad cast message
     */
    void handleBroadCastMessage(const BroadCastMessage& broadCastMessage);

    /**
     * handle channel id mesage.
     */
    void handleChannelIdMessage(const SetChannelIdMessage& channelIdMessage);

    template <class T>
    bool sendToChannel(const T& message, std::function<void(AntChannelHandler*, const T&)> sendFunction);

    std::unique_ptr<AntDevice> _antUsbStick;
    bool _initialized;
    AntMessageGatherer* _antMessageGatherer;

    int _currentHeartRate;

    QVector<QSharedPointer<indoorcycling::AntChannelHandler>> _channels;
    QTimer* _initializationTimer;
};

template <class T>
bool AntCentralDispatch::sendToChannel(const T& message, std::function<void(indoorcycling::AntChannelHandler*,const T&)> sendFunction)
{
    quint8 channelNumber = message.channelNumber();
    QSharedPointer<indoorcycling::AntChannelHandler> channel = _channels[channelNumber];
    if (channel.isNull()) {
        return false;
    } else {
        sendFunction(channel.data(), message);
        return true;
    }
}

}

#endif // ANTCENTRALDISPATCH_H
