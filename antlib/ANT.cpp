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
#include "ANTMessage.h"
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

namespace {
// network key
const std::array<quint8,8> networkKey = { 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45 };
}
// supported sensor types
const QMap<AntChannelType,ant_sensor_type_t> ANT::ant_sensor_types({
    { CHANNEL_TYPE_UNUSED, {0, 0, 0, 0, "Unused", '?', "" }},
    { CHANNEL_TYPE_HR, {ANT_SPORT_HR_PERIOD, ANT_SPORT_HR_TYPE,
      ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Heartrate", 'h', ":images/IconHR.png" }},
    { CHANNEL_TYPE_POWER, {ANT_SPORT_POWER_PERIOD, ANT_SPORT_POWER_TYPE,
      ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Power", 'p', ":images/IconPower.png" }},
    { CHANNEL_TYPE_SPEED, {ANT_SPORT_SPEED_PERIOD, ANT_SPORT_SPEED_TYPE,
      ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Speed", 's', ":images/IconSpeed.png" }},
    { CHANNEL_TYPE_CADENCE, {ANT_SPORT_CADENCE_PERIOD, ANT_SPORT_CADENCE_TYPE,
      ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Cadence", 'c', ":images/IconCadence.png" }},
    { CHANNEL_TYPE_SPEED_AND_CADENCE, {ANT_SPORT_SandC_PERIOD, ANT_SPORT_SandC_TYPE,
      ANT_SPORT_FREQUENCY, ANT_SPORT_NETWORK_NUMBER, "Speed + Cadence", 'd', ":images/IconCadence.png" }},
});

ANT::ANT(QObject *parent): QObject(parent),
    _antDeviceFinder(new indoorcycling::AntDeviceFinder(this)),
    _antMessageGatherer(new AntMessageGatherer(this))
{
    connect(_antMessageGatherer, &AntMessageGatherer::antMessageReceived,
            this, &ANT::processMessage);
    powerchannels=0;

    // setup the channels
    for (int i=0; i<ANT_MAX_CHANNELS; i++) {

        // create the channel
        antChannel[i] = new ANTChannel(i, this);

        // connect up its signals
        connect(antChannel[i], SIGNAL(channelInfo(int,int,int)), this, SLOT(channelInfo(int,int,int)));
        connect(antChannel[i], SIGNAL(dropInfo(int,int,int)), this, SLOT(dropInfo(int,int,int)));
        connect(antChannel[i], SIGNAL(lostInfo(int)), this, SLOT(lostInfo(int)));
        connect(antChannel[i], SIGNAL(staleInfo(int)), this, SLOT(staleInfo(int)));
        connect(antChannel[i], SIGNAL(searchTimeout(int)), this, SLOT(slotSearchTimeout(int)));
        connect(antChannel[i], SIGNAL(searchComplete(int)), this, SLOT(slotSearchComplete(int)));

        connect(antChannel[i], &ANTChannel::heartRateMeasured, this, &ANT::heartRateMeasured);
        connect(antChannel[i], &ANTChannel::powerMeasured, this, &ANT::powerMeasured);
        connect(antChannel[i], &ANTChannel::cadenceMeasured, this, &ANT::cadenceMeasured);
        connect(antChannel[i], &ANTChannel::speedMeasured, this, &ANT::speedMeasured);
        connect(antChannel[i], &ANTChannel::antMessageGenerated, this, &ANT::antMessageGenerated);
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
    powerchannels = 0;

    antDevice = _antDeviceFinder->openAntDevice();
    if (antDevice.isNull()) {
        emit initializationFailed();
        return;
    }
    channels = 0;

    if (antDevice->isValid()) {
        channels = 4;
    } else {
        emit initializationFailed();
        return;
    }
    antlog.setFileName("antlog.bin");
    antlog.open(QIODevice::WriteOnly | QIODevice::Truncate);

    qDebug() << "resetting system";
    sendMessage(AntMessage2::systemReset());
    // wait for 500ms before sending network key.
    _initializiationTimer.singleShot(500, this, SLOT(sendNetworkKey()));
}

void ANT::sendNetworkKey()
{
    sendMessage(AntMessage2::setNetworkKey(1, networkKey));
    emit initializationSucceeded();
}

void ANT::configureDeviceChannels()
{
    addDevice(0, CHANNEL_TYPE_SPEED_AND_CADENCE, 0);
    addDevice(0, CHANNEL_TYPE_POWER, 1);
    addDevice(0, CHANNEL_TYPE_CADENCE, 2);
    addDevice(0, CHANNEL_TYPE_HR, 3);
}

void ANT::readCycle()
{
    bool bytesRead = false;

    while (true) {
        QByteArray bytes = rawRead();
        if (bytes.isEmpty())
            break;
        _antMessageGatherer->submitBytes(bytes);
        bytesRead = true;
    }

    if (!bytesRead)
        return;
}

/*======================================================================
 * Channel management
 *====================================================================*/

// returns 1 for success, 0 for fail.
int
ANT::addDevice(int device_number, AntChannelType device_type, int channel_number)
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
                    (antChannel[i]->device_number == device_number)) {
                // send the channel found...
                //XXX antChannel[i]->channelInfo();
                return 1;
            }
        }
    }

    // look for an unused channel and use on that one
    for (int i=0; i<channels; i++) {
        if (antChannel[i]->channel_type == CHANNEL_TYPE_UNUSED) {

            //antChannel[i]->close();
            antChannel[i]->open(device_number, device_type);
            return 1;
        }
    }

    // there are no unused channels.  fail.
    return 0;
}

void
ANT::channelInfo(int channel, int device_number, int device_id)
{
    QString description(deviceTypeDescription(device_id));
    QString typeCode(device_id);

    qDebug()<<"found device number"<<device_number<<"type"<<device_id<<"on channel"<<channel
           << "is a "<< description << "with code"<< typeCode;

    emit foundDevice(channel, device_number, device_id, description, typeCode);
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

void
ANT::slotSearchComplete(int number) // search completed successfully
{
    if (number < 0 || number >= channels) return; // ignore out of bound

    emit searchComplete(number);
    //qDebug()<<"search completed on channel"<<number;
}

void ANT::antMessageGenerated(const AntMessage2 &antMessage)
{
    qDebug() << "sending" << antMessage.toString();
    antDevice->writeAntMessage(antMessage);
}

/*----------------------------------------------------------------------
 * Message I/O
 *--------------------------------------------------------------------*/
void
ANT::sendMessage(const ANTMessage &m) {
    qDebug() << "Sending ANT Message" << m.toString();
    QByteArray bytes((const char*) m.data, m.length);
    static const char padding[5] = { '\0', '\0', '\0', '\0', '\0' };
    QByteArray paddingBytes(padding, 5);
    rawWrite(bytes + paddingBytes);
}

void
ANT::sendMessage(const AntMessage2& m) {
    qDebug() << "Sending ANT Message" << m.toString();
    antDevice->writeAntMessage(m);
}

//
// Pass inbound message to channel for handling
//
void
ANT::handleChannelEvent(QByteArray& message) {
    int channel = message[ANT_OFFSET_DATA] & 0x7;
    if(channel >= 0 && channel < channels) {
        // handle a channel event here!
        antChannel[channel]->receiveMessage(message);
    }
}

void
ANT::processMessage(QByteArray message) {
    QDataStream out(&antlog);
    for (int i=0; i<ANT_MAX_MESSAGE_SIZE; i++)
        out<< message;

    switch (message[ANT_OFFSET_ID]) {
    case ANT_ACK_DATA:
    case ANT_BROADCAST_DATA:
    case ANT_CHANNEL_STATUS:
    case ANT_CHANNEL_ID:
    case ANT_BURST_DATA:
        handleChannelEvent(message);
        break;

    case ANT_CHANNEL_EVENT:
    {
        AntChannelEventMessage eventMessage(message);
        qDebug() << "received" << eventMessage.toString();
        switch (eventMessage.messageId()) {
        case AntMessage2::SET_NETWORK_KEY:
            if (eventMessage.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR) {
                qDebug() << "succesfully set network key";
                configureDeviceChannels();
            }
            break;
        default:
            handleChannelEvent(message);
        }
    }
        break;
    default:
        break;
    }
}

int ANT::rawWrite(const QByteArray &bytes) // unix!!
{
    return antDevice->writeBytes(bytes);
}

QByteArray ANT::rawRead()
{
    return antDevice->readBytes();
}

// convert ANT value to human string
const QString ANT::deviceTypeDescription(int type)
{
    for (auto& sensorType: ant_sensor_types) {
        if (sensorType.device_id == type) {
            return sensorType.descriptive_name;
        }
    }
    return "Unknown device type";
}

