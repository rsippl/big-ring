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
#include "antmessage2.h"
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
    AntMessage2 lastAntMessage;
    ANTMessage lastMessage, lastStdPwrMessage;
    int dualNullCount, nullCount, stdNullCount;
    QDateTime _lastMessageTime;
    char id[10]; // short identifier
    bool channel_assigned;
    bool opened;

    int messages_received; // for signal strength metric
    int messages_dropped;

    void handlePowerMessage(ANTMessage antMessage);
    void handleHeartRateMessage(const AntMessage2& antMessage);
public:
    // Channel Information - to save tedious set/getters made public
    int number; // Channel number within Ant chip
    int state;
    int channel_type;
    int device_number;
    int channel_type_flags;
    int device_id;
    int search_type;

    ANTChannel(int number, QObject *parent);

    // channel open/close
    void init();
    void open(int device_number, int channel_type);

    // handle inbound data
    void receiveMessage(const QByteArray &message);
    void channelEvent(const QByteArray& bytes);
    void broadcastEvent(const AntMessage2& broadcastMessage);
    void channelId(unsigned char *message);
    void setId();
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
