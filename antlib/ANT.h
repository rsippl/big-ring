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

#ifndef gc_ANT_h
#define gc_ANT_h

#include "antsensortype.h"
#include "antmessage2.h"

#include <array>
#include <memory>

//
// QT stuff
//
#include <QObject>
#include <QStringList>
#include <QFile>
#include <QSharedPointer>
#include <QTimer>

#include <QDebug>

namespace indoorcycling {

class AntDeviceFinder;
class AntDevice;
}
class ANTChannel;
class AntDevice;

//======================================================================
// ANT Constants
//======================================================================

enum AntSportPeriod {
    ANT_SPORT_UNUSED_PERIOD = 0,
    ANT_SPORT_HR_PERIOD = 8070,
    ANT_SPORT_POWER_PERIOD = 8182,
    ANT_SPORT_SPEED_PERIOD = 8118,
    ANT_SPORT_CADENCE_PERIOD = 8102,
    ANT_SPORT_SPEED_AND_CADENCE_PERIOD = 8086
};

class AntMessage2;
class AntMessageGatherer;
//======================================================================
// Worker thread
//======================================================================
class ANT: public QObject
{
    Q_OBJECT


public:
    ANT(QObject *parent = 0);
    ~ANT();

signals:
    void initializationSucceeded();
    /** Signal failure of initialization */
    void initializationFailed();

    void foundDevice(int channel, int device_number, int device_id, QString description, QString typeCode); // channelInfo
    void lostDevice(int channel);            // dropInfo
    void searchTimeout(int channel);         // searchTimeount
    void searchComplete(int channel);         // searchComplete
    void signalStrength(int channel, double reliability);

    /** heart rate in beats per minute */
    void heartRateMeasured(int bpm);
    /** power in watts */
    void powerMeasured(float watts);
    /** cadence in revolutions per minute. Also sets the channel type, as this can come from both cadence and power sensors */
    void cadenceMeasured(float rpm, indoorcycling::AntSensorType channelType);
    /** speed in wheel revolutions per minute */
    void speedMeasured(float rpm);
public slots:

    /** Initialize ANT+ device */
    void initialize();

private slots:
    void startCommunication();
    void sendNetworkKey();
    void processMessage(QByteArray message);
    void channelInfo(int channelNumber, int deviceNumber, int deviceTypeId, const QString &description);  // found a device
    void dropInfo(int number, int drops, int received);    // we dropped a connection
    void lostInfo(int number);    // we lost informa
    void staleInfo(int number);   // info is now stale
    void slotSearchTimeout(int number); // search timed out
    void slotSearchComplete(int number); // search completed successfully

    void sendMessage(const AntMessage2&);

    void bytesReady(const QByteArray&  bytes);
public:
    std::array<ANTChannel*, 8> antChannel;

    // ANT Devices and Channels
    int addDevice(int device_number, indoorcycling::AntSensorType device_type, int channel_number);

private:
    void handleChannelEvent(const AntChannelEventMessage& channelEventMessage);
    void handleBroadCastEvent(const BroadCastMessage& broadCastEventMessage);
    void handleChannelIdMessage(const SetChannelIdMessage& message);

    QByteArray rawRead();
    int rawWrite(const QByteArray &bytes);

    void configureDeviceChannels();

    int channels;  // how many 4 or 8 ? depends upon the USB stick...

    indoorcycling::AntDeviceFinder* _antDeviceFinder;
    std::unique_ptr<indoorcycling::AntDevice> antDevice;
    AntMessageGatherer* _antMessageGatherer;
};

#endif
