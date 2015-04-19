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
#include "ANTMessage.h"
#include <QObject>
#include <QDateTime>

#define CHANNEL_TYPE_QUICK_SEARCH 0x10 // or'ed with current channel type
/* after fast search, wait for slow search.  Otherwise, starting slow
   search might postpone the fast search on another channel. */
#define CHANNEL_TYPE_WAITING 0x20
#define CHANNEL_TYPE_PAIR   0x40 // to do an Ant pair
#define MESSAGE_RECEIVED -1

class ANTChannel : public QObject {
    Q_OBJECT
private:
    ANT *parent;

    ANTMessage lastMessage, lastStdPwrMessage;
    int dualNullCount, nullCount, stdNullCount;
    qint64 last_message_timestamp;
    qint64 blanking_timestamp;
    int blanked;
    char id[10]; // short identifier
    bool channel_assigned;

    int messages_received; // for signal strength metric
    int messages_dropped;

    unsigned char rx_burst_data[RX_BURST_DATA_LEN];
    int           rx_burst_data_index;
    unsigned char rx_burst_next_sequence;
    void (*rx_burst_disposition)(struct ant_channel *);
    void (*tx_ack_disposition)(struct ant_channel *);

    // what we got
    int manufacturer_id;
    int product_id;
    int product_version;

    void handlePowerMessage(ANTMessage antMessage);
public:
    // Channel Information - to save tedious set/getters made public
    int number; // Channel number within Ant chip
    int state;
    int channel_type;
    int device_number;
    int channel_type_flags;
    int device_id;
    int search_type;

    ANTChannel(int number, ANT *parent);

    // channel open/close
    void init();
    void open(int device_number, int channel_type);
    void close();

    // handle inbound data
    void receiveMessage(unsigned char *message);
    void channelEvent(unsigned char *message);
    void burstInit();
    void burstData(unsigned char *message);
    void broadcastEvent(unsigned char *message);
    void channelId(unsigned char *message);
    void setChannelID(int device, int id, int txtype);
    void setId();
    void requestCalibrate();
    void attemptTransition(int message_code);

    // search
    int isSearching();

signals:

    void channelInfo(int number, int device_number, int device_id); // we got a channel info message
    void dropInfo(int number, int dropped, int received);    // we dropped a packet
    void lostInfo(int number);    // we lost a connection
    void staleInfo(int number);   // the connection is stale
    void searchTimeout(int number); // search timed out
    void searchComplete(int number); // search completed successfully

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
