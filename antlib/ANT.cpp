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

//------------------------------------------------------------------------
// This code has been created by folding the ANT.cpp source with
// the Quarqd source provided by Mark Rages and the Serial device
// code from Computrainer.cpp
//------------------------------------------------------------------------

#include "ANT.h"
#include "ANTChannel.h"
#include "antmessage2.h"
#include "antmessagegatherer.h"

#include "antdevicefinder.h"
#include "antdevice.h"
#ifdef Q_OS_WIN
#include "usbexpressantdevice.h"
#else
#include "unixserialusbant.h"
#endif

#include <QByteArray>
#include <QtDebug>
#include <QThread>

using indoorcycling::AntSensorType;
namespace {
// network key
const std::array<quint8,8> networkKey = { 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45 };
}

ANT::ANT(QObject *parent): QObject(parent),
    _antDeviceFinder(new indoorcycling::AntDeviceFinder(this)),
    _antMessageGatherer(new AntMessageGatherer(this))
{
    qRegisterMetaType<indoorcycling::AntSensorType>("indoorcycling::AntChannelType");
    connect(_antMessageGatherer, &AntMessageGatherer::antMessageReceived,
            this, &ANT::processMessage);

    // setup the channels
    for (uint i=0; i<antChannel.size(); i++) {

        // create the channel
        antChannel[i] = new ANTChannel(i, this);

        // connect up its signals
        connect(antChannel[i], &ANTChannel::channelInfo, this, &ANT::channelInfo);
        connect(antChannel[i], SIGNAL(lostInfo(int)), this, SLOT(lostInfo(int)));
        connect(antChannel[i], SIGNAL(staleInfo(int)), this, SLOT(staleInfo(int)));
        connect(antChannel[i], SIGNAL(searchTimeout(int)), this, SLOT(slotSearchTimeout(int)));

        connect(antChannel[i], &ANTChannel::heartRateMeasured, this, &ANT::heartRateMeasured);
        connect(antChannel[i], &ANTChannel::powerMeasured, this, &ANT::powerMeasured);
        connect(antChannel[i], &ANTChannel::cadenceMeasured, this, &ANT::cadenceMeasured);
        connect(antChannel[i], &ANTChannel::speedMeasured, this, &ANT::speedMeasured);
        connect(antChannel[i], &ANTChannel::antMessageGenerated, this, &ANT::sendMessage);
    }

    channels = 0;
}

ANT::~ANT()
{
    qDebug() << "destroying ant" << QThread::currentThreadId();
}

/*======================================================================
 * Main thread functions; start, stop etc
 *====================================================================*/

void ANT::initialize()
{
    antDevice = _antDeviceFinder->openAntDevice();
    if (!antDevice) {
        emit initializationFailed();
        return;
    }
    channels = 0;

    if (antDevice->isValid()) {
        channels = 4;
        connect(antDevice.get(), &indoorcycling::AntDevice::bytesRead, this, &ANT::bytesReady);
        if (antDevice->isReady()) {
            startCommunication();
        } else {
            connect(antDevice.get(), &indoorcycling::AntDevice::deviceReady, this, &ANT::startCommunication);
        }
    } else {
        emit initializationFailed();
        return;
    }
}

void ANT::startCommunication()
{
    qDebug() << "resetting system";
    sendMessage(AntMessage2::systemReset());
    // wait for 500ms before sending network key.
    QTimer::singleShot(800, this, SLOT(sendNetworkKey()));
    connect(antDevice.get(), &indoorcycling::AntDevice::bytesRead, this, &ANT::bytesReady);
}

void ANT::sendNetworkKey()
{
    sendMessage(AntMessage2::setNetworkKey(1, networkKey));
    emit initializationSucceeded();
}

void ANT::configureDeviceChannels()
{
    addDevice(0, indoorcycling::SENSOR_TYPE_SPEED, 0);
    addDevice(0, indoorcycling::SENSOR_TYPE_POWER, 1);
    addDevice(0, indoorcycling::SENSOR_TYPE_HR, 2);
    addDevice(0, indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE, 3);
}

/*======================================================================
 * Channel management
 *====================================================================*/

// returns 1 for success, 0 for fail.
int
ANT::addDevice(int device_number, AntSensorType device_type, int channel_number)
{
    // if we're given a channel number, then use that one
    if (channel_number>-1) {
        //antChannel[channel_number]->close();
        antChannel[channel_number]->open(device_number, device_type);
        return 1;
    }

    // if we already have the device, then return.
    // but only if the device number is given since
    // we may choose to scan for multiple devices
    // on separate channels (e.g. 0p on channel 0
    // and 0p on channel 1
    if (device_number != 0) {
        for (int i=0; i<channels; i++) {
            if (((antChannel[i]->channel_type & 0xf ) == device_type) &&
                    (antChannel[i]->deviceNumber == device_number)) {
                // send the channel found...
                //XXX antChannel[i]->channelInfo();
                return 1;
            }
        }
    }

    // look for an unused channel and use on that one
    for (int i=0; i<channels; i++) {
        if (antChannel[i]->channel_type == indoorcycling::SENSOR_TYPE_UNUSED) {

            //antChannel[i]->close();
            antChannel[i]->open(device_number, device_type);
            return 1;
        }
    }

    // there are no unused channels.  fail.
    return 0;
}

void ANT::channelInfo(int channelNumber, int deviceNumber, int deviceTypeId, const QString& description)
{
    QString typeCode(deviceTypeId);

    qDebug()<<"found device number"<<deviceNumber<<"type"<<deviceTypeId<<"on channel"<<channelNumber
           << "is a "<< description << "with code"<< typeCode;

    emit foundDevice(channelNumber, deviceNumber, deviceTypeId, description, typeCode);
}

void
ANT::dropInfo(int channel, int drops, int received)    // we dropped a message
{
    double reliability = 100.00f - (100.00f * double(drops) / double(received));
    //qDebug()<<"Channel"<<channel<<"reliability is"<< (int)(reliability)<<"%";
    emit signalStrength(channel, reliability);
    return;
}

void
ANT::lostInfo(int number)    // we lost the connection
{
    if (number < 0 || number >= channels) return; // ignore out of bound

    emit lostDevice(number);
    //qDebug()<<"lost info for channel"<<number;
}

void
ANT::staleInfo(int number)   // info is now stale - set to zero
{
    if (number < 0 || number >= channels) return; // ignore out of bound

    //qDebug()<<"stale info for channel"<<number;
}

void
ANT::slotSearchTimeout(int number) // search timed out
{
    if (number < 0 || number >= channels) return; // ignore out of bound

    emit searchTimeout(number);
    //qDebug()<<"search timeout on channel"<<number;
}

void ANT::slotSearchComplete(int number) // search completed successfully
{
    if (number < 0 || number >= channels) return; // ignore out of bound

    emit searchComplete(number);
    //qDebug()<<"search completed on channel"<<number;
}

void ANT::sendMessage(const AntMessage2& m) {
    qDebug() << "Sending ANT Message" << m.toString();
    antDevice->writeAntMessage(m);
}

void ANT::bytesReady(const QByteArray &bytes)
{
    if (!bytes.isEmpty()) {
        _antMessageGatherer->submitBytes(bytes);
    }
}

//
// Pass inbound message to channel for handling
//
void
ANT::handleChannelEvent(const AntChannelEventMessage& channelEventMessage) {
    if (channelEventMessage.messageId() == AntMessage2::AntMessageId::SET_NETWORK_KEY &&
            channelEventMessage.messageCode() == AntChannelEventMessage::MessageCode::EVENT_RESPONSE_NO_ERROR) {
        qDebug() << "succesfully set network key";
        configureDeviceChannels();
    } else {
        quint8 channel = channelEventMessage.channelNumber();
        antChannel[channel]->channelEvent(channelEventMessage);
    }
}

void ANT::handleBroadCastEvent(const BroadCastMessage &broadCastEventMessage)
{
    antChannel[broadCastEventMessage.channelNumber()]->broadcastEvent(broadCastEventMessage);
}

void ANT::handleChannelIdMessage(const SetChannelIdMessage &message)
{
    antChannel[message.channelNumber()]->channelIdEvent(message);
}

void ANT::processMessage(QByteArray message) {
    std::unique_ptr<AntMessage2> antMessage = AntMessage2::createMessageFromBytes(message);
    switch(antMessage->id()) {
    case AntMessage2::AntMessageId::CHANNEL_EVENT:
        handleChannelEvent(*antMessage->asChannelEventMessage());
        break;
    case AntMessage2::AntMessageId::BROADCAST_EVENT:
        handleBroadCastEvent(BroadCastMessage(*antMessage));
        break;
    case AntMessage2::AntMessageId::SET_CHANNEL_ID:
        handleChannelIdMessage(SetChannelIdMessage(*antMessage));
        break;
    default:
        qDebug() << "Unhandled Message" << antMessage->toString();
    }
}
